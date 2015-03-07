#include <Wire.h>
#include <RFduinoBLE.h> 
#include <Adafruit_NeoPixel.h>
#include <Adafruit_TCS34725.h>


//#define USE_NEOPIXEL 1

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
//Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_1X);

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
    Serial.println(gammatable[i]);
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
  
#ifdef  USE_NEOPIXEL
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
#endif  
}

void loop() {
  uint16_t red, green, blue, clear, colorTemp, lux;

  delay(10);
  tcs.getRawData(&red, &green, &blue, &clear);
  colorTemp = tcs.calculateColorTemperature(red, green, blue);
  lux = tcs.calculateLux(red, green, blue);
  
  uint32_t sum = clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  
#ifdef USE_NEOPIXEL  
  uint32_t colour = strip.Color(gammatable[(int)r], gammatable[(int)g], gammatable[(int)b]);
  strip.setPixelColor(1, colour);
  strip.show();
#endif  
  
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(red, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(green, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(blue, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(clear, DEC); Serial.print(" ");

  Serial.print("R: "); Serial.print( gammatable[(int)r], DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print( gammatable[(int)g], DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print( gammatable[(int)b], DEC); Serial.print(" ");
  Serial.println(" ");
}


