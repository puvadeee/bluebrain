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
#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>
void radio_send_formatted(char *fmt, ... );
//#define   SERIAL_DEBUG            // enablign this will print message to serial and also disable the motors.

// Notes:


// Function that are almost unchanged and can be considred the same as before:  setup, readIRSensors, motorSpeed & calculatePID

// Significant changes:
//   loop -                           first pass at a discriminator for left/right/t-junction,
//                                    notifies client of state (determined corner) and stops if not on the line.
//                                    uses smoothed IR values and a counter
//

// New functions:

//  calibrateIRSensors():              a simple IR sensor min/max (bot must be place onm the line on powerup)
//                                     1. bot spins round and records min & max values read for each IR sensor
//                                     2a) IR1 & IR3 have their whitethreshol set to 95% of max
//                                     2b) IR2 has it own whitethreshold set to 90% of max

//  send_status :                      notifies client of a trasistion betwween line discrinatorstates, e.g. LINE_STATUS_FOLLOWING_LINE, LINE_STATUS_RIGHT_TURN
//                                     surpresses duplicates

//  received_client_disconnect :       called when client radio disconneccts

//  received_client_data:              over the air data, raw input, not filterd or processed by Radio.ino  (unlike the Joypad stuf fin the racer)

//  calculate_smoothing:               perform a rolling average filter, used byt the IS_ON_LINE/NOT_ON_LINE helpers
//                                     smoothing is applied to the IR vals for the discriminator in loop(), however the orginal raw (unsmoothed) values are still used by the PID loop.

//
// Helpers
//  IS_ON_LINE(irNum:1-3) / NOT_ONLINE(irNum:1-3)        uses the smoothed values to determine is sensor x is on or of the line.


// Tuning

// IR

// Scale factor to apply to IRmax values detected during calibration; result is used to set the whitethreshold level individually for each IR sensor.
#define IR1_SCALING 0.98
#define IR2_SCALING 0.90
#define IR3_SCALING 0.98


// PID gain setting
//Kp = 30; Ki = 0; Kd = 300; //20,200 //35,300 for 50mm tail
//Kp = 50; Ki = 0; Kd = 500; //laser cut

#define PID_P 50
#define PID_I 0
#define PID_D 400

#define PID_SAMPLE_TIME 5                       //determine how fast the loop is executed (millis seconds)


int counterMax = 20;                            // number of samples (loop() cycles) to take before IR corner detection decides it's found a corner match
// or put another way, how far 'into' the corner to stop, ideally stop half over the intesection way
// so turing left or right lines results in IR2 being on the center of the line, ideally...s


#define DEFAULT_CRUISE_SPEED 40                // default speed when line following (calibration speed uses 50% of this)
#define DEFAULT_TURN_SPEED   40
#define MOTOR_IDLE_TIME 1000                   // number of samples (loop() cycles) to wait to determine if bot is halted, on halt detection the bot sends corner type 


// BLE name and Gazelle ID

#define BOT_NAME "CannyMaze"                   // custom name (16 chars max)
#define GZLL_HOST_ADDRESS 0x99ACB010           // this needs to match the Client sketch value



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

// Line status constants

#define LINE_STATUS_UNKNOWN           -1
#define LINE_STATUS_NO_LINE           0
#define LINE_STATUS_FOLLOWING_LINE    1
#define LINE_STATUS_RIGHT_TURN        2
#define LINE_STATUS_LEFT_TURN         3
#define LINE_STATUS_T_JUNCTION        4


// Helper Macros
#define IS_ON_LINE(x) ( (average[(x)-1])>=IRwhiteThreshold[(x)-1] )
#define NOT_ON_LINE(x) (!IS_ON_LINE(x))


//set IR initial reading to zero
int IRvals[3] = {0};
int IRvalMin[3] = {0};
int IRvalMax[3] = {0};
int IRwhiteThreshold[3] = {0};


