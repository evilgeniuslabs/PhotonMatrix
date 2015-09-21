#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 32

#include "math.h"

#include "Adafruit_GFX.h"   // Core graphics library
#include "RGBmatrixPanel.h" // Hardware-specific library

#include "font3x5.h"
#include "font5x5.h"

// Arduino equivalence
#define pgm_read_byte_near(_addr) (pgm_read_byte(_addr))
#define pgm_read_byte_far(_addr)	(pgm_read_byte(_addr))
#define pgm_read_word(_addr) (*(const uint16_t *)(_addr))
#define pgm_read_word_near(_addr) (pgm_read_word(_addr))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

// allow us to use itoa() in this scope
extern char* itoa(int a, char* buffer, unsigned char radix);

/** Define RGB matrix panel GPIO pins **/
#if defined (STM32F10X_MD)	//Core
#define CLK D6
#define OE  D7
#define LAT A4
#define A   A0
#define B   A1
#define C   A2
#define D   A3
#endif

#if defined (STM32F2XX)	//Photon
#define CLK D6
#define OE  D7
#define LAT A4
#define A   A0
#define B   A1
#define C   A2
#define D   A3
#endif

#define MillsPerMinute 60UL * 1000UL
#define MillisPerHour 60UL * MillsPerMinute
#define MillisPerDay 24UL * MillisPerHour

const float PI = 3.1415926535;

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, true);

SYSTEM_MODE(MANUAL);

STARTUP(WiFi.selectAntenna(ANT_AUTO); ); // continually switches at high speed between
// STARTUP( WiFi.selectAntenna(ANT_INTERNAL); ); // selects the CHIP antenna
// STARTUP( WiFi.selectAntenna(ANT_EXTERNAL); ); // selects the u.FL antenna

uint8_t externalLedR = 0;
uint8_t externalLedG = 0;
uint8_t externalLedB = 0;

char DayNames[7][4] = {
  "Sun", "Mon","Tue", "Wed", "Thu", "Fri", "Sat"
};

#include "Graphics.h"
#include "GraphicsFonts.h"

int power = 1;

int timezone = -5;
int eeAddressTimezone;

int modeIndex = 0;
int eeAddressModeIndex;

int zip = 66207;
int eeAddressZip;

boolean ampm = true;
int eeAddressAmpm;

#define Paddle1Pin A6
#define Paddle2Pin A5

int paddle1PinState = 0;
int paddle2PinState = 0;

#define Paddle1ButtonPin RX
#define Paddle2ButtonPin TX

#include "Rectangle.h"
#include "Vector.h"
#include "Movable.h"
#include "ClickButton.h"

Movable paddle1(1, 15);
Movable paddle2(29, 15);

ClickButton paddle1Button(RX, LOW, CLICKBTN_PULLUP);
ClickButton paddle2Button(TX, LOW, CLICKBTN_PULLUP);

#include "Drawable.h"
#include "Clock.h"
#include "PongClock.h"
#include "WeatherIcons.h"
#include "Weather.h"
