#include <Arduino.h>
// Copy from here-----------------------------------------------------\/
//----,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,----------------------------------\/
//---|                              |---------------------------------\/
//---|  "Xmas tree" Arduino sketch  |---------------------------------\/
//---|                              |---------------------------------\/
//---````````````````````````````````---------------------------------\/
// Author: Evgeniy Suhminin
//--------------------------INCLUDES----------------------------------\/

#include <Adafruit_NeoPixel.h> // Установить через менеджер библиотек

//--------------------------USER PRESETS------------------------------\/

// #define ONE_STRIPE
#define FOUR_STRIPES

//--------------------------HARDWARE PRESETS--------------------------\/

#define LED_TOTAL 82 // Общее количество светодиодов в Ёлке
// #define LED_STAR 8              // Количество светодиодов звезды (с обоих сторон последовательно) (TEST)
#define LED_STAR 10 // Количество светодиодов звезды (с обоих сторон последовательно)
// #define LED_4STRIPE 16          // (4+4+3+3+2+2) Количество светодиодов в одной из 4-х полосок (TEST)
#define LED_4STRIPE 18 // (4+4+3+3+2+2) Количество светодиодов в одной из 4-х полосок
// #define LED_1STRIPE 32           // Количество светодиодов в одной полоске (4 полоски последовательно) (TEST)
#define LED_1STRIPE 72 // Количество светодиодов в одной полоске (4 полоски последовательно)

#ifdef ONE_STRIPE
#define STRIPE_PIN 5 // Пин ленты
#endif

#ifdef FOUR_STRIPES
#define STRIPE0_PIN 8         // Пин ленты
#define STRIPE1_PIN 7         // Пин второй ленты
#define STRIPE2_PIN 6         // Пин третьей ленты
#define STRIPE3_PIN 5         // Пин четвертой ленты
#define LED_STRIPE_UP 18      // Макс значение для верхнего уровня
#define LED_STRIPE_BOTTOM 8   // Макс значение для нижнего уровня
#define LED_STRIPE_BETWEEN 14 // Макс значение для среднего уровня
#endif

#define STAR_PIN 9         // Пин ленты звезды
#define BUTTONS_ADC_PIN 14 // Пин кнопок
#define LED_BLUE 17        // Пин синего светодиода
#define LED_GREEN 16       // Пин зеленого светодиода
#define LED_RED 18         // Пин красного светодиода
#define LED_YELLOW 15      // Пин желтого светодиода

//--------------------------SOFTWARE PRESETS--------------------------\/

#define MODE_DEFAULT 0    // Основной режим (При нажатии + и - одновременно - сохранить параметры)
#define MODE_STRIPE 1     // Режим выбора группы светодиодов (Все вместе, Звезда, Ёлка)
#define MODE_PROGRAMM 2   // Режим выбора программы свечения
#define MODE_BRIGHTLESS 3 // Режим выбора яркости
#define MODE_SPEED 4      // Режим выбора скорости
#define MAXMODES 5        // Максимальное количество режимов

#define SUBMODE_STRIPE 1 // Подрежим - Ёлка
#define SUBMODE_STAR 2   // Подрежим - Звезда
#define SUBMODE_ALL 0    // Подрежим - Всё вместе

#define LED_MODE LED_BLUE      // Светодиод ответственный за выбор Елки/звезды
#define LED_PROGRAMM LED_GREEN // Светодиод ответственный за выбор программы
#define LED_BRIGHT LED_RED     // Светодиод ответственный за выбор яркости
#define LED_SPEED LED_YELLOW   // Светодиод ответственный за выбор скорости

#define BRIGHT_MAX 255              // Максимально возможная яркость
#define SPEED_MAX 10000             // Максимально возможное замедление
#define SPEED_STEP 500              // Шаг изменения замедления
#define SPEED_MIN 500               // Минимально возможное замедление (Далее всё сливается в кучу)
#define CORRECT_SPEED_RAINBOW 10    // Коэффициент коррекции скорости режима rainbow
#define SPEED_RAINBOW_COEF sp       // Переменная коррекции скорости режима rainbow
#define BRIGHT_STEP BRIGHT_MAX / 30 // Шаг изменения яркости

