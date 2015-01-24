//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Maze Solver Drone
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  23.10.2014  -  Inital Version  (wayne@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////
//#define SERIAL_DEBUG

// ****** IMPORTANT: these bias must be ajusted so that all 3 radings are +/-2
//IR sensor bias
#if 0
#define IR1_BIAS -66
#define IR2_BIAS 0
#define IR3_BIAS -178
#else
#define IR1_BIAS 0
#define IR2_BIAS 0
#define IR3_BIAS 0
#endif

// ****** IMPORTANT: Very important this is tuned!

#define IR_WHITE_THRESHOLD_DEFAULT 850 //to determinie on line or not 

// PID gain setting
#define PID_P 50  //PID gains set below in the code.. 
#define PID_I 0
#define PID_D 400
#define PID_SAMPLE_TIME 5 //determine how fast the loop is executed




#define DEFAULT_CRUISE_SPEED 80
//Kp = 30; Ki = 0; Kd = 300; //20,200 //35,300 for 50mm tail
//Kp = 50; Ki = 0; Kd = 500; //laser cut

//#define   SERIAL_DEBUG            // enablign this will print message to serial and also disable the motors.

#define BOT_NAME "CannyMaze"                   // custom name (16 chars max)
#define GZLL_HOST_ADDRESS 0x99ACB010           // this needs to match the Client sketch value

#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>
void radio_send_formatted(char *fmt, ... );

///////////////////////////////////////////////////////////////////////////////////////
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
#define MOTOR_MAX_SPEED 250 // max speed (0..255)

// Line satus constants

#define LINE_STATUS_UNKNOWN           -1
#define LINE_STATUS_NO_LINE           0
#define LINE_STATUS_FOLLOWING_LINE    1
#define LINE_STATUS_RIGHT_TURN        2
#define LINE_STATUS_LEFT_TURN         3
#define LINE_STATUS_T_JUNCTION        4


// Helper Macros

//#define IS_ON_LINE(x) ( (x)>=IRwhiteThreshold )
//#define NOT_ON_LINE(x) (!IS_ON_LINE(x))




#define IS_ON_LINE(x) ( (average[(x)-1])>=IR##x##whiteThreshold )
#define NOT_ON_LINE(x) (!IS_ON_LINE(x))



//
volatile int IRwhiteThreshold = IR_WHITE_THRESHOLD_DEFAULT;


//set IR initial reading to zero
int IRval1 = 0;
int IRval2 = 0;
int IRval3 = 0;

int IRval1Min, IRval1Max, IR1whiteThreshold;
int IRval2Min, IRval2Max, IR2whiteThreshold;
int IRval3Min, IRval3Max, IR3whiteThreshold;

// Analog smoothing

const int numReadings = 10;
int readings[3][numReadings];      // the readings from the analog input
int sindex[3] = {0};                  // the index of the current reading
int total[3] = {0};                  // the running total
int average[3] = {0};                // the average for each IR





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
volatile int16_t  zAxisValue    = 100;


// Timers in milli-seconds (1/1000 of a second)
unsigned long time_Now = millis();                    // the time at the start of the loop()
unsigned long pidLastTime = millis();                // when the PID was calculated last

// motor speeds
int lineSpeed = 0;
int speedA = 0;
int speedB = 0;


///////////////////////////////////////////////////////////////////////////////////////
// Setup

void setup() {
#if defined(SERIAL_DEBUG)
  Serial.begin(9600);
  Serial.println("Cannybots Maze Solver Started in Serial Debug Mode- Motors are DISABLED!");
#else
  Serial.end();
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);
#endif  // Motor pins
  setup_smoothing();
  radio_setup();
  delay(500);
  calibrateIRSensors();
}



// READ & PROCESS IR VALUES
void readIRSensors() {

  //analogRead(IR1_PIN);
  //delay(20);
  IRval1 = analogRead(IR1_PIN) + IR1_BIAS; //left looking from behind
  if (IRval1 >= 1000)
    IRval1 = 1000;

  //analogRead(IR2_PIN);
  IRval2 = analogRead(IR2_PIN) + IR2_BIAS; //centre
  if (IRval2 >= 1000)
    IRval2 = 1000;

  //analogRead(IR3_PIN);
  IRval3 = analogRead(IR3_PIN) + IR3_BIAS; //right
  if (IRval3 >= 1000)
    IRval3 = 1000;
    
  calculate_smoothing();
}


