/*
* Photon Matrix by Jason Coon @jasoncoon_ at Evil Genius Labs @evilgeniuslab
*
* Derived from Paul Kourany's RGBPongClock: https://github.com/pkourany/RGBPongClock
* Which was derived by RGB Pong Clock by Andrew Holmes @pongclock: https://github.com/twonk/RGBPongClock
* Which was inspired by, and shamelessly derived from Nick's LED Projects: https://123led.wordpress.com/about
*
*/

#include "PhotonMatrix.h"

#include "PongGame.h"
#include "BreakoutGame.h"

PongGame pongGame;
BreakoutGame breakoutGame;

typedef uint8_t (*ModeList[])();

ModeList modes = {
  weatherAndPongClock,
  weatherAndDateTime,
  gmailAndPongClock,
  gmailAndDateTime,
  runPongGame,
  runBreakoutGame,
};

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

const uint8_t modeCount = ARRAY_SIZE(modes);

void setup() {
  Serial.begin(115200);

  // Lets give ourselves 3 seconds before we actually start the program.
  // That will just give us a chance to open the serial monitor before the program sends the request
  for (int i = 0; i < 2; i++) {
    Serial.println("Waiting " + String(3 - i) + " seconds for serial monitor...");
    delay(1000);
  }

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

  Particle.subscribe("hook-response/gmail_hook", gotGmailData, MY_DEVICES);

  readSettings();
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
    if (lastWeatherSync == 0 || millis() > lastWeatherSync + MillisPerHour || (!weatherReceived && millis() > lastWeatherSync + MillisPerMinute)) {
      Serial.println("Requesting Weather!");

      weatherReceived = false;

      // publish the event that will trigger our Webhook
      Particle.publish("weather_hook", String(zip));
      lastWeatherSync = millis();
    }

    // update gmail every 5 minutes
    if (lastGmailSync == 0 || millis() > lastGmailSync + MillisPerMinute || (!gmailReceived && millis() > lastGmailSync + MillisPerMinute)) {
      Serial.println("Requesting Gmail!");

      Serial.print("gmailUserAndPassword: ");
      Serial.println(gmailUserAndPassword);

      gmailReceived = false;

      // publish the event that will trigger our Webhook
      Particle.publish("gmail_hook", gmailUserAndPassword);
      lastGmailSync = millis();
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

uint8_t gmailAndPongClock()
{
  // show the gmail on the top of the screen
  drawGmail(1, 1);

  // draw the pong clock on the bottom of the screen
  drawPongClock(16);

  return 40;
}

uint8_t gmailAndDateTime()
{
  // show the gmail on the top of the screen
  drawGmail(1, 1);

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

  int result = -42;

  if (args.startsWith("pwr:")) {
    result = setPower(args.substring(4).toInt());
  }
  else if (args.startsWith("tz:")) {
    result = setTimezone(args.substring(3).toInt());
  }
  else if (args.startsWith("ampm:")) {
    result = setAmPm(args.substring(5).toInt());
  }
  else if (args.startsWith("zip:")) {
    result = setZip(args.substring(4).toInt());
  }
  else if (args.startsWith("gmail:")) {
    result = setGmailUserAndPassword(args.substring(6));
  }
  else if (args.startsWith("mode:")) {
    result = setModeIndex(args.substring(5).toInt());
  }
  else if (args.startsWith("nextMode")) {
    result = setModeIndex(modeIndex++);
  }
  else if (args.startsWith("prevMode")) {
    result = setModeIndex(modeIndex--);
  }

  if(result != -42)
    writeSettings();

  return result;
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

  return modeIndex;
}

int setAmPm(int value) {
  ampm = value;
  if (ampm < 0)
    ampm = 0;
  else if (ampm > 1)
    ampm = 1;

  return ampm;
}

int setZip(int value) {
  zip = value;
  if (zip < 0)
    zip = 0;
  else if (zip > 99999)
    zip = 99999;

  lastWeatherSync = 0;

  return zip;
}

int setGmailUserAndPassword(String value) {
  Serial.print("gmailUserAndPassword: ");
  Serial.println(value);

  gmailUserAndPassword = value;

  lastGmailSync = 0;

  return 0;
}
