// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// This example code is in the public domain.


#include <Servo.h> 
 
Servo myservo[2];  // create servo objects to control a servo 

int pos = 0;    // variable to store the servo position 
 
void setup() 
{ 
  myservo[0].attach(2);  // attaches the servo on pin 2 to the servo object (Servo Output 1 of Servo Shield)
  myservo[1].attach(3);  // attaches the servo on pin 3 to the servo object (Servo Output 2 of Servo Shield)
} 
 
 
void loop() 
{ 
  for(pos = 0; pos < 180; pos += 1)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo[0].write(pos);           // sets the servo 1 position according to the scaled value 
    myservo[1].write(pos);           // sets the servo 2 position according to the scaled value 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  for(pos = 180; pos >= 1; pos -= 1) // goes from 180 degrees to 0 degrees 
  {                                
    myservo[0].write(pos);           // sets the servo 1 position according to the scaled value 
    myservo[1].write(pos);           // sets the servo 2 position according to the scaled value 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
} 
