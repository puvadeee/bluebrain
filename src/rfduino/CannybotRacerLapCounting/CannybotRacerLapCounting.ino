//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Blue Brain
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  04.10.2014  -  Inital Version  (wayne@cannybots.com)
// Version:   1.1  -  11.10.2014  -  Customisable bot name advertised over BLE (wayne@cannybots.com)
//                                   Do not set a custom GZLL_HOST_ADDRESS by default
// Version:   1.2  -  15.10.2014  -  Make use of joypad z axis (wayne@cannybots.com)
// Version:   1.3  -  21.10.2014  -  Added LineFollowing (mampetta@cannybots.com)
// Version:   1.4  -  02.12.2014  -  Added ability to update threshold over the air (wayne@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////

#define BOT_NAME "Cannybot3"                   // custom name (16 chars max)
#define GZLL_HOST_ADDRESS 0x12ACB010           // this needs to match the Joypad sketch value


#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>

void radio_send_formatted(char *fmt, ... );



// PIN ASSIGNEMENT
// total of 7 pins available of which any 4 can be defined as PWM
// Infrared sensors
#define IR1_PIN 2
#define IR2_PIN 3
#define IR3_PIN 4
// Motor Pins
#define MOTOR_A1_PIN 0	// motor direction
#define MOTOR_A2_PIN 6  // motor speed (PWM)
#define MOTOR_B1_PIN 1	// motor direction
#define MOTOR_B2_PIN 5	// motor speed (PWM)


// DEFINITIONS & ITIALISATION
#define DEFAULT_CRUISE_SPEED 70

#define MOTOR_MAX_SPEED 250 // max speed (0..255)
#define JOYPAD_AXIS_DEADZONE 20 //this is to eleminate jitter in 0 position
#define IR_WHITE_THRESHOLD_DEFAULT 750 //to determinie on line or not 

//IR sensor bias
#define IR1_BIAS 0
#define IR2_BIAS 0
#define IR3_BIAS 0
// PID gain setting
#define PID_P 5  //PID gains set below in the code.. 
#define PID_I 0
#define PID_D 1
#define PID_SAMPLE_TIME 5 //determine how fast the loop is executed



// PID working parameters
int Kp = PID_P;
int Ki = PID_I;
int Kd = PID_D;
int P_error = 0;
int I_sum = 0;
int I_error = 0;
int D_error = 0;
int error = 0;
int error_last = 0; // to calculate D_error = error - error_last
int correction = 0;

// Joypad
volatile int16_t  xAxisValue    = 0;              // (left) -255 .. 255 (right)
volatile int16_t  yAxisValue    = 0;              // (down) -255 .. 255 (up)
volatile int16_t  zAxisValue    = 0;
volatile bool     buttonPressed = 0;              // 0 = not pressed, 1 = pressed

// LineFollowing State
bool isLineFollowingMode = false;
bool forceManualMode     = false;
int justOut = 0; // to break the speed if the bot just came out of line

// Timers in milli-seconds (1/1000 of a second)
unsigned long time_Now = millis();                    // the time at the start of the loop()
unsigned long pidLastTime = millis();                // when the PID was calculated last
unsigned long joypadLastTime = millis();             // the time the bot last received a joypad command
unsigned long offLineLastTime = millis();            // last time the bot came off the line
unsigned long offTheLineTime = 0;                    // how long has the bot been off the line, total since last leaving the line

// motor speeds
int lineSpeed = 0;
int speedA = 0;
int speedB = 0;


// IR

#define LINE_STATUS_UNKNOWN           -1
#define LINE_STATUS_NO_LINE           0
#define LINE_STATUS_FOLLOWING_LINE    1
#define LINE_STATUS_RIGHT_TURN        2
#define LINE_STATUS_LEFT_TURN         3
#define LINE_STATUS_T_JUNCTION        4

// Scale factor to apply to IRmax values detected during calibration; result is used to set the whitethreshold level individually for each IR sensor.
#define IR1_SCALING 0.98
#define IR2_SCALING 0.90
#define IR3_SCALING 0.98


// Helper Macros
#define IS_ON_LINE(x) ( (average[(x)-1])>=IRwhiteThreshold[(x)-1] )
#define NOT_ON_LINE(x) (!IS_ON_LINE(x))


//set IR initial reading to zero
int IRvals[3] = {0};
int IRvalMin[3] = {0};
int IRvalMax[3] = {0};
int IRwhiteThreshold[3] = {IR_WHITE_THRESHOLD_DEFAULT , IR_WHITE_THRESHOLD_DEFAULT, IR_WHITE_THRESHOLD_DEFAULT};

