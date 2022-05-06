/*Sakshi Kulkarni
 * 
 * Design Statement:
 * This code is designed to run an RFID enabled pet feeder. 
 * When the button is pressed the box opens. When button is pressed again, box closes.
 * The RFID sensor scans the chip on the pet's collar. 
 * If it matches the ID in the database, the box will open and a green LED will flash. 
 * If it is not the correct ID, the box will remain closed and a red LED will flash. 
 * Once the box is open, the sonar distance measures if the pet is still present and will keep the box open until the pet is more than 30 cm away from sonar.
 * The stepper motors drive the hinge linkage on either side of the box.
 */

// BUTTON INITIALIZATION
#include <ezButton.h>
ezButton button(A0);  // create ezButton object that attach to pin A0;

// STEPPER MOTOR INITIALIZATION
// Include the AccelStepper Library
#include <AccelStepper.h>
// Define step constants
#define FULLSTEP 4
#define HALFSTEP 8
  
/* STEPPER MOTOR 1
  *  Pin1 - port 4         
  *  Pin2 - port 5         
  *  Pin3 - port 6        
  *  Pin4 - port 7 
  */
/* STEPPER MOTOR 2
  *  Pin1 - port 8           
  *  Pin2 - port 9         
  *  Pin3 - port 10        
  *  Pin4 - port 11
  */


// Define Motor Pins
#define motorPin1  4
#define motorPin2  5    
#define motorPin3  6    
#define motorPin4  7                
#define motorPin5  8    
#define motorPin6  9    
#define motorPin7  10   
#define motorPin8  11  

// Define two motor objects
  // The sequence 1-3-2-4 is required for proper sequencing of 28BYJ48 stepper motor
  AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
  AccelStepper stepper2(HALFSTEP, motorPin5, motorPin7, motorPin6, motorPin8);

//RFID SENSOR INITIALIZATION
#include "SPI.h"
#include "MFRC522.h"

/* RFID SENSOR layout
  *  RST/Reset   RST           47      
  *  SPI SS      SDA(SS)       53       
  *  SPI MISO    MISO          50       
  *  SPI SCK     SCK           52 
  */
#include "SPI.h"
#include "MFRC522.h"

#define SS_PIN 53
#define RST_PIN 47 

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;


//SONAR INITIALIATION
/* SONAR SENSOR Layout
VCC  -> 5V
Trig -> port 38
Echo -> port 39
GND  -> GND
*/
#define trigPin 38// Which digital (output) pin will we trigger the pulse on
#define echoPin 39 // Which digital (input) pin will we listen for echo on
long duration;     // Duration used to calculate distance
float distance;

// LED INITIALIZATION
#define LED_PIN A6 //green led
#define LED_PIN A7 //red led

void setup() { //this sets up everything in the code
  //serial monitor 
  Serial.begin(9600);
  //button
  button.setDebounceTime(50); // set debounce time to 50 milliseconds
  //sonar
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() { //this is the main code that continuously loops
 
  // button loop
  bool box = false; //initialize box state as closed.
  
  if(button.isPressed() && box == false){ //if box is closed, open box
  Serial.println("The button is pressed, opening box.");
  delay(100);
  openBox(); 
  box = true;
  }

  if(button.isPressed() && box == true){ //if box is open, close box
  Serial.println("The button is pressed, closing box.");
  delay(100);
  closeBox(); 
  box = false;
  }

  if (sonarDistance <= 30) {  //if pet is approaching, scan for RFID tag
    rfidScan();
  }
 }
  
float microSecondsToCentimeters(long microseconds){ //sonar relays microseconds and this converts it to distance
  // Sound travels 29.386 microseconds per centimeter
  // This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  return microseconds / 29.387 / 2.0;
}


  void rfidScan() { //scans RFID chip and either grants or denies access
     Serial.println("Pet approaching. Scanning for chip...");
       
    if (!rfid.PICC_IsNewCardPresent()){
      String strID = "";
      for (byte i = 0; i < 4; i++) {
        strID +=
          (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
          String(rfid.uid.uidByte[i], HEX) +
          (i != 3 ? ":" : "");
      }
      strID.toUpperCase();
      Serial.print("Tap card key: ");
      Serial.println(strID);
      delay(1000);
    
      if (strID.indexOf("41:17:6B:1B") == 0) {  //this is the unique chip ID that grants access.
        Serial.println("**Access granted**"); 
        digitalWrite(A6, HIGH); // green light flashes
        delay (5000);
        digitalWrite(A6, LOW);
        openBox();
        
        while (sonarDistance <= 20) { //keep the box open while pet is present and eating
          openBox();
          break;
        }
      }
      else {
        Serial.println("**Access denied**");
        digitalWrite(A7, HIGH); //red light flashes
        delay (5000);
        digitalWrite(A7, LOW);
        
        }
    }
  }


  float sonarDistance() { //collects data from sonar
   // sonar function
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Call pulseIn function to wait for High pulse
  // result will be time in microseconds until pulse is detected
  duration = pulseIn(echoPin, HIGH);
  distance = microSecondsToCentimeters(duration); 
  return distance; //returns the disance from sonar to object
  }

  void openBox() { //instructs motors to open the box
  // Motor 1 - Right
  stepper1.setMaxSpeed(1000.0);
  stepper1.setAcceleration(50.0);
  stepper1.setSpeed(200);
  stepper1.moveTo(1000);  //CW
  
  // Motor 2 - Left
  stepper2.setMaxSpeed(1000.0);
  stepper2.setAcceleration(50.0);
  stepper2.setSpeed(200);
  stepper2.moveTo(-1000); //CCW
  }

 void closeBox() { //instructs motors to close the box
  // Motor 1 - Right
  stepper1.setMaxSpeed(1000.0);
  stepper1.setAcceleration(50.0);
  stepper1.setSpeed(200);
  stepper1.moveTo(-1000);  //CCW
  
  // Motor 2 - Left
  stepper2.setMaxSpeed(1000.0);
  stepper2.setAcceleration(50.0);
  stepper2.setSpeed(200);
  stepper2.moveTo(1000); //CW
  }
