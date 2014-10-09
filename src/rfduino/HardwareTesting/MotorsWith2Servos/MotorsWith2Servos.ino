#define MOTOR_A1_PIN           0       // A PHASE
#define MOTOR_A2_PIN           6       // A ENABLE
#define MOTOR_B1_PIN           1       
#define MOTOR_B2_PIN           5
#define MOTOR_MAX_SPEED      255

#include <Servo.h> 
 
Servo myservo[2];  // create servo objects to control a servo 

int pos = 0;    // variable to store the servo position 
 
void setup() 
{ 
  Serial.end();
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);  
  stopMotors();
  
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
  motorTestLowLevel( MOTOR_MAX_SPEED, 2000, 500, MOTOR_A2_PIN, MOTOR_A1_PIN);
  motorTestLowLevel(-MOTOR_MAX_SPEED, 2000, 500, MOTOR_A2_PIN, MOTOR_A1_PIN);
  motorTestLowLevel( MOTOR_MAX_SPEED, 2000, 500, MOTOR_B2_PIN, MOTOR_B1_PIN);
  motorTestLowLevel(-MOTOR_MAX_SPEED, 2000, 500, MOTOR_B2_PIN, MOTOR_B1_PIN);
} 


void stopMotors() {
  digitalWrite(MOTOR_A1_PIN, LOW) ;
  analogWrite (MOTOR_A2_PIN, 0);
  digitalWrite(MOTOR_B1_PIN, LOW) ;
  analogWrite (MOTOR_B2_PIN, 0);
}

void motorTestLowLevel(int maxSpeed, int onTime, int intTime, int enablePin, int phasePin) {
  digitalWrite(enablePin, HIGH) ;
  analogWrite (phasePin, maxSpeed);
  delay(onTime);
  digitalWrite(enablePin, LOW) ;
  analogWrite (phasePin, 0);
  delay(intTime);

}

