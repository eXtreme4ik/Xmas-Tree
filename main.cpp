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

//--------------------------HARDWARE PRESETS--------------------------\/

#define LED_TOTAL 82   // Общее количество светодиодов в Ёлке
#define LED_STAR 10    // Количество светодиодов звезды (с обоих сторон последовательно)
#define LED_4STRIPE 18 // (4+4+3+3+2+2) Количество светодиодов в одной из 4-х полосок
#define LED_1STRIPE 72 // Количество светодиодов в одной полоске (4 полоски последовательно)

#define STRIPE0_PIN 8         // Пин ленты
#define STRIPE1_PIN 7         // Пин второй ленты
#define STRIPE2_PIN 6         // Пин третьей ленты
#define STRIPE3_PIN 5         // Пин четвертой ленты
#define LED_STRIPE_UP 18      // Макс значение для верхнего уровня
#define LED_STRIPE_BOTTOM 8   // Макс значение для нижнего уровня
#define LED_STRIPE_BETWEEN 14 // Макс значение для среднего уровня

#define STAR_PIN 9         // Пин ленты звезды
#define BUTTONS_ADC_PIN 14 // Пин кнопок
#define LED_BLUE 17        // Пин синего светодиода VD2
#define LED_GREEN 16       // Пин зеленого светодиода VD3
#define LED_RED 18         // Пин красного светодиода VD1
#define LED_YELLOW 15      // Пин желтого светодиода VD4

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
#define SPEED_MAX 20000            // Максимально возможное замедление
#define SPEED_STEP 200              // Шаг изменения замедления
#define SPEED_MIN 200               // Минимально возможное замедление (Далее всё сливается в кучу)
#define CORRECT_SPEED_RAINBOW 10    // Коэффициент коррекции скорости режима rainbow
#define SPEED_RAINBOW_COEF 512      // Переменная коррекции скорости режима rainbow
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

#define BUTTON_UP SW2          // Выбор значения напряжения для кнопки 2.4-2.6V (+)
#define BUTTON_DOWN SW1        // Выбор значения напряжения для кнопки 1.6-1.8V (-)
#define BUTTON_MODE SW3        // Выбор значения напряжения для кнопки 3.2-3.4V (Режим)
#define BUTTON_UP_AND_DOWN SW4 // Выбор значения напряжения для кнопки 2.9-3.1V (- и Режим одновременно)

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

