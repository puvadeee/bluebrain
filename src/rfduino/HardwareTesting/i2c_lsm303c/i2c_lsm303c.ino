#include <Wire.h>
#include <Adafruit_LSM303.h>

Adafruit_LSM303 lsm;

void setup() 
{
  Serial.begin(9600);
  
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM303. Check your wiring!");
    while (1);
  }
}

void loop() 
{
  lsm.read();
  Serial.print("Accel X: "); Serial.print((int)lsm.accelData.x); Serial.print(" ");
  Serial.print("Y: "); Serial.print((int)lsm.accelData.y);       Serial.print(" ");
  Serial.print("Z: "); Serial.println((int)lsm.accelData.z);     Serial.print(" ");
  Serial.print("Mag X: "); Serial.print((int)lsm.magData.x);     Serial.print(" ");
  Serial.print("Y: "); Serial.print((int)lsm.magData.y);         Serial.print(" ");
  Serial.print("Z: "); Serial.println((int)lsm.magData.z);       Serial.print(" ");
  delay(1000);
}

/*
background:  https://learn.adafruit.com/lsm303-accelerometer-slash-compass-breakout/coding

basic lib:    https://github.com/adafruit/Adafruit_LSM303

'unified' lib:  https://github.com/adafruit/Adafruit_LSM303DLHC/blob/master/Adafruit_LSM303_U.cpp

difrent lib:    http://www.dfrobot.com/wiki/index.php?title=LSM303_Tilt_Compensated_Compass(SEN0079)


python

print  0b11011
27



*/
