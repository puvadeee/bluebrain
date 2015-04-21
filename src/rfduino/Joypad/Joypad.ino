//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Joypad
//
// Author:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0   -  14.08.2014  -  Inital Version 
// Version:   1.1   -  14.08.2014  -  Inital Version 
//
//////////////////////////////////////////////////////////////////////////////////////////////////

#define GZLL_HOST_ADDRESS 0x22ACB010           // this needs to match the Bot sketch value
//#define LOCAL_DEBUG
#define REMOTE_DEBUG

#include <RFduinoGZLL.h>

#define X_AXIS_PIN 3    // joypad x
#define Y_AXIS_PIN 2    // joypad y
#define Z_AXIS_PIN 5    // wheel throttle
#define BUTTON_PIN 4    // joypad button 
#define MSG_LEN 4

#define GZLL_MAX_MSG_SIZE 32
char  gzllDebugBuf[GZLL_MAX_MSG_SIZE+1] = {0};
char* dbgMsg = NULL;


device_t role = DEVICE0;
char msg[MSG_LEN+1] = {0};  // 4 bytes  = 4 bytes data 1 byte NULL terminator

void setup()
{
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
#ifdef GZLL_HOST_ADDRESS  
  RFduinoGZLL.hostBaseAddress = GZLL_HOST_ADDRESS;
#endif  
  RFduinoGZLL.begin(role);
}

void loop()
{
  bool buttonPressed = digitalRead(BUTTON_PIN)  & 0xFF;
  uint8_t xAxis = 255-(analogRead(X_AXIS_PIN)/4) ;          // scale from 0..1023 to 0-255 so data fits in a byte
  uint8_t yAxis = 255-(analogRead(Y_AXIS_PIN)/4) ;
  uint8_t zAxis = 255-(analogRead(Z_AXIS_PIN)/4) ;
  
  snprintf(msg, MSG_LEN+1, "%c%c%c%c", xAxis, yAxis, buttonPressed, zAxis );
  
  RFduinoGZLL.sendToHost((const char*)msg, MSG_LEN);      // don't bother sending the NULL byte at the end
#ifdef LOCAL_DEBUG
  //Serial.write((uint8_t*)msg, MSG_LEN);
  Serial.print(xAxis);
  Serial.print("\t");  
  Serial.print(yAxis);
  Serial.print("\t");  
  Serial.print(zAxis);
  Serial.print("\t");  
  Serial.println(buttonPressed);
#endif

  delay(50);
#ifdef REMOTE_DEBUG
  if (dbgMsg) {
      Serial.println(dbgMsg);
      dbgMsg=NULL;   
  }    
#endif 
}

void RFduinoGZLL_onReceive(device_t device, int rssi, char *data, int len)
{
  if (len > 0)
  {
    if (!dbgMsg) {
      memcpy( gzllDebugBuf, data, min(sizeof(gzllDebugBuf)-1,len));
      gzllDebugBuf[min(sizeof(gzllDebugBuf)-1,len)]=0;
      dbgMsg=gzllDebugBuf;      
    }
  }
}

