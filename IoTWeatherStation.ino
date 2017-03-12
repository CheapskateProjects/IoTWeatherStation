/*
  IoT Weather station

  Measures temperature and humidity with DHT11 sensor. Also measures air dust content with GP2Y1010AU0F dust sensor. 

  Measured values are passed into AWS IoT cloud though IoT gatewa with serial message. 
  
  created   Mar 2017
  by CheapskateProjects

  ---------------------------
  The MIT License (MIT)

  Copyright (c) 2017 CheapskateProjects

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <dht.h>
dht DHT;
#define DHT11_PIN 5

// Analog pin to measure dust sensor voltage
int dustMeasurePin = A5;
// Digital pin to enable dust led
int dustLedPin = 12;
// From datasheet: 0.28ms
unsigned int samplingTime = 280;
// Datasheet: pulse width = 32ms. 32-28=4ms
unsigned int deltaTime = 40;

float dustAnalogValue = 0;
float measuredVoltage = 0;
float dustDensity = 0;
float voc = 0.9; // This may change based on dust attached to the sensor. Use this to calibrate results. 0.9 is typical voltage based on datasheet
float voltageToDensity = 0.17; // This is based on the flat part of the curve presented in datasheet.

void setup()
{
  Serial.begin(115200);
  pinMode(dustLedPin,OUTPUT);
  delay(10);
  Serial.println();
  delay(5000);
}

float getDustDensity()
{
  // Led on
  digitalWrite(dustLedPin,LOW);
  // Wait until optimal measurement time given by data sheet
  delayMicroseconds(samplingTime);
  // Take sample
  dustAnalogValue = analogRead(dustMeasurePin);
  // Wait until pulse width end
  delayMicroseconds(deltaTime);
  // Led off
  digitalWrite(dustLedPin,HIGH);
  // Change analog value to voltage (datasheet dust graph is voltage based)
  measuredVoltage = dustAnalogValue*(5.0/1024);// Uno is 5V platform. Use 3.0 for 3V platforms

  // Instructions from data sheet
  if ( measuredVoltage < voc)
  {
    voc = measuredVoltage;
  }
  
  dustDensity = voltageToDensity * (measuredVoltage-voc);// mg/m^3
  
  return dustDensity;
}

void loop()
{
  // READ DATA
  DHT.read11(DHT11_PIN);
  
  String tmes = "WeatherStation1;{\"state\":{\"reported\":{\"temperature\":";
  tmes += DHT.temperature;
  tmes += ", \"humidity\":";
  tmes += DHT.humidity;
  tmes += ", \"dust_density\":";
  tmes += getDustDensity();
  tmes += "}}}";

  Serial.println(tmes);

  delay(30000);
}
