/*
 * Photon Matrix by Jason Coon @jasoncoon_ at Evil Genius Labs @evilgeniuslab
 *
 * Derived from Paul Kourany's RGBPongClock: https://github.com/pkourany/RGBPongClock
 * Which was derived by RGB Pong Clock by Andrew Holmes @pongclock: https://github.com/twonk/RGBPongClock
 * Which was inspired by, and shamelessly derived from Nick's LED Projects: https://123led.wordpress.com/about
 *
 */

#include "PhotonMatrix.h"

#define HOOK_RESP

void setup() {
  Serial.begin(115200);

	Spark.function("variable", setVariable);		// Receive mode commands

	Spark.subscribe("hook-response/weather_hook", gotWeatherData, MY_DEVICES);

  Time.zone(timezone);

  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(210, 210, 210));

  /*// Lets give ourselves 10 seconds before we actually start the program.
  // That will just give us a chance to open the serial monitor before the program sends the request
  for(int i=0;i<3;i++) {
    Serial.println("Waiting " + String(3-i) + " seconds for serial monitor...");
    delay(1000);
  }*/
}

void loop() {
	matrix.fillScreen(0);

  if(power == 0) {
    matrix.swapBuffers(false);
    delay(100);
    return;
  }

  // sync time every 24 hrs
  if (millis() > lastTimeSync + MillisPerDay) {
    Spark.syncTime();
    lastTimeSync = millis();
  }

	// update weather every hour
	if(lastWeatherSync == 0 || millis() > lastWeatherSync + MillisPerHour) {
    Serial.println("Requesting Weather!");

		weatherReceived = false;

		// publish the event that will trigger our Webhook
    /*Spark.publish("get_weather");*/
		Spark.publish("weather_hook");
		lastWeatherSync = millis();
	}

  // show the clock on the bottom of the screen
  switch(clockIndex) {
    case 0:
      drawPongClock(16);
      break;

    case 1:
      drawDateAndTime(1, 17);
      break;
  }

  // show the weather on the top of the screen
	drawWeather();

  delay(40);

  matrix.swapBuffers(false);
}

int setVariable(String args) {
    if(args.startsWith("pwr:")) {
        return setPower(args.substring(4));
    }
    else if (args.startsWith("tz:")) {
        return setTimezone(args.substring(3));
    }
    else if (args.startsWith("ampm:")) {
        return setAmPm(args.substring(5));
    }
    else if (args.startsWith("clk:")) {
        return setClockIndex(args.substring(4));
    }
    /*
    else if (args.startsWith("brt:")) {
        return setBrightness(args.substring(4));
    }
    else if (args.startsWith("flpclk:")) {
        return setFlipClock(args.substring(7));
    }
    else if (args.startsWith("r:")) {
        r = parseByte(args.substring(2));
        solidColor.r = r;
        EEPROM.write(5, r);
        patternIndex = patternCount - 1;
        return r;
    }
    else if (args.startsWith("g:")) {
        g = parseByte(args.substring(2));
        solidColor.g = g;
        EEPROM.write(6, g);
        patternIndex = patternCount - 1;
        return g;
    }
    else if (args.startsWith("b:")) {
        b = parseByte(args.substring(2));
        solidColor.b = b;
        EEPROM.write(7, b);
        patternIndex = patternCount - 1;
        return b;
    }
    else if (args.startsWith("nsx:")) {
        noiseSpeedX = args.substring(4).toInt();
        if(noiseSpeedX < 0)
            noiseSpeedX = 0;
        else if (noiseSpeedX > 65535)
            noiseSpeedX = 65535;
        return noiseSpeedX;
    }
    else if (args.startsWith("nsy:")) {
        noiseSpeedY = args.substring(4).toInt();
        if(noiseSpeedY < 0)
            noiseSpeedY = 0;
        else if (noiseSpeedY > 65535)
            noiseSpeedY = 65535;
        return noiseSpeedY;
    }
    else if (args.startsWith("nsz:")) {
        noiseSpeedZ = args.substring(4).toInt();
        if(noiseSpeedZ < 0)
            noiseSpeedZ = 0;
        else if (noiseSpeedZ > 65535)
            noiseSpeedZ = 65535;
        return noiseSpeedZ;
    }
    else if (args.startsWith("nsc:")) {
        noiseScale = args.substring(4).toInt();
        if(noiseScale < 0)
            noiseScale = 0;
        else if (noiseScale > 65535)
            noiseScale = 65535;
        return noiseScale;
    }
    */

    return -1;
}

int setPower(String args) {
    power = args.toInt();
    if(power < 0)
        power = 0;
    else if (power > 1)
        power = 1;

    return power;
}

int setTimezone(String args) {
    timezone = args.toInt();
    if(timezone < -12)
      timezone = -12;
    else if (timezone > 13)
      timezone = 13;

    Time.zone(timezone);

    if(timezone < 0)
        EEPROM.write(1, 0);
    else
        EEPROM.write(1, 1);

    EEPROM.write(2, abs(timezone));

    return timezone;
}

int setClockIndex(String args) {
    clockIndex = args.toInt();

    if(clockIndex < 0)
      clockIndex = 0;
    else if (clockIndex > 1)
      clockIndex = 1;

    return clockIndex;
}

int setAmPm(String args) {
    int ampmInt = args.toInt();
    if(ampmInt < 0)
      ampmInt = 0;
    else if (ampmInt > 1)
      ampmInt = 1;

    ampm = ampmInt != 0;

    return ampmInt;
}
