/*
 Lego Powerfunctions car model controlled over bluetooth
 Arduino Uno controls motors and servo
 Remote controlled with android app developed with MIT App Inventor 2
 
 Circuit:
 * Serial communication   (uses Uno pin 0,1)    Bluetooth-module is attached (has to be detached when loading program over USB)
 * L293D with motor       (uses Uno digital pins 2,5,9)
 * L293D with servo       (uses Uno digital pins 6,8,3)
 
 
*/

// Used libraries:
#include <SPI.h>                             // Serial Peripheral Interface Library
#include <String.h>                          // contains function strtok: split string into tokens


// Serial buffer size: calculate based on max input size expected for one command over bluetooth serial interface
#define INPUT_SIZE 30

// Motor control digital output pins defined as global constants (4 wheel drive with 2 Lego motors):
const int controlPin1A = 2;                  // L293D driver input 1A on pin no 2  http://www.ti.com/lit/ds/symlink/l293.pdf connected to Arduino digital output pin 2
const int controlPin2A = 5;                  // L293D driver input 2A on pin no 7 connected to Arduino digital output pin 5
const int ENablePin = 9;                     // L293D ENable(1,2) input on pin no 1 connected to Arduino digital output pin 9
// Servo control digital output pins defined as global constants (Servo steering with 1 Lego servo):
const int controlPin3A = 6;                  // L293D driver input 3A on pin no 10 connected to Arduino digital output pin 6
const int controlPin4A = 8;                  // L293D driver input 4A on pin no 15 connected to Arduino digital output pin 8 
const int servoENablePin = 3;                // L293D ENable(3,4) input on pin no 9 connected to Arduino digital output pin 3
// Motor control global variables: 
int motorSpeed = 0;                          // Motor speed 0..255
int motorDirection = 1;                      // Forward (1) or reverse (0)
// Servo control global variables:
int steering = 0;                            // Servo position 0..255
int steeringDirection = 0;                   // Left (0) and Right (1)



void setup() 
{
   Serial.begin(9600);                       // initialize serial communication
   Serial.setTimeout(1000);                  // 1000 ms time out
   // Declare digital output pins:
   pinMode(controlPin1A, OUTPUT);      // 1A
   pinMode(controlPin2A, OUTPUT);      // 2A
   pinMode(ENablePin, OUTPUT);         // EN1,2
   pinMode(controlPin3A, OUTPUT);      // 3A
   pinMode(controlPin4A, OUTPUT);      // 4A
   pinMode(servoENablePin, OUTPUT);    // EN3,4
   digitalWrite(ENablePin, LOW);       // motor off
   digitalWrite(servoENablePin, LOW);  // steering centered
}


void loop() 
{
   // Get next command from serial bluetooth (add 1 byte for final 0)
   char input[INPUT_SIZE + 1];                  // array of type char (C-string) with null-termination (https://www.arduino.cc/en/Reference/String)
   byte size = Serial.readBytesUntil('\n', input, INPUT_SIZE);   //read Serial until new line or buffer full or time out
   // Add the final 0 to end the C-string
   input[size] = 0;

   // Read command which is a C-string of the form:  "RC,RCsteering,RCspeed,\n\0"  (ends with a new line character and a null)
   char* command = strtok(input, ",");          // ignore command for now (RC for Remote Control is the only command sent by the app for the time being)    
   char* RCsteering = strtok(NULL, ",");        // next comes RCsteering which is in the Range: -100 (left).. 0 .. 100 (right)
   char* RCspeed = strtok(NULL, ",");           // next comes RCspeed which is in the Range: -100 (full speed reverse).. 0 .. 100 (full speed forward)
   int iRCsteering = atoi(RCsteering);          // convert string RCsteering into integer iRCsteering
   int iRCspeed = atoi(RCspeed);                // convert string RCspeed into integer iRCspeed
   
   // Set motor and servo control variables
   if (iRCsteering > 0) steeringDirection = 1;   // turn right if iRCsteering = 1..100
   else steeringDirection = 0;                   // turn left  if iRCsteering = -100..0
   steering = int(2.55*abs(iRCsteering));        // Servo position 0..255 based on command 0..100
   if (iRCspeed > 0) motorDirection = 1;         // Set car in forward if iRCspeed = 1..100
   else motorDirection = 0;                      // Set car in reverse if iRCspeed = -100..0
   motorSpeed = int(2.55*abs(iRCspeed));         // Set speed 0..255 based on command 0..100
   
   SetMotorControl();                            // adjust motor direction and speed 
}




void SetMotorControl()
/*
L293 logic:    EN1,2   1A    2A
               H       H     L    Motor turns left  (Forward; motorDirection == 1)
               H       L     H    Motor turns right (Reverse; motorDirection == 0)
               
               EN3,4   3A    4A
               H       H     L    Servo turns left  (steeringDirection == 0)
               H       L     H    Servo turns right (steeringDirection == 1)

Motor speed:   PWM signal on EN1,2 (490 Hz; digital output value 0..255 for motorSpeed)
Servo position:PWM signal on EN3,4 (490 Hz; digital output value 0..255 for position; 0 is straight ahead)
*/
{
  if (motorDirection == 1)               //Forward
    {
       digitalWrite(controlPin1A, HIGH);
       digitalWrite(controlPin2A, LOW);
    }
  else                                   //Reverse
    {
       digitalWrite(controlPin1A, LOW);
       digitalWrite(controlPin2A, HIGH);
    } 
  analogWrite(ENablePin, motorSpeed);    //Speed
    
  if (steeringDirection == 0)            //Left
    {
       digitalWrite(controlPin3A, HIGH);
       digitalWrite(controlPin4A, LOW);
    }
  else                                   //Right
    {
       digitalWrite(controlPin3A, LOW);
       digitalWrite(controlPin4A, HIGH);
    } 
  analogWrite(servoENablePin, steering); //Servo position

}


