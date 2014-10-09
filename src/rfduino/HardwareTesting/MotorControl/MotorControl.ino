// RFD22301 LEG 16 ->  GPIO0  ->  A_PH  ->
// RFD22301 LEG 7  ->  GPIO6  ->  A_EN  ->

#define MOTOR_A1_PIN           0       // A PHASE
#define MOTOR_A2_PIN           6       // A ENABLE
#define MOTOR_B1_PIN           1       
#define MOTOR_B2_PIN           5
#define MOTOR_MAX_SPEED      255

void setup() {
  Serial.end();
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);  
  stopMotors();
}

void loop() {
  motorTestLowLevel( MOTOR_MAX_SPEED, 2000, 500, MOTOR_A2_PIN, MOTOR_A1_PIN);
  motorTestLowLevel(-MOTOR_MAX_SPEED, 2000, 500, MOTOR_A2_PIN, MOTOR_A1_PIN);
  motorTestLowLevel( MOTOR_MAX_SPEED, 2000, 500, MOTOR_B2_PIN, MOTOR_B1_PIN);
  motorTestLowLevel(-MOTOR_MAX_SPEED, 2000, 500, MOTOR_B2_PIN, MOTOR_B1_PIN);

  // halt
  while (1);
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
