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

#define LED_TOTAL 82 // Общее количество светодиодов в Ёлке
#define LED_STAR 8   // Количество светодиодов звезды (с обоих сторон последовательно) (TEST)
// #define LED_STAR 10    // Количество светодиодов звезды (с обоих сторон последовательно)
#define LED_4STRIPE 18 // (4+4+3+3+2+2) Количество светодиодов в одной из 4-х полосок
#define LED_1STRIPE 32 // Количество светодиодов в одной полоске (4 полоски последовательно) (TEST)
// #define LED_1STRIPE 72  // Количество светодиодов в одной полоске (4 полоски последовательно)
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

#define LED_BLUE 15
#define LED_GREEN 16
#define LED_RED 17
#define LED_YELLOW 18

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

#define MODE_DEFAULT 0
#define MODE_STRIPE 1
#define MODE_PROGRAMM 2
#define MODE_BRIGHTLESS 3
#define MODE_SPEED 4

#define MAXMODES 5

#define SUBMODE_STRIPE 1
#define SUBMODE_STAR 2
#define SUBMODE_ALL 0

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
#define BRIGHT_MAX 255
#define SPEED_MAX 50
#define SPEED_STEP 5
#define BRIGHT_STEP BRIGHT_MAX / 20
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
void statefunction(Adafruit_NeoPixel &strip, char &programma, int &firstPixelHue, uint16_t &roundcount, char sp);

// Переменные циклов (сохранены)
int firstPixelHueStripe = 0;
uint16_t roundcountStripe = 0;
int firstPixelHueStar = 0;
uint16_t roundcountStar = 0;

// Каунтер для нажатия кнопок
int timeButton = 0;

// Каунтер для статусных светодиодов
int strobe = 0;

// Каунтер для программ
int progCountStripe = 0;
int progCountStar = 0;

// Переменные Режимов
int mode = 0;
int submode = 0;

// Переменные программ для лент
char progStripe = 0;
char progStar = 0;
int brightStripe = 50;
char speedStripe = 5;
int brightStar = 50;
char speedStar = 5;

// Флаги
char firststart = 1;
bool save = false;
bool up = false;
bool dn = false;
bool md = false;
bool flag = false;
bool sync = false;

