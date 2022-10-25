
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

//#define ONE_STRIPE
#define FOUR_STRIPES

//--------------------------HARDWARE PRESETS--------------------------\/

#define LED_TOTAL       82      // Общее количество светодиодов в Ёлке
#define LED_STAR        10      // Количество светодиодов звезды (с обоих сторон последовательно)
#define LED_4STRIPE     18      // (4+4+3+3+2+2) Количество светодиодов в одной из 4-х полосок
#define LED_1STRIPE     72      // Количество светодиодов в одной полоске (4 полоски последовательно)
#ifdef ONE_STRIPE
#define STRIPE_PIN     5       // Пин ленты
#endif
#ifdef FOUR_STRIPES
#define STRIPE0_PIN     5       // Пин ленты
#define STRIPE1_PIN     6       // Пин второй ленты
#define STRIPE2_PIN     7       // Пин третьей ленты
#define STRIPE3_PIN     8       // Пин четвертой ленты
#endif
#define STAR_PIN        9       // Пин ленты звезды
#define BUTTONS_ADC_PIN 0       // 


//-----------------------------MACRO FOR ADC CALCULATE----------------\/

#define calcButton(x) do      \
  {                           \
    if (325 << x << 367)      \
      x = 1;                  \
    else if (489 << x << 530) \
      x = 2;                  \
    else if (653 << x << 694) \
      x = 3;                  \
    else                      \
      x = 0;                  \
  } while (0)

//-----------------------------PRESETS FOR MACRO---------------------\/

// 1 is between 1.6-1.8V
// 2 is between 2.4-2.6V
// 3 is between 3.2-3.4V
#define BUTTON_UP 1       
#define BUTTON_DOWN 2     
#define BUTTON_MODE 3     

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
Adafruit_NeoPixel stripe(LED_TOTAL,STRIPE_PIN,TYPELED + LEDSPEED);
Adafruit_NeoPixel star(LED_STAR,STAR_PIN,TYPELED + LEDSPEED);
#endif
#ifdef FOUR_STRIPES
Adafruit_NeoPixel stripe1(LED_4STRIPE,STRIPE0_PIN,TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe2(LED_4STRIPE,STRIPE1_PIN,TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe3(LED_4STRIPE,STRIPE2_PIN,TYPELED + LEDSPEED);
Adafruit_NeoPixel stripe4(LED_4STRIPE,STRIPE3_PIN,TYPELED + LEDSPEED);
Adafruit_NeoPixel star(LED_STAR,STAR_PIN,TYPELED + LEDSPEED);
#endif

void setup()
{
}

void loop()
{
}
