#include <YouveBeenNotified.h> 
#include <Servo.h>

#include "RTC.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

// --- Servo + Notifier Setup ---
Servo drum;
ServoNotifier beatMaker(drum);  // Uses YouveBeenNotified

// --- LED Matrix + RTC Setup ---
ArduinoLEDMatrix matrix;
boolean showDebug = true;

// RTC Time Vars
int currentMinute;
int currentSecond;
int lastMinute = -1;
int lastSecond = -1;
int countDirection = 1; // 1 = countdown from 60
int updateInterval = 1;

// ------------------ SETUP ------------------
void setup() {
  Serial.begin(9600);

  // Servo setup
  drum.attach(9);

  // LED Matrix + RTC setup
  matrix.begin();
  RTC.begin();

  // Optional: Set start time
  RTCTime initialTime(04, Month::APRIL, 2025, 10, 0, 0, DayOfWeek::FRIDAY, SaveLight::SAVING_TIME_ACTIVE);
  RTC.setTime(initialTime);

  // Keyframe Animation: Bidirectional sweep over 2 minutes total (1 min per direction)
  KeyframeAnimation pingPong("pingPong");

  const int step = 2;
  const int sweepDuration = 60000; // 1 minute per direction
  const int totalSteps = 180 / step;

  // Forward sweep
  for (int i = 0; i <= totalSteps; i++) {
    int angle = i * step;
    int time = map(i, 0, totalSteps, 0, sweepDuration);
    pingPong.addKeyFrame(angle, time);
  }

  // Backward sweep
  for (int i = totalSteps; i >= 0; i--) {
    int angle = i * step;
    int time = sweepDuration + map(totalSteps - i, 0, totalSteps, 0, sweepDuration);
    pingPong.addKeyFrame(angle, time);
  }

  beatMaker.addAnimation(pingPong);
  beatMaker.playAnimation("pingPong", LOOP);

  Serial.println("Servo + RTC Countdown Initialized");
  displayMinute(); // Show initial countdown
}

// ------------------ LOOP ------------------
void loop() {
  // Servo animation update
  beatMaker.update();
  if (beatMaker.hasChanged()) {
    drum.write(beatMaker.getValue());
  }

  // RTC Time Polling
  RTCTime currentTime;
  RTC.getTime(currentTime);

  currentMinute = currentTime.getMinutes();
  currentSecond = currentTime.getSeconds();

  // Every second: Debug
  if (currentSecond != lastSecond) {
    if (showDebug) {
      Serial.print("DEBUG Time: ");
      Serial.print(currentMinute);
      Serial.print(":");
      Serial.println(currentSecond);
    }
    lastSecond = currentSecond;
  }

  // Every minute: Update LED matrix
  if (currentMinute != lastMinute) {
    lastMinute = currentMinute;

    if (currentMinute % updateInterval == 0) {
      displayMinute();
    }
  }
}

// ------------------ DISPLAY FUNCTIONS ------------------

void displayMinute() {
  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0xFFFFFFFF); // white
  matrix.textFont(Font_5x7);

  String displayValue = (countDirection == 0)
                          ? String(currentMinute)
                          : String(60 - currentMinute);

  // Center text
  int xPos = (displayValue.length() > 1)
               ? (matrix.width() - (matrix.textFontWidth() * 2)) / 2
               : (matrix.width() - matrix.textFontWidth()) / 2;
  int yPos = (matrix.height() - matrix.textFontHeight()) / 2;

  matrix.text(displayValue, xPos, yPos);
  matrix.endDraw();

  // Serial output
  Serial.println("---------------------");
  Serial.println("MINUTE UPDATE");
  Serial.print("Current Minute: ");
  Serial.println(currentMinute);
  Serial.print(countDirection == 0 ? "Counting UP: " : "Counting DOWN: ");
  Serial.println(displayValue);
  Serial.println("---------------------");
}
