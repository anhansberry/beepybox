/* 
This code controls the BeepyBox, a small, affordable, and easy to build device that can help you maintain social distance and alert others who might not be so careful.
Copyright (C) 2020  Hansberry & Heise

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Be sure to include this library as a zip in your Arduino IDE too
#include <NewPing.h>

// Define pins, but make sure these match your hardware
#define trigPin  3
#define echoPin  2
#define buzzerPin 9
#define ledPin 10
#define sensorPower 4

// Define distances, you can change these as you see fit
#define MAX_DISTANCE 350 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define DANGER_RANGE 80 // Threshold for distance in cm that is considered too close!
#define REPEAT_READS 4 // The number of times a danger distance must be read in a row to turn on alert mode

#define DEBUG 1 // Change this to 0 if you do NOT want the Serial outputs to print

// Variables used throughout the program, you should be able to leave these be
NewPing sonar(trigPin, echoPin, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
float duration, distance;
float lastDistance;

int numCloseReads;
int numFarReads;

unsigned long currentMillis;
unsigned long prevMillisSensor;
unsigned long prevMillisLED;

bool alert = false;
int ledState = LOW;

// Initialization code for when the program starts
void setup() {
  Serial.begin(9600); // Open serial monitor at 9600 baud to see ping results.

  // we use this pin as another power pin, see below
  pinMode(sensorPower, OUTPUT);

  // setup the board LEDs so we can turn them off later
  pinMode(LED_BUILTIN, OUTPUT);

  // setup the buzzer and LED as outputs
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
}

/*
 * A function to blink our led for a specific number of seconds, in order 
 * to avoid using a blocking delay call in the loop
 * Note: This is hard coded to use our predefined pin and timer for simplicity,
 * you'd need to change that if you want to have multiple LEDs.
 */
void blinkLED(int ms) {
  if (currentMillis - prevMillisLED >= ms) {
    // save the last time you blinked the LED
    prevMillisLED = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}

/*
 * The main loop of the program. All of the sensing and reacting to inputs
 * happens down here in this loop!
 */
void loop() {

  // turn off the board LEDs to save power!
  digitalWrite(LED_BUILTIN, LOW);

  // send power to an extra pin to power the sensor
  // not Best Practice, but this worked best for our hardware setup
  digitalWrite(sensorPower, HIGH);

  currentMillis = millis();

  // get a reading from the sensor as fast as possible, every 29 ms
  // the code avoids using the blocking delay call by doing timing manually
  if (currentMillis - prevMillisSensor >= 29) { // Wait 29ms, which should be the shortest delay between pings

    // save the last time you checked the distance, so you can know when 29 ms has elapsed
    prevMillisSensor = currentMillis;

    // uses the NewPing library to read 5 times in succession and give the median
    int iterations = 5;
    duration = sonar.ping_median(iterations);
    distance = (duration / 2) * 0.0343;

    // Below, we keep track of whether someone is in the danger range 
    // for multiple reads in a row, to help cut down on false positives

    // if someone is recognized within the danger range
    if (distance < DANGER_RANGE) {
      numCloseReads++;
      if (numCloseReads > REPEAT_READS) { // if they've been in the danger range for 5 reads now
        alert = true; // alert
      }
    } else {
      numFarReads++; // not currently in the danger range
      if (numFarReads > REPEAT_READS) { // if that remains true for 5 reads
        alert = false; // you're all good againd
        numCloseReads = 0;
        numFarReads = 0;
      }
    }
  }

  // Control the buzzer and led based on whether or not someone is in the danger zone
  if (alert) {
    tone(buzzerPin, 1000, 50); // Send 1KHz sound signal for 500ms
    blinkLED(100); // blink the LED at 100ms rate
  } else {
    noTone(buzzerPin);
    digitalWrite(ledPin, LOW);
  }

  // This will show up in the Serial Monitor when the DEBUG define is set to 1
  // To turn it off, set it to 0
  if (DEBUG) {
    Serial.print("Distance = ");
    Serial.print(distance); // Distance will be 0 when out of set max range.
    Serial.println(" cm");

    Serial.print("Alert mode? = ");
    Serial.println(alert);
  }
}