// Analog smoothing

const int numReadings = 5;
int readings[3][numReadings] = {0};      // the readings from the analog input
int sindex[3] = {0};                  // the index of the current reading
int total[3] = {0};                  // the running total
int average[3] = {0};                // the average for each IR


int counterL = 0;
int counterR = 0;
int counterT = 0;

int lastStatus = LINE_STATUS_UNKNOWN;


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


// Timers in milli-seconds (1/1000 of a second)
unsigned long time_Now = millis();                    // the time at the start of the loop()
unsigned long pidLastTime = millis();                // when the PID was calculated last

// motor speeds
int lineSpeed = 0;
int speedA = 0;
int speedB = 0;

//

static bool sendIRStatsFlag = 0;



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
// CALCULATE PID

int calculatePID() {

  // Calculate PID on a regular time basis
  if ((time_Now - pidLastTime) < PID_SAMPLE_TIME ) {
    // just return previous value if called too soon
    return correction;
  }
  pidLastTime = time_Now;

  // process IR readings via PID
  error_last = error; // store previous error value before new one is caluclated
  error = IRvals[0] - IRvals[2];
  P_error = error * Kp / 100.0; // calculate proportional term
  I_sum = constrain ((I_sum + error), -1000, 1000); // integral term
  I_error = I_sum * Ki / 100.0;
  D_error = (error - error_last) * Kd / 100.0;          // calculate differential term
  correction = P_error + D_error + I_error;
  return correction;
}


///////////////////////////////////////////////////////////////////////////////////////
// Radio


void received_client_disconnect () {
}

bool turnLeftFlag = false;
bool turnRightFlag = false;
bool haltFlag = false;
bool calibrateIRFlag = false;

// do as little in here as possible (e..g just set flags)
// as it's called under an interrupt and must return quickly.
void received_client_data(char *data, int len)  {
  // Commands: find line left, find line right, halt,
  // settings: p, i d, whitethress
  if (len > 0) {
    switch (data[0]) {
      case 'i':  sendIRStatsFlag = 1; break;
      case 'l':  turnLeftFlag = true; break;
      case 'r':  turnRightFlag = true; break;
      case 's':  haltFlag = true; break;
      case 'c':  calibrateIRFlag = true; break;
      default:
        radio_send("CMD?");
        return;
        break;
    }
  }
}

void run_commands() {
  if (sendIRStatsFlag) {
    sendIRStatsFlag = false;
    sendIRStats();
    radio_send("OK");
  }

  if (turnLeftFlag) {
    turnLeftFlag = false;
    turn_left();
    radio_send("OK");

  }

  if (turnRightFlag) {
    turnRightFlag = false;
    turn_right();
    radio_send("OK");

  }

  if (haltFlag) {
    haltFlag = false;
    motorSpeed(0, 0);
    radio_send("OK");
  }

  if (calibrateIRFlag) {
    calibrateIRFlag = false;
    calibrateIRSensors();
    radio_send("OK");
  }
}


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

    radio_send_formatted("MAZE:%c,%d,%d,%d", c, IRvals[0], IRvals[1], IRvals[2]);
  }
}


///////////////////////////////////////////////////////////////////////////////////////
// Main Loop


