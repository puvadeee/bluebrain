#include <Wire.h>
#include <RFduinoBLE.h> 
#include <Adafruit_NeoPixel.h>
#include <Adafruit_TCS34725.h>

// set to false if using a common cathode LED
#define commonAnode true

/* Connect Sensor VCC    
   Connect In     to sensor bar IR 2   (pin 2)
   Connect GROUND to sensor bar IR 3   (pin 3)
   Connect VDD    to sensor bar VCC    (pin 4)*/
#define PIN 3

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

#ifdef USE_NEOPIXEL
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
#endif

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */
   
/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

// our RGB -> eye-recognized gamma color
byte gammatable[256];


void setup() {
  Serial.begin(9600);
  
  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
      
    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;      
    }
    //Serial.println(gammatable[i]);
  }
  
  
  
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  
  
  // create a ground.
  pinMode(2, INPUT);
  digitalWrite(2, LOW);
  
  //strip.begin();
  //strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  uint16_t r, g, b, c, colorTemp, lux;
  
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);
  
#ifdef USE_NEOPIXEL  
  uint32_t colour = strip.Color(gammatable[(int)r], gammatable[(int)g], gammatable[(int)b]);
  strip.setPixelColor(1, c);
  strip.show();
#endif  
  
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");

  Serial.print("R: "); Serial.print( gammatable[(int)r], DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print( gammatable[(int)g], DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print( gammatable[(int)b], DEC); Serial.print(" ");
  Serial.println(" ");
}


