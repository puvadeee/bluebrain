//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Blue Brain ufdffff- PingEcho Service (assumes unpopulated BlueBrain module)
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  28.11.2014  -  Inital Version  (wayne@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////

#define BOT_NAME "PingMockBot"                   // custom name (16 chars max)

//#define GZLL_HOST_ADDRESS 0x12ABCD00           // this needs to match the Joypad sketch value
//#define GZLL_HOST_ADDRESS 0x99ACB010    // TURTLE
//#define GZLL_HOST_ADDRESS 0x88ACB010    // Maze

#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>

void radio_send_formatted(char *fmt, ... );
int ardprintf(char *str, ...);

#define SEND(str, args...) radio_send_formatted(str, args); /*ardprintf(str, args);*/ delay(15);


static unsigned long lastPing = millis();
static unsigned long lastDataSample = millis();
static unsigned long lastFrame = millis();

unsigned long frame = 0;
int r, g, b, c;
int x, y, z;
int ir1, ir2, ir3, ir4;
int compass;
int batt;
uint8_t button = B00000010;

#define DEG2RAD(degrees) ( ( degrees * 71) / 4068)


void calculateMockNavData() {
  ardprintf("frame:%d", frame);
  Serial.println(DEG2RAD((frame * 4)));
  Serial.println(sin(DEG2RAD(frame * 4)));

  r = abs ( sin(DEG2RAD((float)frame)) * 255);
  g = abs ( sin(DEG2RAD((float)(frame + 30))) * 255);
  b = abs ( sin(DEG2RAD((float)(frame + 45))) * 255);
  c = abs ( sin(DEG2RAD((float)frame)) * 255);

  ir1 = abs ( sin(DEG2RAD((float)frame)) * 1023);
  ir2 = abs ( sin(DEG2RAD((float)(frame + 100))) * 1023);
  ir3 = abs ( sin(DEG2RAD((float)(frame + 200))) * 1023);
  ir4 = abs ( sin(DEG2RAD((float)frame)) * 1023); 

  x =  ( sin(DEG2RAD((float)frame)) * 64);
  y =  ( cos(DEG2RAD((float)(frame - 30))) * 255);
  z =  ( sin(DEG2RAD((float)(frame * 3))) * 192);

  button = ~button;
  batt = (frame * 10) % 1024;
  compass = sin(DEG2RAD((float)frame)) * 360;
}



void setup() {
  radio_setup();
  Serial.begin(9600);
}

void loop() {
  radio_loop();

  if (millis() - lastPing > 1000) {
    lastPing = millis();
    radio_send("ping!");
  } else if (millis() - lastDataSample > 100) {
    calculateMockNavData();
    SEND("RGB:%d,%d,%d,%d", r, g, b, c);
    SEND("IR:%d,%d,%d,%d", ir1, ir2, ir3, ir4);
    SEND("ACL:%d,%d,%d", z, y, x);
    SEND("CMP:%d", compass);
    SEND("BAT:%d", batt);
    SEND("BTN:%d", button);
    lastDataSample = millis();
  }

  if (millis() - lastFrame > 25) {
    lastFrame = millis();
    frame++;
  }
}



void received_client_connect()
{
  Serial.println(F("* Connected!"));
}


void  received_client_disconnect() {
  Serial.println(F("* Disconnected"));
}

void received_client_data(char *data, int len)  {
  Serial.println(F("* receviedMessage!"));

}



// see: http://arduino.stackexchange.com/questions/176/how-do-i-print-multiple-variables-in-a-string
#define ARDBUFFER 16
int ardprintf(char *str, ...)
{
  int i, count = 0, j = 0, flag = 0;
  char temp[ARDBUFFER + 1];
  for (i = 0; str[i] != '\0'; i++)  if (str[i] == '%')  count++;

  va_list argv;
  va_start(argv, count);
  for (i = 0, j = 0; str[i] != '\0'; i++)
  {
    if (str[i] == '%')
    {
      temp[j] = '\0';
      Serial.print(temp);
      j = 0;
      temp[0] = '\0';

      switch (str[++i])
      {
        case 'd': Serial.print(va_arg(argv, int));
          break;
        case 'l': Serial.print(va_arg(argv, long));
          break;
        case 'f': Serial.print(va_arg(argv, double));
          break;
        case 'c': Serial.print((char)va_arg(argv, int));
          break;
        case 's': Serial.print(va_arg(argv, char *));
          break;
        default:  ;
      };
    }
    else
    {
      temp[j] = str[i];
      j = (j + 1) % ARDBUFFER;
      if (j == 0)
      {
        temp[ARDBUFFER] = '\0';
        Serial.print(temp);
        temp[0] = '\0';
      }
    }
  };
  Serial.println();
  return count + 1;
}