ISR(TIMER2_OVF_vect)
{
}
ISR(TIMER1_OVF_vect)
{
  int value = analogRead(BUTTONS_ADC_PIN);
  calcButton(value);
  if (timeButton)
    timeButton--;
  if (value && !timeButton)
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

    if (md)
    {
      if (mode < MAXMODES - 1)
        mode++;
      else
        mode = 0;
      md = false;
    }
    if (mode == MODE_DEFAULT)
    {
      if (value == BUTTON_HIDDEN)
        save = true;
      up = false;
      dn = false;
    }
    if (mode == MODE_STRIPE)
    {
      if (up)
      {
        submode += 1;
        up = false;
      }
      if (dn)
      {
        submode -= 1;
        dn = false;
      }
      if (submode > 2)
        submode = 0;
      if (submode < 0)
        submode = 2;
    }
    if (mode == MODE_PROGRAMM)
    {
      if (submode == SUBMODE_STRIPE)
      {
        if (up)
        {
          progStripe += 1;
          up = false;
        }
        if (dn)
        {
          progStripe -= 1;
          dn = false;
        }
      }
      if (submode == SUBMODE_STAR)
      {
        if (up)
        {
          progStar += 1;
          up = false;
        }
        if (dn)
        {
          progStar -= 1;
          dn = false;
        }
      }
      if (submode == SUBMODE_ALL)
      {
        if (up)
        {
          progStar += 1;
          progStripe = progStar;
          up = false;
        }
        if (dn)
        {
          progStar -= 1;
          progStripe = progStar;
          dn = false;
        }
      }
    }
    if (mode == MODE_BRIGHTLESS)
    {
      if (submode == SUBMODE_STRIPE)
      {
        if (up)
        {
          brightStripe += BRIGHT_STEP;
          up = false;
        }
        if (dn)
        {
          brightStripe -= BRIGHT_STEP;
          dn = false;
        }
        if (brightStripe < 1)
          brightStripe = 1;
        if (brightStripe > BRIGHT_MAX)
          brightStripe = BRIGHT_MAX;
      }
      if (submode == SUBMODE_STAR)
      {
        if (up)
        {
          brightStar += BRIGHT_STEP;
          up = false;
        }
        if (dn)
        {
          brightStar -= BRIGHT_STEP;
          dn = false;
        }
        if (brightStar < 1)
          brightStar = 1;
        if (brightStar > BRIGHT_MAX)
          brightStar = BRIGHT_MAX;
      }
      if (submode == SUBMODE_ALL)
      {
        if (up)
        {
          brightStripe += BRIGHT_STEP;
          brightStar = brightStripe;
          up = false;
        }
        if (dn)
        {
          brightStripe -= BRIGHT_STEP;
          brightStar = brightStripe;
          dn = false;
        }
        if (brightStripe < 1)
        {
          brightStripe = 1;
          brightStar = brightStripe;
        }
        if (brightStripe > BRIGHT_MAX)
        {
          brightStripe = BRIGHT_MAX;
          brightStar = brightStripe;
        }
      }
    }
    if (mode == MODE_SPEED)
    {
      if (submode == SUBMODE_STRIPE)
      {
        if (up)
        {
          speedStripe -= SPEED_STEP;
          up = false;
        }
        if (dn)
        {
          speedStripe += SPEED_STEP;
          dn = false;
        }
        if (speedStripe < 1)
          speedStripe = 1;
        if (speedStripe > SPEED_MAX)
          speedStripe = SPEED_MAX;
      }
      if (submode == SUBMODE_STAR)
      {
        if (up)
        {
          speedStar -= SPEED_STEP;
          up = false;
        }
        if (dn)
        {
          speedStar += SPEED_STEP;
          dn = false;
        }
        if (speedStar < 1)
          speedStar = 1;
        if (speedStar > SPEED_MAX)
          speedStar = SPEED_MAX;
      }
      if (submode == SUBMODE_ALL)
      {
        if (up)
        {
          speedStripe -= SPEED_STEP;
          speedStar = speedStripe;
          up = false;
        }
        if (dn)
        {
          speedStripe += SPEED_STEP;
          speedStar = speedStripe;
          dn = false;
        }
        if (speedStripe < 1)
        {
          speedStripe = 1;
          speedStar = speedStripe;
        }
        if (speedStripe > SPEED_MAX)
        {
          speedStripe = SPEED_MAX;
          speedStar = speedStripe;
        }
      }
    }
  }
  strobe++;
  switch (mode)
  {
  case MODE_DEFAULT:
    break;
  case MODE_STRIPE:
  {
    digitalWrite(LED_GREEN, 1);
    digitalWrite(LED_RED, 1);
    digitalWrite(LED_YELLOW, 1);
  }
  break;
  case MODE_PROGRAMM:
  {
    digitalWrite(LED_GREEN, 0);
    digitalWrite(LED_RED, 1);
    digitalWrite(LED_YELLOW, 1);
  }
  break;
  case MODE_BRIGHTLESS:
  {
    digitalWrite(LED_RED, 0);
    digitalWrite(LED_YELLOW, 1);
    digitalWrite(LED_GREEN, 1);
  }
  break;
  case MODE_SPEED:
  {
    digitalWrite(LED_YELLOW, 0);
    digitalWrite(LED_RED, 1);
    digitalWrite(LED_GREEN, 1);
  }
  break;
  }
  if (mode == MODE_DEFAULT)
  {
    digitalWrite(LED_BLUE, 1);
    digitalWrite(LED_GREEN, 1);
    digitalWrite(LED_RED, 1);
    digitalWrite(LED_YELLOW, 1);
  }
  else
  {
    if (submode == SUBMODE_ALL)
    {
      digitalWrite(LED_BLUE, 0);
    }
    if (submode == SUBMODE_STRIPE)
    {
      if (strobe > 100)
      {
        digitalWrite(LED_BLUE, 0);
        strobe = 0;
      }
      else
        digitalWrite(LED_BLUE, 1);
    }
    if (submode == SUBMODE_STAR)
    {
      if (strobe > 50)
      {
        digitalWrite(LED_BLUE, 0);
        strobe = 0;
      }
      else
        digitalWrite(LED_BLUE, 1);
    }
  }
}

