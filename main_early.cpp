#include <Arduino.h>
// Copy from here-----------------------------------------------------\/
//----,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,----------------------------------\/
//---|                              |---------------------------------\/
//---|  "Xmas tree" Arduino sketch  |---------------------------------\/
//---|                              |---------------------------------\/
//---````````````````````````````````---------------------------------\/

//--------------------------INCLUDES----------------------------------\/

#include <Adafruit_NeoPixel.h> //

//--------------------------USER PRESETS------------------------------\/

#define ONE_STRIPE
// #define FOUR_STRIPES

//--------------------------HARDWARE PRESETS--------------------------\/

#define LED_TOTAL 82   // Общее количество светодиодов в Ёлке
#define LED_STAR 10    // Количество светодиодов звезды (с обоих сторон последовательно)
#define LED_4STRIPE 18 // (4+4+3+3+2+2) Количество светодиодов в одной из 4-х полосок
#define LED_1STRIPE 40  // Количество светодиодов в одной полоске (4 полоски последовательно) (TEST)
//#define LED_1STRIPE 72  // Количество светодиодов в одной полоске (4 полоски последовательно)
#ifdef ONE_STRIPE
#define STRIPE_PIN 5 // Пин ленты
#endif
#ifdef FOUR_STRIPES
#define STRIPE0_PIN 5 // Пин ленты
#define STRIPE1_PIN 6 // Пин второй ленты
#define STRIPE2_PIN 7 // Пин третьей ленты
#define STRIPE3_PIN 8 // Пин четвертой ленты
#endif
#define STAR_PIN 9         // Пин ленты звезды
#define BUTTONS_ADC_PIN A0 //
#define SW1 1              // Кнопки
#define SW2 2
#define SW3 3
#define SW4 4

#define LED_RUN
#define LED_PROG
#define LED_BRIGHT
#define LED_SPEED

//-----------------------------MACRO FOR ADC CALCULATE----------------\/

#define calcButton(x)            \
  do                             \
  {                              \
    if (380 < x && x < 420)      \
      x = 1;                     \
    else if (570 < x && x < 625) \
      x = 2;                     \
    else if (760 < x && x < 830) \
      x = 3;                     \
    else if (680 < x && x < 740) \
      x = 4;                     \
    else                         \
      x = 0;                     \
  } while (0)

//-----------------------------PRESETS FOR MACRO---------------------\/

// 1 is between 1.6-1.8V  ~SW1
// 2 is between 2.4-2.6V  ~SW2
// 3 is between 3.2-3.4V  ~SW3

#define BUTTON_UP SW1
#define BUTTON_DOWN SW3
#define BUTTON_MODE SW2
#define BUTTON_HIDDEN SW4

#define MODE_BRIGHTLESS 1
#define MODE_PROGRAMM 0
#define MAXMODES 2

//----------------------------EXAMPLE FOR MACRO----------------------\/
//
//    int value = analogRead(BUTTONS_ADC_PIN);
//    calcButton(value);
//    if (value == BUTTON_UP) {}
//    if (value == BUTTON_DOWN) {}
//    if (value == BUTTON_MODE) {}
//
//----------------------------END MACRO------------------------------\/

//----------------------------ADAFRUIT PRESETS-----------------------\/
#define TYPELED NEO_GRB
#define LEDSPEED NEO_KHZ800
// NEO_GRB (пиксели привязаны к битовому потоку GRB - лента WS2812)
// NEO_RGB (пиксели привязаны к битовому потоку RGB - лента WS2811)
// NEO_RGBW (для RGBW адресных лент)
// NEO_KHZ800 (800 кГц - указывается для светодиодов WS2812)
// NEO_KHZ400 (400 кГц - указывается для светодиодов WS2811)

