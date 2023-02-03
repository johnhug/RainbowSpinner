// ATMEL ATTINY85
//
//             +-\/-+
//            1|    |8  Vcc
// ANALOG_IN  2|    |7  
//            3|    |6  LED_OUT
//       GND  4|    |5  
//             +----+
//
// ANALOG_IN - 10k POT
//
//      S - PIN 2
//    +---+
//    | + |  
//    +---+
//    A   E - VCC
//    - GND

#include <Adafruit_NeoPixel.h>  //needed for the WS2812
#include <avr/sleep.h>
#include <avr/eeprom.h>

#define LED_OUT 1
#define ANALOG_IN A3
#define BRIGHTNESS 255
#define LEDS 16

Adafruit_NeoPixel ring = Adafruit_NeoPixel(LEDS, LED_OUT, NEO_GRB + NEO_KHZ800);

uint32_t reseedRandomSeed EEMEM = 0xFFFFFFFF;

// https://en.wikipedia.org/wiki/HSL_and_HSV
float hsv_f(float n, uint16_t h, float s, float v) {
  float k = fmod(n + h / 60.0f, 6);
  return v - v * s * fmax(fmin(fmin(k, 4 - k), 1), 0);
}

uint32_t hueColor(uint16_t h, float s = 1) {
  float v = 1;
  uint8_t r = 255 * hsv_f(5, h, s, v);
  uint8_t g = 255 * hsv_f(3, h, s, v);
  uint8_t b = 255 * hsv_f(1, h, s, v);
  return ring.Color(r, g, b);
}

void rainbowSpin(long wait) {
    for (uint8_t procession = 0; procession < LEDS; procession++) {
      for (uint8_t led = 0; led < LEDS; led++) {
        uint32_t color = hueColor(((led + procession) * 360 / LEDS) % 360);
        ring.setPixelColor(led, color);
      }
      ring.show();
      delay(wait);
    }
}

void setup() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  ring.begin();
  ring.setBrightness(BRIGHTNESS);
  reseedRandom(&reseedRandomSeed);
}

void loop() {
  int analogInput = analogRead(ANALOG_IN);

  uint8_t decay = map(analogInput, 0, 1023, 2, 10);

  for (uint8_t spin = 0; spin < LEDS; spin++) {
    rainbowSpin(spin * decay);
  }

  rainbowSpin(20 * decay + 25);

  // uint32_t color = hueColor(map(analogInput, 0, 1023, 0, 359));
  uint32_t color = hueColor(random(360));
  ring.fill(color, 0, LEDS);
  ring.show();

  // delay(1000);
  sleep_mode();
}

/*==============================================================================
  Call reseedRandom once in setup to start random on a new sequence.  Uses 
  four bytes of EEPROM.
==============================================================================*/

void reseedRandom(uint32_t* address) {
  static const uint32_t HappyPrime = 127807 /*937*/;
  uint32_t raw;
  unsigned long seed;

  // Read the previous raw value from EEPROM
  raw = eeprom_read_dword(address);

  // Loop until a seed within the valid range is found
  do {
    // Incrementing by a prime (except 2) every possible raw value is visited
    raw += HappyPrime;

    // Park-Miller is only 31 bits so ignore the most significant bit
    seed = raw & 0x7FFFFFFF;
  } while ((seed < 1) || (seed > 2147483646));

  // Seed the random number generator with the next value in the sequence
  srandom(seed);

  // Save the new raw value for next time
  eeprom_write_dword(address, raw);
}

inline void reseedRandom(unsigned short address) {
  reseedRandom((uint32_t*)(address));
}


/*==============================================================================
  So the reseedRandom raw value can be initialized allowing different 
  applications or instances to have different random sequences.

  Generate initial raw values...

  https://www.random.org/cgi-bin/randbyte?nbytes=4&format=h
  https://www.fourmilab.ch/cgi-bin/Hotbits?nbytes=4&fmt=c&npass=1&lpass=8&pwtype=3

==============================================================================*/

void reseedRandomInit(uint32_t* address, uint32_t value) {
  eeprom_write_dword(address, value);
}

inline void reseedRandomInit(unsigned short address, uint32_t value) {
  reseedRandomInit((uint32_t*)(address), value);
}
