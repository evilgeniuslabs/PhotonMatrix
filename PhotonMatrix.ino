/*
* Photon Matrix by Jason Coon @jasoncoon_ at Evil Genius Labs @evilgeniuslab
*
* Derived from Paul Kourany's RGBPongClock: https://github.com/pkourany/RGBPongClock
* Which was derived by RGB Pong Clock by Andrew Holmes @pongclock: https://github.com/twonk/RGBPongClock
* Which was inspired by, and shamelessly derived from Nick's LED Projects: https://123led.wordpress.com/about
*
*/

#include "PhotonMatrix.h"

typedef uint8_t (*ModeList[])();

ModeList modes = {
  weatherAndPongClock,
  weatherAndDateTime,
  runPongGame,
  runBreakoutGame,
};

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

const uint8_t modeCount = ARRAY_SIZE(modes);

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

  readPaddleInput();

  if (power == 0) {
    matrix.swapBuffers(false);

    // wake if a paddle button is pressed
    if(paddle1Button.clicks != 0 || paddle2Button.clicks != 0)
    {
      power = 1;
      paddle1Button.clicks = 0;
      paddle2Button.clicks = 0;
    }
    else
    {
      // let Particle process, then delay a bit and stay asleep
      if (Spark.connected()) {
        Spark.process();
      }

      delay(100);

      return;
    }
  }

  if (Spark.connected()) {
    Spark.process();

    static unsigned long lastTimeSync = 0;

    // sync time every 24 hrs
    if (millis() > lastTimeSync + MillisPerDay) {
      Spark.syncTime();
      lastTimeSync = millis();
    }

    // update weather every hour
    if (lastWeatherSync == 0 || millis() > lastWeatherSync + MillisPerHour || (!weatherReceived && millis() > lastWeatherSync + MillsPerMinute)) {
      Serial.println("Requesting Weather!");

      weatherReceived = false;

      // publish the event that will trigger our Webhook
      Spark.publish("weather_hook");
      lastWeatherSync = millis();
    }
  }

  // move to the next mode if a paddle button is held
  if(paddle1Button.clicks < 0 || paddle2Button.clicks < 0)
  {
    setModeIndex(modeIndex + 1);
  }

  // run the current mode
  uint8_t requestedDelay = modes[modeIndex]();

  // draw the status LED state at in the top left corner
  matrix.drawPixel(0, 0, matrix.Color888(externalLedR, externalLedG, externalLedB));

  // show the current frame
  matrix.swapBuffers(true);

  // "sleep" for the amount of time requested by the current mode/pattern
  delay(requestedDelay);
}

uint8_t weatherAndPongClock()
{
  // show the weather on the top of the screen
  drawWeather();

  // draw the pong clock on the bottom of the screen
  drawPongClock(16);

  return 40;
}

uint8_t weatherAndDateTime()
{
  // show the weather on the top of the screen
  drawWeather();

  // draw the date and time on the bottom of the screen
  drawDateAndTime(1, 17);

  return 40;
}

uint8_t runPongGame()
{
  return pongGame.drawFrame();
}

uint8_t runBreakoutGame()
{
  return breakoutGame.drawFrame();
}

void readPaddleInput()
{
  paddle1Button.Update();
  paddle2Button.Update();

  // read paddle rotary analog pins
  paddle1PinState = analogRead(Paddle1Pin);
  paddle2PinState = analogRead(Paddle2Pin);
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
    return setModeIndex(args.substring(5).toInt());
  }
  else if (args.startsWith("nextMode")) {
    return setModeIndex(modeIndex++);
  }
  else if (args.startsWith("prevMode")) {
    return setModeIndex(modeIndex--);
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

int setModeIndex(int value) {
  pongGame.isPaused = true;
  breakoutGame.isPaused = true;

  modeIndex = value;

  if (modeIndex < 0)
    modeIndex = modeCount - 1;
  else if (modeIndex >= modeCount)
    modeIndex = 0;

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