void loop() {

  time_Now = millis(); //record time at start of loop
  radio_loop(); //read radio input
  readIRSensors(); //read IR sensors


  if ( IS_ON_LINE(2) )
  {
    int cornerType = detectCornerType();

    if (cornerType ==  LINE_STATUS_T_JUNCTION)                // T-Junction
    {
      counterT++;
      if (counterT > counterMax) {
        counterT = counterR = counterL = 0;
        correction = 0;
        lineSpeed = 0;
      }
    }
    else if ( cornerType == LINE_STATUS_LEFT_TURN  )          // Left Corner
    {
      counterL++;
      if (counterL > counterMax) {
        counterT = counterR = counterL = 0;
        correction = 0;
        lineSpeed = 0;
      }
    }

    else if ( cornerType == LINE_STATUS_RIGHT_TURN)         // Right C
    {
      counterR++;
      if (counterR > counterMax) {
        counterT = counterR = counterL = 0;
        correction = 0;
        lineSpeed = 0;
      }
    } else {                                            // Line Following
      correction = calculatePID();
      lineSpeed = DEFAULT_CRUISE_SPEED;
      send_status(LINE_STATUS_FOLLOWING_LINE);
    }
  } else
  {
    correction = 0;
    lineSpeed = 0;
    send_status(LINE_STATUS_NO_LINE);                    // Off the line
  }

  //if speeds are 0 (or a 'cotner found flag set) then check line status and send thestat just here instead, seems the status is ok after stopping!
  speedA = (lineSpeed + correction);
  speedB = (lineSpeed - correction);
  motorSpeed(speedA, speedB);

  updateIdleTimer();

  run_commands();
}

//////////////////////////////////////////////////////////////////////////////////////
// Movement


bool idleTimerTriggered = false;
int idleCount = 0;

void updateIdleTimer() {
  if ( (speedA == 0) && (speedB == 0) ) {
    idleCount++;
  } else {
    idleCount = 0;
    idleTimerTriggered = false;
    return;
  }

  if (idleTimerTriggered)
    return;

  if (idleCount > MOTOR_IDLE_TIME) {
    idleTimerTriggered = true;
    send_status(detectCornerType());
    delay(150);
  }
}


void turn_left() {
  motorSpeed(DEFAULT_TURN_SPEED * 1.5, -DEFAULT_TURN_SPEED / 2);
  while (detectCornerType() != LINE_STATUS_FOLLOWING_LINE) {
    readIRSensors();
  }
  motorSpeed(0, 0);
}

void turn_right() {
  motorSpeed(-DEFAULT_TURN_SPEED / 2, DEFAULT_TURN_SPEED * 1.5);
  while (detectCornerType() != LINE_STATUS_FOLLOWING_LINE) {
    readIRSensors();
  }
  motorSpeed(0, 0);
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
  radio_send_formatted("A,B,Idle:%d,%d,%d  ", speedA, speedB, idleCount);
}


/*
///////////////////////////////////////////////////////////////////////////////////////
// Data Sampling, for future use...

// stats collection
#define SAMPLES_MAX 50
#define SAMPLES_PER_SECOND 20
typedef struct __attribute__((__packed__))  sample_t{
  int ir1_raw:10, ir2_raw:10, ir3_raw:10;
  int ir1_ave:10, ir2_ave:10, ir3_ave:10;
  int speedA:9,speedB:9;
//  unsigned int timestamp;
} sample;
sample_t sampleBuffer[sizeof(sample_t)*SAMPLES_MAX] = {0};
int  sampleOffset = 0;
int  sampleFrequency=1000/SAMPLES_PER_SECOND;

void sampling_update()  {
  static unsigned long nextUpdate = millis() + sampleFrequency;
  if (nextUpdate < millis())
    return;

  sampleBuffer[sampleOffset].ir1_raw=IRvals[0];
  sampleBuffer[sampleOffset].ir2_raw=IRvals[1];
  sampleBuffer[sampleOffset].ir3_raw=IRvals[2];
  sampleBuffer[sampleOffset].ir1_ave=average[0];
  sampleBuffer[sampleOffset].ir2_ave=average[1];
  sampleBuffer[sampleOffset].ir3_ave=average[2];
  sampleBuffer[sampleOffset].speedA=speedA;
  sampleBuffer[sampleOffset].speedB=speedB;

  sampleOffset = (sampleOffset + 1) % SAMPLES_MAX;
  if (sampleOffset == 0) {
     //we've looped, send samples data if switch on (will hold up main loop/PID/line following)
  }
}
*/
