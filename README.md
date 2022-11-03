Управление
===========
Использовать кнопку "Режим", Параметры:  

1. "Работа" - Все светодиоды отключены, на ёлке работает выбранная программа.  
При нажатии кнопок "+" и "-" одновременно - Сохранение текущих параметров.

2. "Выбор части ёлки" - Кнопками "+" и "-" выбрать требуемую к изменению часть ёлки:  
"%BLUE% светодиод постоянно горит" - Выбраны Звезда и Ёлка вместе.  
"%BLUE% cветодиод мерцает медленно" - Выбрана Ёлка.  
"%BLUE% cветодиод мерцает быстро" - Выбрана Звезда.  

3. "Выбор программы" - Кнопками "+" и "-" выбрать требуемую программу иллюминации.  
"Горит %GREEN% светодиод".

4. "Выбор Яркости" - Кнопками "+" и "-" выбрать требуемую яркость иллюминации.  
"Горит %RED% светодиод".

5. "Выбор скорости" - Кнопками "+" и "-" выбрать требуемую скорость иллюминации.  
"Горит %YELLOW% светодиод".


Установка/Изменение
===========
Для Arduino IDE:
----------------
Для использования требуется установка библиотеки "Adafruit NeoPixel" в менеджере библиотек  
Скетч ардуино лежит в папке xmastree  
До компиляции выбрать плату Arduino Pro Mini и выбрать микроконтроллер Atmega168 5V 16Mhz  

Для Platformio:
---------------
Создать новый проект  
Выбрать плату Arduino Pro Mini Atmega168 5V 16Mhz и фреймворк Arduino  
Для использования требуется установка библиотеки "Adafruit NeoPixel" в менеджере библиотек  
Заменить файл main.cpp в папке .../src/ вашего проекта на файл main.cpp из репозитория  

Загрузка скомпилированного проекта через AVRDUDEPROG:
-----------------------------------
Использовать файл скомпилированной прошивки firmware.hex  
Фьюзы:  
low_fuses=0xff  
high_fuses=0xdd  
extended_fuses=0xF8  