//-----------------------------MACRO FOR ADC CALCULATE----------------\/

#define calcButton(x)            \
  do                             \
  {                              \
    if (327 < x && x < 368)      \
      x = 1;                     \
    else if (491 < x && x < 532) \
      x = 2;                     \
    else if (655 < x && x < 696) \
      x = 3;                     \
    else if (593 < x && x < 634) \
      x = 4;                     \
    else                         \
      x = 0;                     \
  } while (0)
#define SW1 1 // 1 is between 1.6-1.8V (VCC=5V) ~SW1
#define SW2 2 // 2 is between 2.4-2.6V (VCC=5V) ~SW2
#define SW3 3 // 3 is between 3.2-3.4V (VCC=5V) ~SW3
#define SW4 4 // 4 is between 2.9-3.1V (VCC=5V) ~SW4

//-----------------------------PRESETS FOR MACRO---------------------\/

#define BUTTON_UP SW2          // Выбор значения напряжения для кнопки 1.6-1.8V
#define BUTTON_DOWN SW1        // Выбор значения напряжения для кнопки 3.2-3.4V
#define BUTTON_MODE SW3        // Выбор значения напряжения для кнопки 2.4-2.6V
#define BUTTON_UP_AND_DOWN SW4 // Выбор значения напряжения для кнопки 2.9-3.1V

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

#define TYPELED NEO_GRB     // NEO_GRB (пиксели привязаны к битовому потоку GRB - лента WS2812)
#define LEDSPEED NEO_KHZ800 // NEO_KHZ800 (800 кГц - указывается для светодиодов WS2812)

