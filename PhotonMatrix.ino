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

  Particle.connect();

  pinMode(Paddle1Pin, INPUT_PULLDOWN);
  pinMode(Paddle2Pin, INPUT_PULLDOWN);

  Particle.function("variable", setVariable);		// Receive mode commands

  Particle.subscribe("hook-response/weather_hook", gotWeatherData, MY_DEVICES);

  // read settings stored in EEPROM
  int eeAddress = 0;

  // read modeIndex
  setModeIndex(EEPROM.get(eeAddress, modeIndex));
  eeAddressModeIndex = eeAddress;
  eeAddress += sizeof(modeIndex);

  // read timezone
  setTimezone(EEPROM.get(eeAddress, timezone));
  eeAddressTimezone = eeAddress;
  eeAddress += sizeof(timezone);

  // read zip
  setZip(EEPROM.get(eeAddress, zip));
  eeAddressZip = eeAddress;
  eeAddress += sizeof(zip);

  // read ampm (show AM/PM or 24 hour clock)
  setAmPm(EEPROM.get(eeAddress, ampm));
  eeAddressAmpm = eeAddress;
  eeAddress += sizeof(ampm);
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
      if (Particle.connected()) {
        Particle.process();
      }

      delay(100);

      return;
    }
  }

  if (Particle.connected()) {
    Particle.process();

    static unsigned long lastTimeSync = 0;

    // sync time every 24 hrs
    if (millis() > lastTimeSync + MillisPerDay) {
      Particle.syncTime();
      lastTimeSync = millis();
    }

    // update weather every hour
    if (lastWeatherSync == 0 || millis() > lastWeatherSync + MillisPerHour || (!weatherReceived && millis() > lastWeatherSync + MillsPerMinute)) {
      Serial.println("Requesting Weather!");

      weatherReceived = false;

      // publish the event that will trigger our Webhook
      Particle.publish("weather_hook", String(zip));
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
  matrix.drawPixel(0, 0, matrix.Color888(externalLedR, externalLedG, externalLedB, true));

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
    return setPower(args.substring(4).toInt());
  }
  else if (args.startsWith("tz:")) {
    return setTimezone(args.substring(3).toInt());
  }
  else if (args.startsWith("ampm:")) {
    return setAmPm(args.substring(5).toInt());
  }
  else if (args.startsWith("zip:")) {
    return setZip(args.substring(4).toInt());
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

int setPower(int value) {
  power = value;
  if (power < 0)
    power = 0;
  else if (power > 1)
    power = 1;

  return power;
}

int setTimezone(int value) {
  timezone = value;
  if (timezone < -12)
    timezone = -12;
  else if (timezone > 13)
    timezone = 13;

  Time.zone(timezone);

  EEPROM.put(eeAddressTimezone, timezone);

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

  EEPROM.put(eeAddressModeIndex, modeIndex);

  return modeIndex;
}

int setAmPm(int value) {
  int ampmInt = value;
  if (ampmInt < 0)
    ampmInt = 0;
  else if (ampmInt > 1)
    ampmInt = 1;

  ampm = ampmInt != 0;

  EEPROM.put(eeAddressAmpm, ampm);

  return ampmInt;
}

int setZip(int value) {
  zip = value;
  if (zip < 0)
    zip = 0;
  else if (zip > 99999)
    zip = 99999;

  EEPROM.put(eeAddressZip, zip);

  lastWeatherSync = 0;

  return zip;
}