// MOTOR DRIVER
void motorSpeed(int _speedA, int _speedB) {
#if defined(SERIAL_DEBUG)
  return;
#endif

  _speedA = constrain(_speedA, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  _speedB = constrain(_speedB, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

  digitalWrite(MOTOR_A1_PIN, _speedA >= 0 ? HIGH : LOW) ;
  analogWrite (MOTOR_A2_PIN, abs(_speedA));

  digitalWrite(MOTOR_B1_PIN, _speedB >= 0 ? HIGH : LOW);
  analogWrite (MOTOR_B2_PIN, abs(_speedB));
}



///////////////////////////////////////////////////////////////////////////////////////
// Radio



int lastStatus = LINE_STATUS_UNKNOWN;
void send_status(int status) {
  char c = '?';
  if (status != lastStatus) {
    lastStatus = status;
    switch (status) {
      case LINE_STATUS_UNKNOWN: c = '?';  break;
      case LINE_STATUS_NO_LINE: c = 'N';  break;
      case LINE_STATUS_FOLLOWING_LINE: c = 'O';  break;
      case LINE_STATUS_RIGHT_TURN: c = 'R';  break;
      case LINE_STATUS_LEFT_TURN: c = 'L';  break;
      case LINE_STATUS_T_JUNCTION: c = 'T';  break;
      default: c = 'x';  break;
    }

    radio_send_formatted("MAZE:%c,%d,%d,%d", c, IRval1, IRval2, IRval3);
  }

}

void received_client_disconnect () {
}



// do as little in here as possible as it's called under an interrupt.
void received_client_data(char *data, int len)  {
  // Commands: find line left, find line right, halt,
  // settings: p, i d, whitethress
  if (len > 0) {
    //radio_send_formatted("RCV:%d", data[0]);

    //if (data[0] == 'c')  calibrateIRSensors();
  }

  radio_send_formatted("MAZE:%d,%d,%d", IS_ON_LINE(1), IS_ON_LINE(2), IS_ON_LINE(3));

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
  error_last = error; // store previous error value before new one is caluclated
  error = IRval1 - IRval3;
  P_error = error * Kp / 100.0; // calculate proportional term
  I_sum = constrain ((I_sum + error), -1000, 1000); // integral term
  I_error = I_sum * Ki / 100.0;
  D_error = (error - error_last) * Kd / 100.0;          // calculate differential term
  correction = P_error + D_error + I_error;
}


void loop() {

  time_Now = millis(); //record time at start of loop
  radio_loop(); //read radio input
  readIRSensors(); //read IR sensors

  correction = 0;
  lineSpeed = 0;

  if ( IS_ON_LINE(2) )
  {
    if (IS_ON_LINE(1) && IS_ON_LINE(3) )
    {
      send_status(LINE_STATUS_T_JUNCTION);
    }
    else if (IS_ON_LINE(1) &&  NOT_ON_LINE(3) )
    {
      send_status(LINE_STATUS_LEFT_TURN);
    }
    else if (NOT_ON_LINE(1) &&  IS_ON_LINE(3) )
    {
      send_status(LINE_STATUS_RIGHT_TURN);
    } else {
      calculatePID();
      lineSpeed = DEFAULT_CRUISE_SPEED;
      send_status(LINE_STATUS_FOLLOWING_LINE);
    }
  } else
  {
    send_status(LINE_STATUS_NO_LINE);
  }

  speedA = (lineSpeed + correction);
  speedB = (lineSpeed - correction);
  motorSpeed(speedA, speedB);

  // TODO:  check IR sensors current value (and past?) to determine if we are at a junction/choice point


#if defined(SERIAL_DEBUG)
  Serial.print(IRval1, DEC);
  Serial.print("\t");
  Serial.print(IRval2, DEC);
  Serial.print("\t");
  Serial.print(IRval3, DEC);
  Serial.print("\n");
#endif

}



void calibrateIRSensors() {
  radio_send_formatted("MAZE:Calibrating...");
  // take intial readings and set the min/max as those.
  readIRSensors();
  IRval1Min = IRval1Max = IRval1;
  IRval2Min = IRval2Max = IRval2;
  IRval3Min = IRval3Max = IRval3;
  for (int i = -1; i <= 1; i += 1) {
    motorSpeed( i * DEFAULT_CRUISE_SPEED / 3, -(i * DEFAULT_CRUISE_SPEED / 3));
    for (int i = 0; i < 40; i++) {

      readIRSensors();
      // find minimums
      if (IRval1 < IRval1Min) IRval1Min = IRval1;
      if (IRval2 < IRval2Min) IRval2Min = IRval2;
      if (IRval3 < IRval3Min) IRval3Min = IRval3;

      // find maximums
      if (IRval1 > IRval1Max) IRval1Max = IRval1;
      if (IRval2 > IRval2Max) IRval2Max = IRval2;
      if (IRval3 > IRval3Max) IRval3Max = IRval3;
      delay(25);
    }
  }
  //IR1whiteThreshold = IRval1Max * 0.98;
  //IR2whiteThreshold = IRval2Max * 0.95;
  //IR3whiteThreshold = IRval3Max * 0.98;
  radio_send_formatted("MAZE:IR1,%d..%d=%d", IRval1Min, IRval1Max, IRval1Max - IRval1Min);
  delay(150);
  radio_send_formatted("MAZE:IR2,%d..%d=%d", IRval2Min, IRval2Max, IRval2Max - IRval2Min);
  delay(150);
  radio_send_formatted("MAZE:IR3,%d..%d=%d", IRval3Min, IRval3Max, IRval3Max - IRval3Min);
  delay(150);
  radio_send_formatted("MAZE:WT,%d,%d,%d", IR1whiteThreshold, IR2whiteThreshold, IR3whiteThreshold);
  delay(150);

}


void setup_smoothing()
{
  for (int i = 0; i < 3; i++) {
    for (int thisReading = 0; thisReading < numReadings; thisReading++)
      readings[i][thisReading] = 0;
  }
}

void calculate_smoothing() {
  int IRvals[3];
  IRvals[0]=IRval1;
  IRvals[1]=IRval2;
  IRvals[2]=IRval3;
  
  for (int i = 0; i < 3; i++) {
    total[i] = total[i] - readings[i][sindex[i]];
    readings[i][sindex[i]] = IRvals[i];
    total[i] = total[i] + readings[i][sindex[i]];
    sindex[i] = sindex[i] + 1;

    if (sindex[i] >= numReadings)
      sindex[i] = 0;
    average[i] = total[i] / numReadings;
  }
}