int counterMax = 20; 
// Analog smoothing

const int numReadings = 5;
int readings[3][numReadings] = {0};      // the readings from the analog input
int sindex[3] = {0};                  // the index of the current reading
int total[3] = {0};                  // the running total
int average[3] = {0};                // the average for each IR


int counterL = 0;
int counterR = 0;
int counterT = 0;

void setup() {
  Serial.end();

  // Motor pins
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);
  setup_smoothing();
  radio_setup();
  delay(500);
  calibrateIRSensors();
}

void loop() {

  time_Now = millis(); //record time at start of loop
  radio_loop(); //read radio input
  readIRSensors(); //read IR sensors
  int userSpeed = zAxisValue + DEFAULT_CRUISE_SPEED;
  if (IRvals[1] >= IRwhiteThreshold[1])
  {
    isLineFollowingMode = true;
    calculatePID();
    
    if (userSpeed <= 0)
      userSpeed = 0;

    int cornerType = detectCornerType();

    if (cornerType ==  LINE_STATUS_T_JUNCTION)                // T-Junction
    {
      counterT++;
      if (counterT > counterMax) {
        counterT =  0;
        static int nextSend = millis()+ 2000;
        if (millis() > nextSend) {
          radio_send("LAP_START");
          nextSend = millis()+ 2000;
        }
      }
    }
  }
  else
  {
    isLineFollowingMode = false;
    userSpeed = 0;
    correction = 0;
  }

  speedA = (userSpeed + correction) + (yAxisValue / 2 - xAxisValue / 2);
  speedB = (userSpeed - correction) + (yAxisValue / 2 + xAxisValue / 2);
  motorSpeed(speedA, speedB);


}


// READ & PROCESS IR VALUES
void readIRSensors() {

  //analogRead(IR1_PIN);
  //delay(20);
  IRvals[0] = analogRead(IR1_PIN) ; //left looking from behind
  if (IRvals[0] >= 1000)
    IRvals[0] = 1000;

  //analogRead(IR2_PIN);
  IRvals[1] = analogRead(IR2_PIN) ; //centre
  if (IRvals[1] >= 1000)
    IRvals[1] = 1000;

  //analogRead(IR3_PIN);
  IRvals[2] = analogRead(IR3_PIN) ; //right
  if (IRvals[2] >= 1000)
    IRvals[2] = 1000;

  calculate_smoothing();
}

// CALCULATE PID
void calculatePID() {

  // Calculate PID on a regular time basis
  if ((time_Now - pidLastTime) < PID_SAMPLE_TIME ) {
    // return if called too soon
    return;
  }
  pidLastTime = time_Now;

  // process IR readings via PID
  //Kp = 30; Ki = 0; Kd = 300; //20,200 //35,300 for 50mm tail
  Kp = 50; Ki = 0; Kd = 500; //laser cut
  error_last = error; // store previous error value before new one is caluclated
  error = IRvals[0] - IRvals[2];
  P_error = error * Kp / 100.0; // calculate proportional term
  I_sum = constrain ((I_sum + error), -1000, 1000); // integral term
  I_error = I_sum * Ki / 100.0;
  D_error = (error - error_last) * Kd / 100.0;          // calculate differential term
  correction = P_error + D_error + I_error;
}


