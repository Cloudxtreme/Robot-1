
//#define sclk 13
//#define mosi 11
#define cs   10
#define dc   9
#define rst  8  // you can also connect this to the Arduino reset

#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <ProFont_ST7735.h>

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);
ProFont_ST7735 proFont = ProFont_ST7735(tft);

void setup() {
  tft.initR(INITR_BLACKTAB); // initialize a ST7735R chip, red tab
  tft.fillScreen(tft.Color565(0,0,0));
}

void loop() {
  proFont.testScreen(true);
  delay(1000);
};

