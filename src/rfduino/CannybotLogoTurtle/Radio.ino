#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>
#include <stdarg.h>

// Choose one of:
//#define  RADIO_ONLY_GZLL 
//#define  RADIO_ONLY_BLE 
#define RADIO_TOGGLE 
//#define RADIO_NONE

// don't change these
#define GZLL_MAX_MSG_SIZE 32
char  gzllDebugBuf[GZLL_MAX_MSG_SIZE+1] = {0};
char* gzllDebug=NULL;

// don't change these
#define BLE_MAX_MSG_SIZE 20
#define BLE_UUID                   "7e400001-b5a3-f393-e0a9-e50e24dcca9e"

#define BLE_ADVERTISEMENT_DATA_MAX 16

// might want to alter these
#define BLE_TX_POWER_LEVEL                 0
#define TOGGLE_MILLIS                   1500
#define TOGGLE_GZLL_CONNECTION_TIMEOUT  1000



void radio_setup() {

#if defined(RADIO_ONLY_GZLL)
  setup_gzll();
#elif defined(RADIO_ONLY_BLE)
  setup_ble();
#elif defined(RADIO_TOGGLE)
  // Setup of BLE & GZLL is handled in radio_loop()
#elif defined(RADIO_NONE)
  // no radio chosen
#else
#error Invalid Radio config, choose: RADIO_ONLY_GZLL, RADIO_ONLY_BLE, RADIO_TOGGLE or RADIO_NONE
#endif
}


////////////////////////////////////////////////////////////////////////////////////////
// Radio toggling

#if defined(RADIO_TOGGLE) 
volatile bool startGZLL = true;
volatile unsigned long nextRadioToggleTime = millis();
#endif

#if defined(RADIO_TOGGLE) || defined(RADIO_ONLY_BLE)
volatile bool bleConnected = false;
#endif

#if defined(RADIO_TOGGLE) || defined(RADIO_ONLY_GZLL)
volatile bool gzllConnected = false;
volatile unsigned long gzllConnectionTimeout = millis();
#endif


void radio_loop() {
#if defined(RADIO_ONLY_GZLL) || defined(RADIO_TOGGLE)
  loop_gzll();
#endif  

#ifdef RADIO_TOGGLE
  if (!bleConnected && !gzllConnected) {
    if (millis() > nextRadioToggleTime) {
#ifdef DEBUG
      Serial.println("RADIO_TOGGLE");
#endif //DEBUG
      nextRadioToggleTime = millis() + TOGGLE_MILLIS;

      if (startGZLL) {
        while (RFduinoBLE.radioActive);
        RFduinoBLE.end();
        delay(50);
        setup_gzll();
      } else  {
        RFduinoGZLL.end();
        delay(50);
        setup_ble();
      }
      startGZLL   = !startGZLL;
    }
  }
#endif // RADIO_TOGGLE

}



////////////////////////////////////////////////////////////////////////////////////////
// GZLL

#if defined(RADIO_TOGGLE) || defined(RADIO_ONLY_GZLL)
void setup_gzll() {
#ifdef GZLL_HOST_ADDRESS  
  RFduinoGZLL.hostBaseAddress = GZLL_HOST_ADDRESS;
#endif  
  RFduinoGZLL.begin(HOST);    // default 
}

void loop_gzll() {
  // simulate a GZLL disconnect event
  return;
  if ( gzllConnected && (millis()  > gzllConnectionTimeout)) {
      gzllConnected = false;
      client_disconnected();
  }
}

void RFduinoGZLL_onReceive(device_t device, int rssi, char *data, int len) {
  gzllConnected = true;
  gzllConnectionTimeout = millis() + TOGGLE_GZLL_CONNECTION_TIMEOUT;
  process_message(data, len);
  if (gzllDebug) { 
    RFduinoGZLL.sendToDevice(device, gzllDebugBuf, strlen(gzllDebugBuf));
    gzllDebug=NULL;   
  } 
}
#endif
////////////////////////////////////////////////////////////////////////////////////////
// BLE

char bleName[BLE_ADVERTISEMENT_DATA_MAX] = {0};

void setup_ble() {
#ifndef BOT_NAME  
  snprintf(bleName, BLE_ADVERTISEMENT_DATA_MAX, "CB_%x%x", getDeviceIdHigh, getDeviceIdLow());
#else
  snprintf(bleName, BLE_ADVERTISEMENT_DATA_MAX, BOT_NAME);
#endif  
  RFduinoBLE.txPowerLevel      = BLE_TX_POWER_LEVEL;
#ifdef BLE_UUID  
  RFduinoBLE.customUUID        = BLE_UUID;
#endif  
  RFduinoBLE.deviceName        = bleName;
  RFduinoBLE.advertisementData = "CB01";
  RFduinoBLE.begin();
  //RFduinoBLE_update_conn_interval(20, 20);
}


void RFduinoBLE_onReceive(char *data, int len) {
  process_message(data, len);
}

void RFduinoBLE_onConnect() {
#if defined(RADIO_TOGGLE) || defined(RADIO_ONLY_BLE)
  bleConnected = true;
#endif
}

void RFduinoBLE_onDisconnect() {
#if defined(RADIO_TOGGLE) || defined(RADIO_ONLY_BLE)
  bleConnected = false;
#endif
  client_disconnected();
}

void _radio_debug(char* msg) {
#if defined(RADIO_TOGGLE) || defined(RADIO_ONLY_GZLL)  
    if (!gzllDebug && gzllConnected) {
     snprintf(gzllDebugBuf, GZLL_MAX_MSG_SIZE, msg);
     gzllDebug = gzllDebugBuf;
#if defined(SERIAL_DEBUG)    
      Serial.print("Will send...");
      Serial.println(msg);
#endif   
    }
#endif

#if defined(RADIO_ONLY_BLE) || defined(RADIO_TOGGLE)
    if (bleConnected) {
      RFduinoBLE.send(msg, min(BLE_MAX_MSG_SIZE, strlen(msg)));
    }
#endif    
}

void radio_send_formatted(char *fmt, ... ){
        char buf[GZLL_MAX_MSG_SIZE+1]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, GZLL_MAX_MSG_SIZE+1, fmt, args);
        va_end (args);
        _radio_debug(buf);
}



////////////////////////////////////////////////////////////////////////////////////////
// BLE/GZLL shared message processing

// We're expecting messages of at least 4 bytes in the form:  [SRC] [DST] [COMMMAND] [PARAMETER1]
// Where:
// SRC = uint8_t (ignored)
// DST = uint8_t (ignored)
// COMMMAND = uint8_t enum { s, f, b, l, r, u, d, a, v }  =>  stop, forward, back, left, right, pen up, pen down, audio(hoot), visual(LED effect) 
// PARAMETER1 = int16

void process_message(char *data, int len) {
  if (len >= 5) {
      logo_received_command(data[2], word(data[3], data[4]));
  } 
}

void radio_send(char* msg) {
  _radio_debug(msg);
}

// tidyup helper for when GZLL connection times out or BLE client disconnects
void client_disconnected() {
#if defined(SERIAL_DEBUG)    
    Serial.println("client_disconnected()");
#endif  
  logo_received_command('s', 0); // stop turtle
}