// MOTOR DRIVER
void motorSpeed(int _speedA, int _speedB) {

  _speedA = constrain(_speedA, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  _speedB = constrain(_speedB, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

  digitalWrite(MOTOR_A1_PIN, _speedA >= 0 ? HIGH : LOW) ;
  analogWrite (MOTOR_A2_PIN, abs(_speedA));

  digitalWrite(MOTOR_B1_PIN, _speedB >= 0 ? HIGH : LOW);
  analogWrite (MOTOR_B2_PIN, abs(_speedB));
}


// READ INPUT FROM JOYPAD
void joypad_update(int x, int y, int z, int b) {
  // If the axis readings are small, in the 'deadzone', set them to 0
  if ( abs(x) < JOYPAD_AXIS_DEADZONE)  x = 0;
  if ( abs(y) < JOYPAD_AXIS_DEADZONE)  y = 0;
  //if ( abs(z) < JOYPAD_AXIS_DEADZONE)  z = -255;

  xAxisValue = x;
  yAxisValue = y;
  zAxisValue = -(z - 255) / 2;
  buttonPressed = b;

  //radio_debug("%d,%d,%d,%d = %d,%d,%d,%d", x,y,z,b, xAxisValue,yAxisValue,zAxisValue,buttonPressed);
}

void settings_update(uint16_t whiteThreshold)  {
}




///////////////////////////////////////////////////////////////////////////////////////
// IR


void calibrateIRSensors() {
  radio_send_formatted("Calibrating IR...");
  // take intial readings and set the min/max as those.
  readIRSensors();
  IRvalMin[0] = IRvalMax[0] = IRvals[0];
  IRvalMin[1] = IRvalMax[1] = IRvals[1];
  IRvalMin[2] = IRvalMax[2] = IRvals[2];
  for (int i = -1; i <= 1; i += 1) {
    motorSpeed( i * DEFAULT_CRUISE_SPEED / 2, -(i * DEFAULT_CRUISE_SPEED / 2));
    for (int i = 0; i < 100; i++) {

      readIRSensors();
      for (int j = 0; j < 3; j++) {
        // find min/max
        if (IRvals[j]  < IRvalMin[j]) IRvalMin[j] = IRvals[j];
        if (IRvals[j]  > IRvalMax[j]) IRvalMax[j] = IRvals[j];
        delay(10);
      }

    }
  }
  IRwhiteThreshold[0] = IRvalMax[0] * IR1_SCALING;
  IRwhiteThreshold[1] = IRvalMax[1] * IR2_SCALING;
  IRwhiteThreshold[2] = IRvalMax[2] * IR3_SCALING;


  radio_send_formatted("----IR Calibration");
  delay(150);
  for  (int j = 0; j < 3; j++) {
    radio_send_formatted("IR%d:%d..%d= %d", j, IRvalMin[j],  IRvalMax[j],  IRvalMax[j] - IRvalMin[j]);
    delay(150);
  }

  radio_send_formatted("WT:%d,%d,%d", IRwhiteThreshold[0], IRwhiteThreshold[1], IRwhiteThreshold[2]);
  delay(150);

}


void setup_smoothing()
{
  for (int i = 0; i < 3; i++) {
    for (int thisReading = 0; thisReading < numReadings; thisReading++)
      readings[i][thisReading] = 0;

    IRvals[i] = 0;
    total[i] = 0;
    sindex[i] = 0;
    average[i] = 0;

  }
}

void calculate_smoothing() {
  for (int i = 0; i < 3; i++) {
    total[i] = total[i] - readings[i][sindex[i]];
    readings[i][sindex[i]] = IRvals[i];
    total[i] = total[i] + readings[i][sindex[i]];
    sindex[i] = sindex[i] + 1;

    if (sindex[i] >= numReadings)
      sindex[i] = 0;

    average[i] = total[i] / numReadings; /// map(total[i], IRvalMin[i], IRvalMax[i], 0, 1000)

  }
}

int detectCornerType() {

  if (IS_ON_LINE(1) && IS_ON_LINE(3) )                // T-Junction
  {
    return LINE_STATUS_T_JUNCTION;
  }
  else if (IS_ON_LINE(1) &&  NOT_ON_LINE(3) )          // Left Corner
  {
    return LINE_STATUS_LEFT_TURN;
  }
  else if (NOT_ON_LINE(1) &&  IS_ON_LINE(3) )         // Right Corner
  {
    return LINE_STATUS_RIGHT_TURN;
  }
  else if ( NOT_ON_LINE(1) && IS_ON_LINE(2) && NOT_ON_LINE(3))   // On the line
  {
    return LINE_STATUS_FOLLOWING_LINE;
  }
  else if ( NOT_ON_LINE(1) && NOT_ON_LINE(2) && NOT_ON_LINE(3))  // No line
  {
    return LINE_STATUS_NO_LINE;
  }
  else {
    return LINE_STATUS_UNKNOWN;
  }
}

void sendIRStats() {
  radio_send_formatted("--------IR State");
  delay(150);
  radio_send_formatted("raw:%d,%d,%d  ", IRvals[0], IRvals[1], IRvals[2]);
  delay(150);
  radio_send_formatted("ave:%d,%d,%d  ", average[0], average[1], average[2]);
  delay(150);
  radio_send_formatted("ol?:%d,%d,%d = %d", IS_ON_LINE(1), IS_ON_LINE(2), IS_ON_LINE(3), detectCornerType());
  delay(150);
  radio_send_formatted("--------Motor State");
  delay(150);
  radio_send_formatted("A,B=%d,%d  ", speedA, speedB);
}