void setup()
{
  firststart = eeprom_read_byte((uint8_t *)0x00);
  if (firststart != 13)
  {
    eeprom_write_byte((uint8_t *)0x00, 13);
    eeprom_write_byte((uint8_t *)0x01, progStripe);
    eeprom_write_byte((uint8_t *)0x02, progStar);
    eeprom_write_byte((uint8_t *)0x03, brightStripe);
    eeprom_write_byte((uint8_t *)0x04, brightStar);
    eeprom_write_byte((uint8_t *)0x05, speedStripe);
    eeprom_write_byte((uint8_t *)0x06, speedStar);
  }
  else
  {
    progStripe = eeprom_read_byte((uint8_t *)0x01);
    progStar = eeprom_read_byte((uint8_t *)0x02);
    brightStripe = eeprom_read_byte((uint8_t *)0x03);
    brightStar = eeprom_read_byte((uint8_t *)0x04);
    speedStripe = eeprom_read_byte((uint8_t *)0x05);
    speedStar = eeprom_read_byte((uint8_t *)0x06);
  }
  Serial.begin(9600);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
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
  Serial.println(speedStripe);
  stripe.setBrightness(brightStripe);
  star.setBrightness(brightStar);
  if (speedStripe - 1 <= progCountStripe)
  {
    statefunction(stripe, progStripe, firstPixelHueStripe, roundcountStripe, speedStripe);
    progCountStripe = 0;
  }
  if (speedStar - 1 <= progCountStar)
  {
    if (progStar == 3 && progStripe == 3)
    {
      if (roundcountStripe == stripe.numPixels()) sync = true;
      if (sync == true){
        statefunction(star, progStar, firstPixelHueStar, roundcountStar, speedStar/2);
      }
      if (roundcountStar == star.numPixels()){
        sync = false;
      }
    }
    else
    statefunction(star, progStar, firstPixelHueStar, roundcountStar, speedStar);
    progCountStar = 0;
  }
  progCountStar++;
  progCountStripe++;
  if (save == true)
  {

    eeprom_write_byte((uint8_t *)0x00, 13);
    eeprom_write_byte((uint8_t *)0x01, progStripe);
    eeprom_write_byte((uint8_t *)0x02, progStar);
    eeprom_write_byte((uint8_t *)0x03, brightStripe);
    eeprom_write_byte((uint8_t *)0x04, brightStar);
    eeprom_write_byte((uint8_t *)0x05, speedStripe);
    eeprom_write_byte((uint8_t *)0x06, speedStar);
    cli();
    digitalWrite(LED_BLUE, 0);
    digitalWrite(LED_GREEN, 0);
    digitalWrite(LED_RED, 0);
    digitalWrite(LED_YELLOW, 0);
    _delay_ms(1000);
    digitalWrite(LED_BLUE, 1);
    digitalWrite(LED_GREEN, 1);
    digitalWrite(LED_RED, 1);
    digitalWrite(LED_YELLOW, 1);
    save = false;
    sei();
  }
}

void setupInterrupt(void)
{
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR2A = 0;

  // Подключение прерывания по переполнению Timer2
  TIMSK2 = 1 << TOIE2;

  // Подключение прерывания по переполнению Timer1
  TIMSK1 = (1 << TOIE1);
  // Установить таймер на тактовой частоте:
  TCCR1B |= (1 << CS10);
  TCCR2B |= (1 << CS20);
  sei();
}
void setupStripe(Adafruit_NeoPixel &strip, uint8_t brightness)
{
  strip.begin();
  strip.show();
  strip.setBrightness(brightness);
}
// Some functions of our own for creating animated effects -----------------

void statefunction(Adafruit_NeoPixel &strip, char &programma, int &firstPixelHue, uint16_t &roundcount, char sp)
{

  uint32_t colRand = strip.gamma32(strip.ColorHSV((uint16_t)random()));
  uint8_t num = random();
  switch (programma)
  {

  case 0: // RANDOM CLEAR
  {
    // int k = millis();
    while (num > strip.numPixels())
    {
      num = random();
    }
    strip.setPixelColor(num, colRand);
    strip.show();
    // delay(10 * speed);
    strip.clear();
    // k = millis()-k;
    // Serial.print("0 Prog millis: ");Serial.println(k);
  }
  break;

  case 1: // RANDOM NON CLEAR
  {
    num = random(strip.numPixels());
    strip.setPixelColor(num, colRand);
    strip.show();
    // delay(10 * speed);
  }
  break;

  case 2: // RAINBOW
  {
    // int k = millis();
    for (int b = 0; b < 3; b++)
    {
      for (uint16_t c = b; c < strip.numPixels(); c += 3)
      {
        uint16_t hue = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color);                       // Set pixel 'c' to value 'color'
      }
      strip.show();                      // Update strip with new contents
      firstPixelHue += 65536 / (9 * sp); // One cycle of color wheel over 90 frames
    }
    // delay(10 * speed);
    // k = millis()-k;
    // Serial.print("2 Prog millis: ");Serial.println(k);
  }
  break;

  case 3: // ROUNDPIXEL
  {
    if (roundcount > strip.numPixels())
      roundcount = 0;
    strip.setPixelColor(roundcount, colRand);
    strip.setPixelColor(roundcount - 1, strip.Color(0, 0, 0));
    strip.show();
    roundcount++;
  }
  break;
  case 4: // SMALL SNOW
  {
    for (int i = 0; i < 5; i++)
    {
      uint8_t snows = random(100);
      num = random(strip.numPixels());
      strip.setPixelColor(num, strip.Color(snows, snows, snows));
    }
    strip.show();
    // delay(10 * speed);
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
        uint32_t color = strip.Color(hue, hue, hue); // hue -> RGB
        strip.setPixelColor(c, color);               // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      // delay(10 * speed);
    }
  }
  break;
  default:
  {
    programma = 0;
  }
  break;
  }
}
