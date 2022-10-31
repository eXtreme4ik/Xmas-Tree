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
#define LED_1STRIPE 40 // Количество светодиодов в одной полоске (4 полоски последовательно) (TEST)
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

#define MODE_PROGRAMM 0
#define MODE_BRIGHTLESS 1
#define MODE_SPEED 2
#define MAXMODES 3

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
#define SPEED_MAX 10
#define BRIGHT_STEP BRIGHT_MAX / 10
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
void roundPixel(uint32_t color, int wait, Adafruit_NeoPixel &strip);
void statefunction(Adafruit_NeoPixel &strip);
int firstPixelHue = 0;
uint16_t roundcount = 0;
int timeButton = 0;
int mode = 0;
int bright = 50;
int prog = 0;
int speed = 1;
bool up = false;
bool dn = false;
bool md = false;
bool flag = false;
int pixel = 0;
int r, g, b = 0;
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
      flag = true;
    }
    if (value == BUTTON_UP)
    {
      up = true;
      flag = true;
      timeButton = 100;
    }
    if (value == BUTTON_DOWN)
    {
      dn = true;
      flag = true;
      timeButton = 100;
    }
    if (value == BUTTON_MODE)
    {
      md = true;
      flag = true;
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
  }
  if (mode == MODE_SPEED)
  {
    if (up)
    {
      speed -= 1;
      up = false;
    }
    if (dn)
    {
      speed += 1;
      dn = false;
    }
    if (speed < 1)
      speed = 1;
    if (speed > SPEED_MAX)
      speed = SPEED_MAX;
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
  statefunction(stripe);
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
// Some functions of our own for creating animated effects -----------------
void roundPixel(uint32_t color, int wait, Adafruit_NeoPixel &strip)
{
  if (roundcount > strip.numPixels())
    roundcount = 0;
  strip.setPixelColor(roundcount, color);
  strip.setPixelColor(roundcount - 1, strip.Color(0, 0, 0));
  strip.show();
  delay(wait);
  roundcount++;
}

void statefunction(Adafruit_NeoPixel &strip)
{
  switch (mode)
  {

  case MODE_PROGRAMM:
  {
    uint32_t colRand = strip.gamma32(strip.ColorHSV((uint16_t)random()));
    uint8_t num = random();
    switch (prog)
    {

    case 0: // RANDOM CLEAR
    {
      while (num > strip.numPixels())
      {
        num = random();
      }
      strip.setPixelColor(num, colRand);
      strip.show();
      delay(50*speed);
      strip.clear();
    }
    break;

    case 1: // RANDOM NON CLEAR
    {
      while (num > strip.numPixels())
      {
        num = random();
      }
      strip.setPixelColor(num, colRand);
      strip.show();
      delay(50*speed);
    }
    break;

    case 2: // RAINBOW
    {
      for (int b = 0; b < 3; b++)
      {
        for (uint16_t c = b; c < strip.numPixels(); c += 3)
        {
          uint16_t hue = firstPixelHue + c * 65536L / strip.numPixels();
          uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
          strip.setPixelColor(c, color);                       // Set pixel 'c' to value 'color'
        }
        strip.show();                // Update strip with new contents
        firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
      }
      delay(10*speed);
    }
    break;

    case 3: // ROUNDPIXEL
    {
      roundPixel(colRand, 10*speed, strip);
    }
    break;
    case 4: // SMALL SNOW
    {
      for (int i = 0; i < 5; i++)
      {
        uint8_t snows = random(100);
        num = random();
        while (num > strip.numPixels())
        {
          num = random();
        }
        strip.setPixelColor(num, strip.Color(snows, snows, snows));
      }
      strip.show();
      delay(100*speed);
      strip.clear();
    }
    break;
    case 5: // SNOW STORM
    {
      for (int b = 0; b < 3; b++)
      {
        for (uint16_t c = b; c < strip.numPixels(); c += 3)
        {
          uint8_t hue = random(100);
          uint32_t color = strip.Color(hue,hue,hue); // hue -> RGB
          strip.setPixelColor(c, color);                       // Set pixel 'c' to value 'color'
        }
        strip.show();                // Update strip with new contents
        firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
      }
      delay(30*speed);
    }
    break;
    default:
      prog = 0;
      break;
    }
  }
  break;

  case MODE_BRIGHTLESS:
  {
    stripe.clear();
    stripe.setPixelColor(0, stripe.Color(255, 255, 255));
    stripe.show();
  }
  break;

  case MODE_SPEED:
  {
    stripe.clear();
    delay(100*speed);
    stripe.setPixelColor(0, stripe.Color(255, 255, 255));
    stripe.show();
    delay(100*speed);
    stripe.setPixelColor(0, stripe.Color(0, 0, 0));
    stripe.show();
  }
  break;
  }
}