Adafruit_NeoPixel stripe1(LED_4STRIPE, STRIPE0_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe2(LED_4STRIPE, STRIPE1_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe3(LED_4STRIPE, STRIPE2_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe4(LED_4STRIPE, STRIPE3_PIN, TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe[4];
Adafruit_NeoPixel star(LED_STAR, STAR_PIN, TYPELED + LEDSPEED);

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
    @param  &roundcount Каунтер цикла
    @param  sp Параметр скорости Rainbow
    @return  void
  */
void buildStripe(Adafruit_NeoPixel &strip, char &programma, int &firstPixelHue, uint16_t &roundcount, int sp);
/*!
    @brief  Функция вызова цикла программы для 4-х строк
    @param  &programma указатель на переменную действующей программы
    @param  &firstPixelHue Последний параметр Hue для первого пикселя режима Rainbow
    @param  &roundcount Каунтер цикла
    @param  sp Параметр скорости
    @return  void
  */
void buildTree(char &programma, int &firstPixelHue, uint16_t &roundcount, int sp);
/*!
    @brief  Функция отображения результата игры
    @return  void
  */
void scoreControl(void);

//----------------------------VARIABLES-----------------------------\/
char rxBuffer[100]; // Буфер UART (Не используется)

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
uint8_t staticCounterStripe = 0;  // Каунтер статичного цвета Ёлки
uint8_t staticCounterStar = 0;    // Каунтер статичного цвета Звезды
int mode = 0;                     // Каунтер режимов
int submode = 0;                  // Каунтер подрежима выбора
unsigned int gameCount = 0;       // Каунтер времени для игры
//----------------------------SETTINGS (BASE)-----------------------\/
// БАЗОВЫЕ УСТАНОВКИ ПРИ ПЕРВОМ СТАРТЕ!
char progStripe = 4;             // Программа Ёлки
char progStar = 4;               // Программа Звезды
unsigned int brightStripe = 50;  // Яркость Ёлки
unsigned int speedStripe = 1000; // Скорость Ёлки
unsigned int brightStar = 50;    // Яркость Звезды
unsigned int speedStar = 1000;   // Скорость звезды
// Флаги
char firststart = 1;             // Флаг первого запуска
char testLight = false;          // Флаг тестового Белого цвета
volatile bool save = false;               // Флаг сохранения параметров
volatile bool up = false;                 // Флаг нажатия кнопки +
volatile bool dn = false;                 // Флаг нажатия кнопки -
volatile bool md = false;                 // Флаг нажатия кнопки Режим
bool sync = false;               // Флаг синхронизации программы звезды и ёлки
volatile bool flagLight = false; // Флаг включения режима свечения в игре
volatile bool flagGame = false;  // Флаг включения режима игры
volatile bool tap1 = false;      // Флаг нажатия кнопки (Порт UART)
volatile bool tap2 = false;      // Флаг нажатия кнопки (Порт UART)

// Переменные режима игры (Опционально)
volatile uint16_t treeStripe = 0;   // Переменная хранения рандомного значения строки светодиодов
volatile uint16_t treeLed = 0;      // Переменная хранения рандомного значения светодиода в строке
volatile unsigned int p1score = 0;  // Переменная счёта игрока 1
volatile unsigned int p2score = 0;  // Переменная счёта игрока 2
volatile unsigned int gameStop;     // Переменная для хранения времени включения Лампы в игре
volatile uint8_t colorGameP1 = 0;   // Переменная выбранного цвета игроком 1
volatile uint8_t colorGameP2 = 0;   // Переменная выбранного цвета игроком 2
volatile uint8_t trueColorGame = 0; // Переменная реального цвета

//----------------------------STATIC LIGHTS-------------------------\/
static const uint32_t staticColor[] PROGMEM{
    0xFF0000, // RED
    0xFF7F00, // YELLOW RED
    0xFFFF00, // YELLOW
    0x7FFF00, // YELLOW GREEN
    0x00FF00, // GREEN
    0x00FF7F, // CYAN GREEN
    0x00FFFF, // CYAN
    0x007FFF, // CYAN BLUE
    0x0000FF, // BLUE
    0x7F00FF, // MAGENTA BLUE
    0xFF00FF, // MAGENTA
    0xFF007F, // MAGENTA RED
              // ... and more
};
static const uint32_t staticColorGame[] PROGMEM{
    0xFF0000, // RED
    // 0xFFFF00, // YELLOW
    0x00FF00, // GREEN
    // 0x00FFFF, // CYAN
    0x0000FF, // BLUE
              // 0xFF00FF, // MAGENTA
              // ... and more
};
const uint8_t staticColorMax = (sizeof(staticColor) / sizeof(uint32_t)) - 1;
const uint8_t staticColorGameMax = (sizeof(staticColorGame) / sizeof(uint32_t));

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
    eeprom_write_byte((uint8_t *)0x08, staticCounterStripe);
    eeprom_write_byte((uint8_t *)0x09, staticCounterStar);
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
    staticCounterStripe = eeprom_read_byte((uint8_t *)0x08);
    staticCounterStar = eeprom_read_byte((uint8_t *)0x09);
  }
  // Запуск белого цвета если в EEPROM положить кодовую комбинацию F0
  if (progStripe == (char)0xF0)
    testLight = true;
  // Инициализируем порты светодиодов
  pinMode(LED_MODE, OUTPUT);
  pinMode(LED_BRIGHT, OUTPUT);
  pinMode(LED_PROGRAMM, OUTPUT);
  pinMode(LED_SPEED, OUTPUT);
  // Инициализируем пины для режима игры
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  // Настраиваем таймер1 и таймер2 на прерывания по переполнению
  setupInterrupt();
  // Записываем В массив для простоты обращения к полоскам светодиодов (Да, это просто растрата памяти)
  stripe[0] = stripe1;
  stripe[1] = stripe2;
  stripe[2] = stripe3;
  stripe[3] = stripe4;
  // И настраиваем Дерево
  for (int i = 0; i < 4; i++)
    setupStripe(stripe[i], 50);
  // Настраиваем Звезду
  setupStripe(star, 50);
  // Запуск белого цвета если в EEPROM положить кодовую комбинацию F0
  while (testLight) // Пока не прервём кнопкой MODE
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
  randomSeed(analogRead(19)); // Организуем рандом из космоса
  gameStop = random(2000);    // Присваиваем рандомное значение для игры
  trueColorGame = random(staticColorGameMax);
  colorGameP2 = random(staticColorGameMax);
  colorGameP1 = random(staticColorGameMax);
}
//....................................................................................
//....................................................................................
void loop()
{
  // Выставляем яркость
  for (int i = 0; i < 4; i++)
    stripe[i].setBrightness(brightStripe);
  star.setBrightness(brightStar);
  // Цикл игры
  if (flagGame)
  {
    if (!flagLight) // То чистим ёлку
    {
      for (int i = 0; i < 4; i++)
      {
        stripe[i].fill(0);
        stripe[i].show();
      }
    }
    if (gameCount == gameStop) // То включаем рандомный светодиод
    {
      flagLight = true;
      uint32_t color = pgm_read_dword(&staticColorGame[trueColorGame]);
      stripe[treeStripe].setPixelColor(treeLed, color);
      stripe[treeStripe].show();
    }
    if (gameCount == (gameStop + 1000)) // Выключаем рандомный светодиод и обновляем значения для следующего цикла
    {
      flagLight = false;
      treeStripe = random(4);
      treeLed = random(18);
      gameStop = random(2000);
      trueColorGame = random(staticColorGameMax);
      for (int i = 0; i < 4; i++)
      {
        stripe[i].fill(0);
        stripe[i].show();
      }
      gameCount = 0;
    }
    if (gameCount > 5000)
      gameCount = 0;
    scoreControl(); // Выводим значения статистики на Ёлку и Звезду
    gameCount++;    // Увеличиваем цикл
  }

  else // Используем стандартную иллюминацию
  {
    //................................................................
    if (speedStripe <= progCountStripe) // Если попали в Каунтер, запускаем функцию Ёлки
    {
      buildTree(progStripe, firstPixelHueStripe, roundcountStripe, speedStripe / CORRECT_SPEED_RAINBOW);
      progCountStripe = 1; // Обнуляем каунтер
    }

    //................................................................
    if (speedStar <= progCountStar) // Если попали в Каунтер, запускаем функцию Звезды
    {
      // Если программа Ёлки и Звезды - бегущий огонь, синхронизируем их
      //------------------------------------------------------------------
      if (progStar == 4 && progStripe == 4)
      {
        if (roundcountStripe == stripe[0].numPixels())
          sync = true;
        if (sync == true)
        {
          buildStripe(star, progStar, firstPixelHueStar, roundcountStar, speedStar / CORRECT_SPEED_RAINBOW);
        }
        if (roundcountStar == star.numPixels() + 1)
        {
          star.clear();
          star.show();
          sync = false;
        }
      }
      //------------------------------------------------------------------
      else // Иначе просто запускаем программу
        buildStripe(star, progStar, firstPixelHueStar, roundcountStar, speedStar / CORRECT_SPEED_RAINBOW);
      progCountStar = 1;
    }

    //................................................................
    progCountStar++;   // Увеличиваем Каунтеры каждый цикл
    progCountStripe++; // Увеличиваем Каунтеры каждый цикл

    //................................................................
    if (save == true) // Сохраняем параметры
    {

      eeprom_write_byte((uint8_t *)0x00, 13);
      eeprom_write_byte((uint8_t *)0x01, progStripe);
      eeprom_write_byte((uint8_t *)0x02, progStar);
      eeprom_write_byte((uint8_t *)0x03, brightStripe);
      eeprom_write_byte((uint8_t *)0x04, brightStar);
      eeprom_write_byte((uint8_t *)0x05, speedStripe / 100);
      eeprom_write_byte((uint8_t *)0x06, speedStar / 100);
      eeprom_write_byte((uint8_t *)0x07, submode);
      eeprom_write_byte((uint8_t *)0x08, staticCounterStripe);
      eeprom_write_byte((uint8_t *)0x09, staticCounterStar);
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
void buildStripe(Adafruit_NeoPixel &strip, char &programma, int &firstPixelHue, uint16_t &roundcount, int sp)
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
    for (uint16_t n = 0; n < strip.numPixels(); n++)
    {
      strip.setPixelColor(n, pgm_read_dword(&staticColor[staticCounterStar]));
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
      uint16_t hue = firstPixelHue + roundcount * 65536L / (strip.numPixels() / 2);
      uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
      strip.setPixelColor(roundcount, color);
      strip.show();
    }
    firstPixelHue += 65535 / 32;
    roundcount++;
  }
  break;
  case 10: // RANDOM PLACE or CLEAR
  {
    num = random(strip.numPixels());
    if (strip.getPixelColor(num))
      colRand = 0;
    strip.setPixelColor(num, colRand);
    strip.show();
  }
  break;
  case 11: // FIRE TREE
  {
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
      uint8_t flicker = random(1, 70);
      uint8_t r = 255;
      uint8_t g = flicker;
      uint8_t b = g / 10;
      strip.setPixelColor(i, strip.Color(r, g, b));
      strip.show();
    }
  }
  break;
  case 12: // XMAS LIGHTS
  {
    for (uint16_t c = 0; c < 3; c++)
    {
      uint8_t snowR = random(2);
      uint8_t snowG;
      if (snowR)
      {
        snowR = 0xFF;
        snowG = 0x00;
      }
      if (!snowR)
      {
        snowR = 0x00;
        snowG = 0xFF;
      }
      uint32_t color = strip.Color(snowR, snowG, 0);
      strip.setPixelColor(random(strip.numPixels()), color);
    }
    strip.show();
    strip.clear();
  }
  break;
  default:
  {
    if (programma > 12)
      programma = 1;
    if (programma == 0)
    {
      programma = 12;
    }
  }
  break;
  }
}
//....................................................................................
//....................................................................................
void buildTree(char &programma, int &firstPixelHue, uint16_t &roundcount, int sp)
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
    for (int i = 0; i < 4; i++)
    {
      for (uint16_t n = 0; n < stripe[0].numPixels(); n++)
      {
        stripe[i].setPixelColor(n, pgm_read_dword(&staticColor[staticCounterStripe]));
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
      uint16_t hue = firstPixelHue + roundcount * 65536L / (stripe[0].numPixels() + 1);
      uint32_t color = stripe[0].gamma32(stripe[0].ColorHSV(hue)); // hue -> RGB
      stripe[i].setPixelColor(roundcount, color);
      // colRand = stripe[0].gamma32(stripe[0].ColorHSV((uint16_t)random()));
      // stripe[i].setPixelColor(roundcount, colRand);
      stripe[i].show();
    }
    firstPixelHue += 65535 / 32;
    roundcount++;
  }
  break;
  case 10: // RANDOM PLACE or CLEAR
  {
    num = random(stripe[0].numPixels());
    colRand = stripe[0].gamma32(stripe[0].ColorHSV((uint16_t)random()));
    for (int i = 0; i < 4; i++)
    {
      if (stripe[i].getPixelColor(num))
        colRand = 0;
      stripe[i].setPixelColor(num, colRand);
      stripe[i].show();
    }
  }
  break;
  case 11: // FIRE TREE
  {
    for (uint16_t i = 0; i < stripe1.numPixels(); i++)
    {
      
      uint8_t flicker = random(1, 70);
      uint8_t r = 255; 
      uint8_t g = flicker;
      uint8_t b = g / 10;
      for (int n = 0; n < 4; n++)
      {
        stripe[n].setPixelColor(i, stripe1.Color(r, g, b));
      }
    }
    for (int i = 0; i < 4; i++)
    {
      stripe[i].show();
    }
  }
  break;
  case 12: // XMAS LIGHTS
  {
    for (uint16_t c = 0; c < 3; c++)
    {
      uint8_t snowR = random(2);
      uint8_t snowG;
      if (snowR)
      {
        snowR = 0xFF;
        snowG = 0x00;
      }
      if (!snowR)
      {
        snowR = 0x00;
        snowG = 0xFF;
      }
      uint32_t color = stripe[0].Color(snowR, snowG, 0);
      for (int i = 0; i < 4; i++)
        stripe[i].setPixelColor(random(stripe1.numPixels()), color);
    }
    for (int i = 0; i < 4; i++)
    {
      stripe[i].show();
      stripe[i].clear();
    }
  }
  break;
  default:
  {
    if (programma > 12)
      programma = 1;
    if (programma == 0)
      programma = 12;
  }
  break;
  }
}
//....................................................................................
//.......................ПРЕРЫВАНИЕ...................................................
extern "C" void __vector_9(void) __attribute__((signal, used, externally_visible));
void __vector_9(void)
{
  if (tap1) // Манипуляции по нажатию кнопки
  {
    if (flagLight && trueColorGame == colorGameP1)
    {
      p1score++;
      colorGameP1 = random(staticColorGameMax);
    }
    else
    {
      if (p1score)
        p1score--;
    }
    flagLight = false;
    tap1 = false;
  }

  if (tap2 && trueColorGame == colorGameP2) // Манипуляции по нажатию кнопки
  {
    if (flagLight)
    {
      p2score++;
      colorGameP2 = random(staticColorGameMax);
    }
    else
    {
      if (p2score)
        p2score--;
    }
    flagLight = false;
    tap2 = false;
  }
  switch (mode) // Организация свечения статусных светодиодов согласно режиму
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
  if (mode != MODE_DEFAULT) // Организация мерцания Синего светодиода
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
//.......................ПРЕРЫВАНИЕ...................................................
extern "C" void __vector_13(void) __attribute__((signal, used, externally_visible));
void __vector_13(void)
{
  int value = analogRead(BUTTONS_ADC_PIN);
  calcButton(value);
  if (timeButton) // Если задержка
    timeButton--;
  if (!digitalRead(0) && !timeButton)
  {
    if (flagGame)
      tap1 = true;
    else
    {
      flagGame = true;
      treeLed = random(18);
      treeStripe = random(4);
      trueColorGame = random(staticColorGameMax);
    }
    timeButton = 50;
  }
  if (!digitalRead(1) && !timeButton)
  {
    if (flagGame)
      tap2 = true;
    else
    {
      flagGame = true;
      treeLed = random(18);
      treeStripe = random(4);
      trueColorGame = random(staticColorGameMax);
    }
    timeButton = 50;
  }

  if (value && !timeButton) // Манипуляции по нажатию кнопок ёлки
  {
    if (value == BUTTON_UP_AND_DOWN)
    {
      timeButton = 50;
    }
    if (value == BUTTON_UP)
    {
      up = true;
      timeButton = 50;
    }
    if (value == BUTTON_DOWN)
    {
      dn = true;
      timeButton = 50;
    }
    if (value == BUTTON_MODE)
    {
      md = true;
      flagGame = false;
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
        progStripe = 3;
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
          if (progStripe == 8) // Если статик цвет
          {
            staticCounterStripe++;
            if (staticCounterStripe > staticColorMax)
              staticCounterStripe = 0;
          }
          else
            speedStripe -= SPEED_STEP;
          up = false;
        }
        if (dn)
        {
          if (progStripe == 8) // Если статик цвет
          {
            if (staticCounterStripe == 0)
              staticCounterStripe = staticColorMax;
            else
              staticCounterStripe--;
          }
          else
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
          if (progStar == 8) // Если статик цвет
          {
            staticCounterStar++;
            if (staticCounterStar > staticColorMax)
              staticCounterStar = 0;
          }
          else
            speedStar -= SPEED_STEP;
          up = false;
        }
        if (dn)
        {
          if (progStar == 8) // Если статик цвет
          {
            if (staticCounterStar == 0)
              staticCounterStar = staticColorMax;
            else
              staticCounterStar--;
          }
          else
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
          if (progStripe == 8) // Если статик цвет
          {
            staticCounterStripe++;
            if (staticCounterStripe > staticColorMax)
              staticCounterStripe = 0;
          }
          else
            speedStripe -= SPEED_STEP;
          speedStar = speedStripe;
          staticCounterStar = staticCounterStripe;
          up = false;
        }
        if (dn)
        {
          if (progStripe == 8) // Если статик цвет
          {
            if (staticCounterStripe == 0)
              staticCounterStripe = staticColorMax;
            else
              staticCounterStripe--;
          }
          else
            speedStripe += SPEED_STEP;
          speedStar = speedStripe;
          staticCounterStar = staticCounterStripe;
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
void scoreControl(void)
{
  star.clear();
  for (unsigned int i = 0; i <= p2score; i++)
  {
    if (i != 0)
      star.setPixelColor(i - 1, 0x707000);
    star.setPixelColor(i, pgm_read_dword(&staticColorGame[colorGameP2]));
  }
  for (unsigned int i = 5; i <= p1score + 5; i++)
  {
    if (i != 5)
      star.setPixelColor(i - 1, 0x007070);
    star.setPixelColor(i, pgm_read_dword(&staticColorGame[colorGameP1]));
  }
  star.show();
  if (p2score >= 5)
  {
    for (int i = 0; i < 4; i++)
    {
      for (uint16_t n = 0; n < stripe[0].numPixels(); n++)
      {
        stripe[i].setPixelColor(n, 0x707000);
      }
      stripe[i].show();
    }
    p2score = 0;
    p1score = 0;
    flagGame = false;
    gameStop = random(2000);
    delay(3000);
    for (int i = 0; i < 4; i++)
    {
      for (uint16_t n = 0; n < stripe[0].numPixels(); n++)
      {
        stripe[i].setPixelColor(n, 0);
      }
      stripe[i].show();
    }
  }
  if (p1score >= 5)
  {
    for (int i = 0; i < 4; i++)
    {
      for (uint16_t n = 0; n < stripe[0].numPixels(); n++)
      {
        stripe[i].setPixelColor(n, 0x007070);
      }
      stripe[i].show();
    }
    p1score = 0;
    p2score = 0;
    flagGame = false;
    gameStop = random(2000);
    delay(3000);
    for (int i = 0; i < 4; i++)
    {
      for (uint16_t n = 0; n < stripe[0].numPixels(); n++)
      {
        stripe[i].setPixelColor(n, 0);
      }
      stripe[i].show();
    }
  }
}