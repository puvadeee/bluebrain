//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Serial Proxt (GZLL Client)
//
// Author:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0   -  14.08.2014  -  Inital Version
//
//////////////////////////////////////////////////////////////////////////////////////////////////

// on PI:
// Remove from /boot/cmdline.txt:
// Remove from /etc/inittab:             #T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100

// Wire  Bluebrain   GPIO3 / IR2  to  RPi TX
// Wire  Bluebrain   GPIO4 / IR3  to  RPi RX
// Wire  Bluebrain   VCC          to  RPi 3.3v
// Wire  Bluebrain   GND          to  RPi GND

// On PI:  minicom -b 115200 -o -D /dev/ttyAMA0
// 00?00
// Quit:   CTRL-A  X

//#define GZLL_HOST_ADDRESS 0x99ACB010           // this needs to match the MazeSolver sketch value
//#define GZLL_HOST_ADDRESS 0x88ACB010           // this needs to match the Turtle sketch value

#define GZLL_HOST_ADDRESS 0x12ACB010
//#define SERIAL_DEBUG

#define GZLL_MAX_MSG_SIZE 20      // keep it the same a BLE, otherwise could be 32

#define USE_DEFUALT_UART_PINS

#if defined(USE_DEFUALT_UART_PINS)
#define RX_PIN 0
#define TX_PIN 1
#else
#define RX_PIN 2
#define TX_PIN 3
#endif

#include <RFduinoGZLL.h>


char  serialPrintBuffer[GZLL_MAX_MSG_SIZE + 1] = {0};
char* radioMessageReceived = NULL;

device_t role = DEVICE0;

void setup()
{
  Serial.begin(115200, RX_PIN, TX_PIN);
#ifdef GZLL_HOST_ADDRESS
  RFduinoGZLL.hostBaseAddress = GZLL_HOST_ADDRESS;
#endif
  RFduinoGZLL.begin(role);

 Serial.println("SerialProxy:STARTED!");
}

void loop()
{
  // keep alive ping
  static unsigned long nextPingTime = 0;
  if (millis() > nextPingTime) {
    RFduinoGZLL.sendToHost((const char*)serialPrintBuffer, 0);
    nextPingTime = millis() + 100;
  }
  process_serial2radio();

  // for GZLL check for returned messages
  if (radioMessageReceived) {
    Serial.println(serialPrintBuffer);
    Serial.flush();
    radioMessageReceived = NULL;
  }


}
// Process incomming serial data

void process_serial2radio() {
  static uint8_t serialBuffer[GZLL_MAX_MSG_SIZE];
  static int serialBufPtr = 0;
  static bool foundNewLine = false;
  static char c = 0, lastChar = 0;

  while (Serial.available() > 0) {
    lastChar = c;
    c =  Serial.read();
#if defined(SERIAL_DEBUG)

    Serial.print("Char=");
    Serial.println(c, HEX);
#endif
    if ( (0x0D == c) ) { /*&& ('\r' == lastChar) )*/
      foundNewLine = true;
    }
    // if max buf exceeded send line by assuming newline was found.
    if (serialBufPtr >= GZLL_MAX_MSG_SIZE) {
      foundNewLine = true;
    }

    if (!foundNewLine && (serialBufPtr < GZLL_MAX_MSG_SIZE) && (c!='\r')&& (c!='\n') ) {
#if defined(SERIAL_DEBUG)
      Serial.print("serialBuffer[");
      Serial.print(serialBufPtr, DEC);
      Serial.print("]=");
      Serial.println(c, HEX);
#endif
      serialBuffer[serialBufPtr++] = c;
    }


    if (foundNewLine) {
      if (serialBufPtr > 0) {
#if defined(SERIAL_DEBUG)
        Serial.print("SENDING! len=");
        Serial.println(serialBufPtr, DEC);
#endif
        RFduinoGZLL.sendToHost((const char*)serialBuffer, serialBufPtr);
      }
      foundNewLine = false;
      serialBufPtr = 0;
    }
  }

}



// process incoming radio data
void RFduinoGZLL_onReceive(device_t device, int rssi, char *data, int len)
{
  if (len > 0)
  {
    if (!radioMessageReceived) {
      memcpy( serialPrintBuffer, data, min(sizeof(serialPrintBuffer) - 1, len));
      serialPrintBuffer[min(sizeof(serialPrintBuffer) - 1, len)] = 0;
      radioMessageReceived = serialPrintBuffer;
    }
  }
}

