/*
* Photon Matrix by Jason Coon @jasoncoon_ at Evil Genius Labs @evilgeniuslab
*
* Derived from Paul Kourany's RGBPongClock: https://github.com/pkourany/RGBPongClock
* Which was derived by RGB Pong Clock by Andrew Holmes @pongclock: https://github.com/twonk/RGBPongClock
* Which was inspired by, and shamelessly derived from Nick's LED Projects: https://123led.wordpress.com/about
*
*/

#include "PhotonMatrix.h"

void setup() {
  Serial.begin(115200);

  // Lets give ourselves 3 seconds before we actually start the program.
  // That will just give us a chance to open the serial monitor before the program sends the request
  /*for (int i = 0; i < 3; i++) {
  Serial.println("Waiting " + String(3 - i) + " seconds for serial monitor...");
  delay(1000);
  }*/

  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(210, 210, 210));

  matrix.fillScreen(0);
  matrix.swapBuffers(true);

  RGB.onChange(ledChangeHandler);

  Spark.connect();

  pinMode(Paddle1Pin, INPUT_PULLDOWN);
  pinMode(Paddle2Pin, INPUT_PULLDOWN);

  pinMode(Paddle1ButtonPin, INPUT_PULLUP);
  pinMode(Paddle2ButtonPin, INPUT_PULLUP);

  Spark.variable("paddle1Y", &paddle1PinState, INT);
  Spark.variable("paddle2Y", &paddle2PinState, INT);

  Spark.variable("paddle1Btn", &paddle1ButtonPinState, INT);
  Spark.variable("paddle2Btn", &paddle2ButtonPinState, INT);

  Spark.function("variable", setVariable);		// Receive mode commands

  Spark.subscribe("hook-response/weather_hook", gotWeatherData, MY_DEVICES);

  Time.zone(timezone);

  // read settings stored in EEPROM

  // read timezone sign (positive or negative)
  uint8_t timezoneSign = EEPROM.read(1);

  // read timezone
  if (timezoneSign < 1)
    timezone = -EEPROM.read(2);
  else
    timezone = EEPROM.read(2);

  if (timezone < -12)
    timezone = -12;
  else if (timezone > 14)
    timezone = 14;

  // read modeIndex
  modeIndex = EEPROM.read(3);
}

void loop() {
  matrix.fillScreen(0);

  if (power == 0) {
    matrix.swapBuffers(false);
    delay(100);
    return;
  }

  if (Spark.connected()) {
    Spark.process();

    // sync time every 24 hrs
    if (millis() > lastTimeSync + MillisPerDay) {
      Spark.syncTime();
      lastTimeSync = millis();
    }

    // update weather every hour
    if (lastWeatherSync == 0 || millis() > lastWeatherSync + MillisPerHour) { // || !weatherReceived && millis() > lastWeatherSync + MillsPerMinute) {
      Serial.println("Requesting Weather!");

      weatherReceived = false;

      // publish the event that will trigger our Webhook
      Spark.publish("weather_hook");
      lastWeatherSync = millis();
    }
  }

  // show the clock on the bottom of the screen
  switch (modeIndex) {
  case 0:
    // show the weather on the top of the screen
    drawWeather();

    // draw the pong clock on the bottom of the screen
    drawPongClock(16);

    break;

  case 1:
    // show the weather on the top of the screen
    drawWeather();

    // draw the date and time on the bottom of the screen
    drawDateAndTime(1, 17);

    break;

  case 2:
    runPongGame();

    break;
  }

  matrix.drawPixel(0, 0, matrix.Color888(externalLedR, externalLedG, externalLedB));

  matrix.swapBuffers(true);

  delay(40);
}

void ledChangeHandler(uint8_t r, uint8_t g, uint8_t b) {
  externalLedR = r;
  externalLedG = g;
  externalLedB = b;
}

int setVariable(String args) {
  Serial.print("setVariable args: ");
  Serial.println(args);

  if (args.startsWith("pwr:")) {
    return setPower(args.substring(4));
  }
  else if (args.startsWith("tz:")) {
    return setTimezone(args.substring(3));
  }
  else if (args.startsWith("ampm:")) {
    return setAmPm(args.substring(5));
  }
  else if (args.startsWith("mode:")) {
    return setModeIndex(args.substring(5));
  }

  return -1;
}

int setPower(String args) {
  power = args.toInt();
  if (power < 0)
    power = 0;
  else if (power > 1)
    power = 1;

  return power;
}

int setTimezone(String args) {
  timezone = args.toInt();
  if (timezone < -12)
    timezone = -12;
  else if (timezone > 13)
    timezone = 13;

  Time.zone(timezone);

  if (timezone < 0)
    EEPROM.write(1, 0);
  else
    EEPROM.write(1, 1);

  EEPROM.write(2, abs(timezone));

  return timezone;
}

int setModeIndex(String args) {
  modeIndex = args.toInt();

  if (modeIndex < 0)
    modeIndex = 0;
  else if (modeIndex > 2)
    modeIndex = 2;

  EEPROM.write(3, modeIndex);

  return modeIndex;
}

int setAmPm(String args) {
  int ampmInt = args.toInt();
  if (ampmInt < 0)
    ampmInt = 0;
  else if (ampmInt > 1)
    ampmInt = 1;

  ampm = ampmInt != 0;

  return ampmInt;
}