#ifdef ONE_STRIPE
Adafruit_NeoPixel stripe(LED_1STRIPE, STRIPE_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel star(LED_STAR, STAR_PIN, TYPELED + LEDSPEED);
#endif
#ifdef FOUR_STRIPES
Adafruit_NeoPixel stripe1(LED_4STRIPE, STRIPE0_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe2(LED_4STRIPE, STRIPE1_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe3(LED_4STRIPE, STRIPE2_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe4(LED_4STRIPE, STRIPE3_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe[4];
Adafruit_NeoPixel star(LED_STAR, STAR_PIN, TYPELED + LEDSPEED);
#endif

//----------------------------FUNCTION PROTOTYPES-----------------------\/
/*!
    @brief  Функция настройки таймеров TIMER1 и TIMER2
    @return  void
  */
void setupInterrupt(void);
/*!
    @brief  Функция первичной инициализации объектов
    @param  &strip указатель на объект Adafruit_NeoPixel
    @param  brightness установка первоначальной яркости (Первый запуск)
    @return  void
  */
void setupStripe(Adafruit_NeoPixel &strip, uint8_t brightness);
/*!
    @brief  Функция вызова цикла программы
    @param  &strip указатель на объект Adafruit_NeoPixel
    @param  &programma указатель на переменную действующей программы
    @param  &firstPixelHue Последний параметр Hue для первого пикселя режима Rainbow
    @param  &roundcount Каунтер
    @param  sp Параметр скорости Rainbow
    @return  void
  */
void statefunction(Adafruit_NeoPixel &strip, char &programma, int &firstPixelHue, uint16_t &roundcount, int sp);
#ifdef FOUR_STRIPES
/*!
    @brief  Функция вызова цикла программы для 4-х строк
    @param  &programma указатель на переменную действующей программы
    @param  &firstPixelHue Последний параметр Hue для первого пикселя режима Rainbow
    @param  &roundcount Каунтер
    @param  sp Параметр скорости Rainbow
    @return  void
  */
void statefunction4stripes(char &programma, int &firstPixelHue, uint16_t &roundcount, int sp);
#endif

//----------------------------VARIABLES-----------------------------\/

//----------------------------CYCLE---------------------------------\/
// Переменные для циклически неповторяющихся эффектов
int firstPixelHueStripe = 0;
uint16_t roundcountStripe = 0;
int firstPixelHueStar = 0;
uint16_t roundcountStar = 0;
//----------------------------COUNTERS------------------------------\/
char timeButton = 0;              // Каунтер для нажатия кнопок
char strobe = 0;                  // Каунтер для статусных светодиодов
unsigned int progCountStripe = 0; // Каунтер для программ Ёлки
unsigned int progCountStar = 0;   // Каунтер для программ Звезды
int mode = 0;                     // Каунтер режимов
int submode = 0;                  // Каунтер подрежима выбора
//----------------------------SETTINGS (BASE)-----------------------\/
// БАЗОВЫЕ УСТАНОВКИ ПРИ ПЕРВОМ СТАРТЕ!
char progStripe = 4;             // Программа Ёлки
char progStar = 4;               // Программа Звезды
unsigned int brightStripe = 50;  // Яркость Ёлки
unsigned int speedStripe = 1000; // Скорость Ёлки
unsigned int brightStar = 50;    // Яркость Звезды
unsigned int speedStar = 1000;   // Скорость звезды
// Флаги
char firststart = 1; // Флаг первого запуска
char testLight = false;
bool save = false; // Флаг сохранения параметров
bool up = false;   // Флаг нажатия кнопки +
bool dn = false;   // Флаг нажатия кнопки -
bool md = false;   // Флаг нажатия кнопки Режим
bool flag = false; // Флаг нажатия любой из кнопок
bool sync = false; // Флаг синхронизации программы звезды и ёлки
//....................................................................................
//....................................................................................
//....................................................................................
void setup()
{
  firststart = eeprom_read_byte((uint8_t *)0x00);
  if (firststart != 13) // Если первый запуск Запоминаем стандартно проинициализированные переменные
  {
    eeprom_write_byte((uint8_t *)0x00, 13); // Записываем значение первого запуска
    eeprom_write_byte((uint8_t *)0x01, progStripe);
    eeprom_write_byte((uint8_t *)0x02, progStar);
    eeprom_write_byte((uint8_t *)0x03, brightStripe);
    eeprom_write_byte((uint8_t *)0x04, brightStar);
    eeprom_write_byte((uint8_t *)0x05, speedStripe / 100); // Уменьшаем, чтобы влезло в байт
    eeprom_write_byte((uint8_t *)0x06, speedStar / 100);   // То же
    eeprom_write_byte((uint8_t *)0x07, submode);
  }
  else // Если не первый запуск загружаем переменные из ЕЕPROM
  {
    progStripe = eeprom_read_byte((uint8_t *)0x01);
    progStar = eeprom_read_byte((uint8_t *)0x02);
    brightStripe = eeprom_read_byte((uint8_t *)0x03);
    brightStar = eeprom_read_byte((uint8_t *)0x04);
    speedStripe = eeprom_read_byte((uint8_t *)0x05) * 100;
    speedStar = eeprom_read_byte((uint8_t *)0x06) * 100;
    submode = eeprom_read_byte((uint8_t *)0x07);
  }
  if (progStripe == (char)0xF0)
    testLight = true;
  // Запускаем отладочный UART
  Serial.begin(9600);
  // Инициализируем порты светодиодов
  pinMode(LED_MODE, OUTPUT);
  pinMode(LED_BRIGHT, OUTPUT);
  pinMode(LED_PROGRAMM, OUTPUT);
  pinMode(LED_SPEED, OUTPUT);
  // Настраиваем таймер1 и таймер2 на прерывания по переполнению
  setupInterrupt();
  //
#ifdef ONE_STRIPE
  setupStripe(stripe, 100);
#endif
#ifdef FOUR_STRIPES
  // Записываем В массив для простоты обращения к полоскам светодиодов
  stripe[0] = stripe1;
  stripe[1] = stripe2;
  stripe[2] = stripe3;
  stripe[3] = stripe4;
  // И настраиваем полоски
  for (int i = 0; i < 4; i++)
    setupStripe(stripe[i], 50);
#endif
  // настраиваем Звезду
  setupStripe(star, 50);
  while (testLight)
  {
    for (int i = 0; i < 4; i++)
      stripe[i].setBrightness(170);
    star.setBrightness(170);
    for (int i = 0; i < 4; i++)
    {
      for (uint16_t n = 0; n < stripe[0].numPixels(); n++)
        stripe[i].setPixelColor(n, stripe[0].Color(255, 255, 255));
    }
    for (uint16_t i = 0; i < star.numPixels(); i++)
      star.setPixelColor(i, star.Color(255, 255, 255));
    for (int i = 0; i < 4; i++)
      stripe[i].show();
    star.show();
  }
}
//....................................................................................
//....................................................................................
void loop()
{
  // Serial.println(speedStripe);
  for (int i = 0; i < 4; i++)
    stripe[i].setBrightness(brightStripe);
  star.setBrightness(brightStar);
  if (speedStripe - 1 <= progCountStripe) // Если попали в Каунтер, запускаем функцию Ёлки
  {
    statefunction4stripes(progStripe, firstPixelHueStripe, roundcountStripe, speedStripe / CORRECT_SPEED_RAINBOW);
    progCountStripe = 0; // Обнуляем каунтер
  }
  if (speedStar - 1 <= progCountStar) // Если попали в Каунтер, запускаем функцию Звезды
  {
    if (progStar == 4 && progStripe == 4) // Если программа Ёлки и Звезды - бегущий огонь, синхронизируем их
    {
      if (roundcountStripe == stripe[0].numPixels())
        sync = true;
      if (sync == true)
      {
        statefunction(star, progStar, firstPixelHueStar, roundcountStar, speedStar / CORRECT_SPEED_RAINBOW);
      }
      if (roundcountStar == star.numPixels() + 1)
      {
        star.clear();
        star.show();
        sync = false;
      }
    }
    else // Иначе просто запускаем программу
      statefunction(star, progStar, firstPixelHueStar, roundcountStar, speedStar / CORRECT_SPEED_RAINBOW);
    progCountStar = 0;
  }
  progCountStar++;   // Увеличиваем Каунтеры каждый цикл
  progCountStripe++; // Увеличиваем Каунтеры каждый цикл
  if (save == true)
  {

    eeprom_write_byte((uint8_t *)0x00, 13);
    eeprom_write_byte((uint8_t *)0x01, progStripe);
    eeprom_write_byte((uint8_t *)0x02, progStar);
    eeprom_write_byte((uint8_t *)0x03, brightStripe);
    eeprom_write_byte((uint8_t *)0x04, brightStar);
    eeprom_write_byte((uint8_t *)0x05, speedStripe / 100);
    eeprom_write_byte((uint8_t *)0x06, speedStar / 100);
    eeprom_write_byte((uint8_t *)0x07, submode);
    cli();
    digitalWrite(LED_MODE, 0);
    digitalWrite(LED_PROGRAMM, 0);
    digitalWrite(LED_BRIGHT, 0);
    digitalWrite(LED_SPEED, 0);
    _delay_ms(1000);
    digitalWrite(LED_MODE, 1);
    digitalWrite(LED_PROGRAMM, 1);
    digitalWrite(LED_BRIGHT, 1);
    digitalWrite(LED_SPEED, 1);
    save = false;
    sei();
  }
}
//....................................................................................
//....................................................................................
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
//....................................................................................
//....................................................................................
void setupStripe(Adafruit_NeoPixel &strip, uint8_t brightness)
{
  strip.begin();
  strip.show();
  strip.setBrightness(brightness);
}
//....................................................................................
//....................................................................................
void statefunction(Adafruit_NeoPixel &strip, char &programma, int &firstPixelHue, uint16_t &roundcount, int sp)
{
  uint32_t colRand = strip.gamma32(strip.ColorHSV((uint16_t)random()));
  uint8_t num = random();
  switch (programma)
  {

  case 1: // RANDOM CLEAR
  {
    // int k = millis();
    while (num > strip.numPixels())
    {
      num = random();
    }
    strip.setPixelColor(num, colRand);
    strip.show();
    strip.clear();
  }
  break;

  case 2: // RANDOM NON CLEAR
  {
    num = random(strip.numPixels());
    strip.setPixelColor(num, colRand);
    strip.show();
  }
  break;

  case 3: // RAINBOW
  {
    for (int b = 0; b < 3; b++)
    {
      for (uint16_t c = b; c < strip.numPixels(); c += 3)
      {
        uint16_t hue = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color);                       // Set pixel 'c' to value 'color'
      }
      strip.show();                                // Update strip with new contents
      firstPixelHue += 65536 / SPEED_RAINBOW_COEF; // One cycle of color wheel over 90 frames
    }
  }
  break;

  case 4: // ROUNDPIXEL
  {
    if (roundcount > strip.numPixels())
    {
      roundcount = 0;
    }
    strip.setPixelColor(roundcount, colRand);
    strip.setPixelColor(roundcount - 1, strip.Color(0, 0, 0));
    strip.show();
    roundcount++;
  }
  break;
  case 5: // SMALL SNOW
  {
    for (int i = 0; i < 5; i++)
    {
      uint8_t snows = random(255);
      uint8_t snowB = snows - snows / 3;
      num = random(strip.numPixels());
      strip.setPixelColor(num, strip.Color(snows, snows, snowB));
    }
    strip.show();
    strip.clear();
  }
  break;
  case 6: // SNOW STORM
  {
    for (int b = 0; b < 3; b++)
    {
      for (uint16_t c = b; c < strip.numPixels(); c += 3)
      {
        uint8_t snows = random(255);
        uint8_t snowB = snows - snows / 5;
        uint32_t color = strip.Color(snows, snows, snowB);
        strip.setPixelColor(c, color);
      }
      strip.show();
    }
  }
  break;
#ifdef FOUR_STRIPES
  case 7: // RAINBOW PER 1 SITE
  {
    for (int a = 0; a < 5; a++)
    {
      uint16_t hue = firstPixelHue;
      uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
      strip.setPixelColor(a, color);
    }
    for (int a = 5; a < 10; a++)
    {
      uint16_t hue = firstPixelHue + 65536 / 5;
      uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
      strip.setPixelColor(a, color);
    }
    strip.show();
    firstPixelHue += 65536 / SPEED_RAINBOW_COEF;
  }
  break;
  case 8: // SPEED=STATIC COLOR
  {
    uint32_t col = strip.gamma32(strip.ColorHSV((speedStar * 65535) / 10000));

    for (uint16_t n = 0; n < strip.numPixels(); n++)
    {
      strip.setPixelColor(n, col);
    }
    strip.show();
  }
  break;
  case 9: // ROUNDPIXEL (Non clear)
  {
    if (roundcount > stripe[0].numPixels() + 1)
    {
      roundcount = 0;
      for (int i = 0; i < 4; i++)
        strip.clear();
    }

    for (int i = 0; i < 4; i++)
    {
      colRand = strip.gamma32(strip.ColorHSV((uint16_t)random()));
      strip.setPixelColor(roundcount, colRand);
      strip.show();
    }
    roundcount++;
  }
  break;
#endif
  default:
  {
    if (programma > 9)
      programma = 1;
    if (programma == 0)
    {
      programma = 9;
    }
  }
  break;
  }
}
#ifdef FOUR_STRIPES // ФУНКЦИЯ ОБРАБОТКИ ОДНОВРЕМЕННО 4-Х ПОЛОСОК
void statefunction4stripes(char &programma, int &firstPixelHue, uint16_t &roundcount, int sp)
{
  uint32_t colRand;
  uint8_t num;
  switch (programma)
  {

  case 1: // RANDOM CLEAR
  {
    // int k = millis();

    for (int i = 0; i < 4; i++)
    {
      colRand = stripe[0].gamma32(stripe[0].ColorHSV((uint16_t)random()));
      num = random(stripe[0].numPixels());
      stripe[i].setPixelColor(num, colRand);
      stripe[i].show();
      stripe[i].clear();
    }
  }
  break;

  case 2: // RANDOM NON CLEAR
  {
    num = random(stripe[0].numPixels());
    colRand = stripe[0].gamma32(stripe[0].ColorHSV((uint16_t)random()));
    for (int i = 0; i < 4; i++)
    {
      stripe[i].setPixelColor(num, colRand);
      stripe[i].show();
    }
  }
  break;

  case 3: // RAINBOW
  {
    for (int b = 0; b < 3; b++)
    {
      for (uint16_t c = b; c < stripe[0].numPixels(); c += 3)
      {
        uint16_t hue = firstPixelHue + c * 65536L / stripe[0].numPixels();
        uint32_t color = stripe[0].gamma32(stripe[0].ColorHSV(hue)); // hue -> RGB
        for (int i = 0; i < 4; i++)
          stripe[i].setPixelColor(c, color);
      }
      for (int i = 0; i < 4; i++)
        stripe[i].show();
      firstPixelHue += 65536 / SPEED_RAINBOW_COEF;
    }
  }
  break;

  case 4: // ROUNDPIXEL
  {
    if (roundcount > stripe[0].numPixels() + 1)
    {
      roundcount = 0;
    }

    for (int i = 0; i < 4; i++)
    {
      colRand = stripe[0].gamma32(stripe[0].ColorHSV((uint16_t)random()));
      stripe[i].setPixelColor(roundcount, colRand);
      stripe[i].setPixelColor(roundcount - 1, stripe[i].Color(0, 0, 0));
      stripe[i].show();
    }
    roundcount++;
  }
  break;
  case 5: // SMALL SNOW
  {
    for (int i = 0; i < 3; i++)
    {
      uint8_t snows = random(255);
      uint8_t snowB = snows - snows / 5;
      uint32_t color = stripe[0].Color(snows, snows, snowB);
      num = random(stripe[0].numPixels());
      for (int i = 0; i < 4; i++)
        stripe[i].setPixelColor(num, color);
    }
    for (int i = 0; i < 4; i++)
    {
      stripe[i].show();
      stripe[i].clear();
    }
  }
  break;
  case 6: // SNOW STORM
  {
    for (int b = 0; b < 3; b++)
    {
      for (uint16_t c = b; c < stripe[0].numPixels(); c += 3)
      {
        uint8_t snows = random(255);
        uint8_t snowB = snows - snows / 5;
        uint32_t color = stripe[0].Color(snows, snows, snowB);
        for (int i = 0; i < 4; i++)
          stripe[i].setPixelColor(c, color);
      }
      for (int i = 0; i < 4; i++)
        stripe[i].show();
    }
  }
  break;
  case 7: // RAINBOW PER 1 SITE
  {
    for (int i = 0; i < 4; i++)
    {
      for (int a = 0; a < LED_STRIPE_BOTTOM; a++)
      {
        uint16_t hue = firstPixelHue + 65536 / 4;
        uint32_t color = stripe[0].gamma32(stripe[0].ColorHSV(hue)); // hue -> RGB
        stripe[i].setPixelColor(a, color);
      }
      for (int a = LED_STRIPE_BOTTOM; a < LED_STRIPE_BETWEEN; a++)
      {
        uint16_t hue = firstPixelHue + 65536 / 4 + 65536 / 4;
        uint32_t color = stripe[0].gamma32(stripe[0].ColorHSV(hue)); // hue -> RGB
        stripe[i].setPixelColor(a, color);
      }
      for (int a = LED_STRIPE_BETWEEN; a < LED_STRIPE_UP; a++)
      {
        uint16_t hue = firstPixelHue + 65536 / 4 + 65536 / 4 + 65536 / 4;
        uint32_t color = stripe[0].gamma32(stripe[0].ColorHSV(hue)); // hue -> RGB
        stripe[i].setPixelColor(a, color);
      }
      stripe[i].show();
      firstPixelHue += 65536 / 4;
    }
    firstPixelHue += 65536 / SPEED_RAINBOW_COEF;
  }
  break;
  case 8: // SPEED=STATIC COLOR
  {
    uint32_t col = stripe[0].gamma32(stripe[0].ColorHSV((uint16_t)((speedStripe * 65535) / 10000)));
    for (int i = 0; i < 4; i++)
    {
      for (uint16_t n = 0; n < stripe[0].numPixels(); n++)
      {
        stripe[i].setPixelColor(n, col);
      }
      stripe[i].show();
    }
  }
  break;
  case 9: // ROUNDPIXEL (Non clear)
  {
    if (roundcount > stripe[0].numPixels() + 1)
    {
      roundcount = 0;
      for (int i = 0; i < 4; i++)
        stripe[i].clear();
    }

    for (int i = 0; i < 4; i++)
    {
      colRand = stripe[0].gamma32(stripe[0].ColorHSV((uint16_t)random()));
      stripe[i].setPixelColor(roundcount, colRand);
      stripe[i].show();
    }
    roundcount++;
  }
  break;
  default:
  {
    if (programma > 9)
      programma = 1;
    if (programma == 0)
      programma = 9;
  }
  break;
  }
}
#endif
extern "C" void __vector_9(void) __attribute__((signal, used, externally_visible));
void __vector_9(void)
{
  switch (mode)
  {
  case MODE_DEFAULT:
  {
    digitalWrite(LED_MODE, 1);
    digitalWrite(LED_PROGRAMM, 1);
    digitalWrite(LED_BRIGHT, 1);
    digitalWrite(LED_SPEED, 1);
  }
  break;
  case MODE_STRIPE:
  {
    digitalWrite(LED_PROGRAMM, 1);
    digitalWrite(LED_BRIGHT, 1);
    digitalWrite(LED_SPEED, 1);
  }
  break;
  case MODE_PROGRAMM:
  {
    digitalWrite(LED_PROGRAMM, 0);
    digitalWrite(LED_BRIGHT, 1);
    digitalWrite(LED_SPEED, 1);
  }
  break;
  case MODE_BRIGHTLESS:
  {
    digitalWrite(LED_BRIGHT, 0);
    digitalWrite(LED_SPEED, 1);
    digitalWrite(LED_PROGRAMM, 1);
  }
  break;
  case MODE_SPEED:
  {
    digitalWrite(LED_SPEED, 0);
    digitalWrite(LED_BRIGHT, 1);
    digitalWrite(LED_PROGRAMM, 1);
  }
  break;
  }
  if (mode != MODE_DEFAULT)
  {
    if (submode == SUBMODE_ALL)
    {
      digitalWrite(LED_MODE, 0);
    }
    if (submode == SUBMODE_STRIPE)
    {
      if (strobe == 50)
        digitalWrite(LED_MODE, 0);
      if (strobe > 100)
      {
        digitalWrite(LED_MODE, 1);
        strobe = 0;
      }
    }
    if (submode == SUBMODE_STAR)
    {
      if (strobe == 50)
        digitalWrite(LED_MODE, 0);
      if (strobe > 55)
      {
        digitalWrite(LED_MODE, 1);
        strobe = 0;
      }
    }
    strobe++;
  }
}
//....................................................................................
//....................................................................................
extern "C" void __vector_13(void) __attribute__((signal, used, externally_visible));
void __vector_13(void)
{
  int value = analogRead(BUTTONS_ADC_PIN);
  calcButton(value);
  if (timeButton)
    timeButton--;
  if (value && !timeButton)
  {
    if (value == BUTTON_UP_AND_DOWN)
    {
      timeButton = 50;
      flag = true;
    }
    if (value == BUTTON_UP)
    {
      up = true;
      flag = true;
      timeButton = 50;
    }
    if (value == BUTTON_DOWN)
    {
      dn = true;
      flag = true;
      timeButton = 50;
    }
    if (value == BUTTON_MODE)
    {
      md = true;
      flag = true;
      timeButton = 50;
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
      if (value == BUTTON_UP_AND_DOWN)
        save = true;
      up = false;
      dn = false;
    }
    if (mode == MODE_STRIPE)
    {
      if (testLight)
      {
        progStripe = 5;
        eeprom_write_byte((uint8_t *)0x01, progStripe);
        testLight = false;
      }
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
      // testLight = 0;
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
        if (brightStripe < BRIGHT_STEP)
          brightStripe = BRIGHT_STEP;
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
        if (brightStar < BRIGHT_STEP)
          brightStar = BRIGHT_STEP;
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
        if (brightStripe < BRIGHT_STEP)
        {
          brightStripe = BRIGHT_STEP;
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
        if (speedStripe < SPEED_MIN)
          speedStripe = SPEED_MIN;
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
        if (speedStar < SPEED_MIN)
          speedStar = SPEED_MIN;
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
        if (speedStripe < SPEED_MIN)
        {
          speedStripe = SPEED_MIN;
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
}