#ifdef ONE_STRIPE
Adafruit_NeoPixel stripe(LED_1STRIPE, STRIPE_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel star(LED_STAR, STAR_PIN, TYPELED + LEDSPEED);
#define BRIGHT_MAX 100
#define BRIGHT_STEP BRIGHT_MAX / LED_1STRIPE
#endif
#ifdef FOUR_STRIPES
Adafruit_NeoPixel stripe1(LED_4STRIPE, STRIPE0_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe2(LED_4STRIPE, STRIPE1_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe3(LED_4STRIPE, STRIPE2_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe4(LED_4STRIPE, STRIPE3_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel star(LED_STAR, STAR_PIN, TYPELED + LEDSPEED);
#define BRIGHT_MAX 100
#define BRIGHT_STEP BRIGHT_MAX / LED_4STRIPE
#endif
void setupInterrupt(void);
void setupStripe(Adafruit_NeoPixel &strip, uint8_t brightness);
void allProgramms(Adafruit_NeoPixel &strip);
void randomPixel(int wait, int rounds, Adafruit_NeoPixel &strip, bool clear = 0);
void roundPixel(uint32_t color, int wait, int rounds, Adafruit_NeoPixel &strip);
void rainbow(int wait, Adafruit_NeoPixel &strip);
void theaterChase(uint32_t color, int wait, Adafruit_NeoPixel &strip);
void colorWipe(uint32_t color, int wait, Adafruit_NeoPixel &strip);
int timeButton = 0;
int mode = 0;
int bright = 50;
int prog = 0;
int speed = 50;
bool up = false;
bool dn = false;
bool md = false;
ISR(TIMER1_OVF_vect)
{
  int value = analogRead(BUTTONS_ADC_PIN);
  calcButton(value);
  if (timeButton)
    timeButton--;
  else
  {
    if (value == BUTTON_HIDDEN)
    {
      timeButton = 100;
    }
    if (value == BUTTON_UP)
    {
      up = true;
      timeButton = 100;
    }
    if (value == BUTTON_DOWN)
    {
      dn = true;
      timeButton = 100;
    }
    if (value == BUTTON_MODE)
    {
      md = true;
      timeButton = 100;
    }
  }
  if (md)
  {
    if (mode < MAXMODES - 1)
      mode++;
    else
      mode = 0;
    md = false;
  }
  if (mode == MODE_BRIGHTLESS)
  {
    if (up)
    {
      bright += BRIGHT_STEP;
      up = false;
    }
    if (dn)
    {
      bright -= BRIGHT_STEP;
      dn = false;
    }
    if (bright < 0)
      bright = 0;
    if (bright > BRIGHT_MAX)
      bright = BRIGHT_MAX;
  }
  if (mode == MODE_PROGRAMM)
  {
    if (up)
    {
      prog += 1;
      up = false;
    }
    if (dn)
    {
      prog -= 1;
      dn = false;
    }
    if (prog < 0)
      prog = 3;
    if (prog > 3)
      prog = 0;
  }
}
void setup()
{
  Serial.begin(9600);
  setupInterrupt();
#ifdef ONE_STRIPE
  setupStripe(stripe, 100);
  setupStripe(star, 50);
#endif
#ifdef FOUR_STRIPES
  setupStripe(stripe1, 50);
  setupStripe(stripe2, 50);
  setupStripe(stripe3, 50);
  setupStripe(stripe4, 50);
  setupStripe(star, 50);
#endif
}

void loop()
{
  stripe.setBrightness(bright);
  switch (mode)
  {
  case MODE_PROGRAMM:
    switch (prog)
    {
    case 0:
      randomPixel(50, 5, stripe, true);
      break;
    case 1:
      roundPixel(stripe.Color(255, 0, 0), 50, 1, stripe);
      roundPixel(stripe.Color(0, 255, 0), 50, 1, stripe);
      roundPixel(stripe.Color(0, 0, 255), 50, 1, stripe);
      break;
    case 2:
      randomPixel(50, 50, stripe, false);
      break;
    case 3:
      rainbow(10, stripe);
      break;
    }
    break;
  case MODE_BRIGHTLESS:
    int a = BRIGHT_STEP;
    for (uint16_t b = 0; b < stripe.numPixels(); b++)
    {
      if (a < bright)
      {
        stripe.setPixelColor(b, stripe.Color(100, 100, 100));
        a += BRIGHT_STEP;
      }
      if (a > bright)
      {
        stripe.setPixelColor(b, stripe.Color(0, 0, 0));
        a -= BRIGHT_STEP;
      }
    }

    stripe.show();
    break;
  }
}
void setupInterrupt(void)
{
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  // Timer1 overflow: enable
  TIMSK1 = (1 << TOIE1);
  // Установить таймер на тактовой частоте:
  TCCR1B |= (1 << CS10);
  sei();
}
void setupStripe(Adafruit_NeoPixel &strip, uint8_t brightness)
{
  strip.begin();
  strip.show();
  strip.setBrightness(brightness);
}
void allProgramms(Adafruit_NeoPixel &strip)
{
  rainbow(10, strip);
  strip.clear();
  delay(500);
  roundPixel(strip.Color(255, 0, 0), 100, 1, strip);
  roundPixel(strip.Color(0, 255, 0), 100, 1, strip);
  roundPixel(strip.Color(0, 0, 255), 100, 1, strip);
  roundPixel(strip.Color(255, 0, 255), 100, 1, strip);
  strip.clear();
  delay(500);
  randomPixel(100, 50, strip, true);
  strip.clear();
  delay(500);
}
// Some functions of our own for creating animated effects -----------------
void randomPixel(int wait, int rounds, Adafruit_NeoPixel &strip, bool clear)
{
  for (int i = 0; i < rounds; i++)
  {
    if (clear)
      strip.clear();
    char colR = random();
    char colG = random();
    char colB = random();
    uint8_t num = random();
    while (num > strip.numPixels())
    {
      num = random();
    }
    strip.setPixelColor(num, strip.Color(colR, colG, colB));
    strip.show();
    delay(wait);
  }
  strip.clear();
}
void roundPixel(uint32_t color, int wait, int rounds, Adafruit_NeoPixel &strip)
{
  for (int n = 0; n < rounds; n++)
  {
    for (uint16_t i = 0; i <= strip.numPixels(); i++)
    {
      strip.setPixelColor(i, color);
      strip.setPixelColor(i - 1, strip.Color(0, 0, 0));
      strip.show();
      delay(wait);
    }
  }
}
void colorWipe(uint32_t color, int wait, Adafruit_NeoPixel &strip)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {                                // For each pixel in strip...
    strip.setPixelColor(i, color); //  Set pixel's color (in RAM)
    strip.show();                  //  Update strip to match
    delay(wait);                   //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait, Adafruit_NeoPixel &strip)
{
  for (int a = 0; a < 10; a++)
  { // Repeat 10 times...
    for (int b = 0; b < 3; b++)
    {                //  'b' counts from 0 to 2...
      strip.clear(); //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (uint16_t c = b; c < strip.numPixels(); c += 3)
      {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait, Adafruit_NeoPixel &strip)
{
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
  {
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait, Adafruit_NeoPixel &strip)
{
  int firstPixelHue = 0; // First pixel starts at red (hue 0)
  for (int a = 0; a < 30; a++)
  { // Repeat 30 times...
    for (int b = 0; b < 3; b++)
    {                //  'b' counts from 0 to 2...
      strip.clear(); //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for (uint16_t c = b; c < strip.numPixels(); c += 3)
      {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        uint16_t hue = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color);                       // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}