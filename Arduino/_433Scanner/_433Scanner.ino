/***************************************************
  Derived from:
  an example sketch for the Adafruit 1.8" SPI display.
  This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
  as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618
 
  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

//
// TFT-Signal lines Pro Mini
//
#define cs    10
#define dc    9
#define rst   8
//
// Hardware SPI Pro Mini
//
// pin 11 = MOSI
// pin 12 = MISO
// pin 13 = SCK.
//
// SD-Card signal lines 
//
#define cs_sd 7

#define SERIAL_BAUD  57600

int addr = 0;
int eedata;
File root;

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

void setup(void) 
{
  // pinMode(cs, OUTPUT);

  Serial.begin(SERIAL_BAUD);
     
  eedata = EEPROM.read(addr);
  Serial.print(" eedata read = ");
  Serial.println(eedata);
  
  uint8_t v2 = EEPROM.read(addr+1);
  uint8_t v3 = EEPROM.read(addr+2);
  
  Serial.print(" read v2 = ");
  Serial.println(v2);
  Serial.print(" read v3 = ");
  Serial.println(v3);
  
  if( v2 == 255 && v3 == 255 )
  {
    v2 = 0x10;
    v3 = 0x00;
    EEPROM.write(addr+1, v2);
    EEPROM.write(addr+2, v3);
  }

  int xx = v2 << 8 + v3;

  Serial.print(" xx = ");
  Serial.println(xx);
     
  tft.initR(INITR_BLACKTAB);
//  tft.setAddrWindow(0,0,128,160);
  tft.setRotation(3);
  tft.setTextWrap(true);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);    

  Serial.print("Initializing SD card...");

  if (!SD.begin(cs_sd)) 
  {
    Serial.println("failed!");
  }
  else
  {
    Serial.println("Init OK!");
    root = SD.open("/");
    printDirectory(root, 0);
  }

}

void rotateCross(int xstat)
{
  switch(xstat)
  {
    case 1:
      tft.setCursor(2, 2);
      //tft.setTextColor(ST7735_RED, ST7735_BLACK);
      tft.print(" ");
      tft.setCursor(2, 2);
      //tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.print("|");
      break;
    case 2:
      tft.setCursor(2, 2);
      //tft.setTextColor(ST7735_RED, ST7735_BLACK);
      tft.print(" ");
      tft.setCursor(2, 2);
      //tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.print("/");
      break;
    case 3:
      tft.setCursor(2, 2);
      //tft.setTextColor(ST7735_RED, ST7735_BLACK);
      tft.print(" ");
      tft.setCursor(2, 2);
      //tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.print("-");
      break;
    case 4:
      tft.setCursor(2, 2);
      //tft.setTextColor(ST7735_RED, ST7735_BLACK);
      tft.print(" ");
      tft.setCursor(2, 2);
      //tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.print("\\");
      break;
    default:
      break;      
  }
}

void runningPoint(int num)
{
  int i;

  tft.setTextSize(2);
  tft.setTextColor(ST7735_RED, ST7735_BLACK);    

  tft.setCursor(1, 1);
  tft.print("1");

  tft.setCursor(10, 10);
  tft.print("2");

  tft.setCursor(20, 20);
  tft.print("3");

  tft.setCursor(30, 30);
  tft.print("4");
  
   tft.setCursor(40, 40);
  tft.print("5");

  tft.setCursor(50, 50);
  tft.print("6");
  
  tft.setCursor(60, 60);
  tft.print("7");
  
  delay(1000);


  //tft.setTextSize(3);
  //tft.setTextColor(ST7735_WHITE, ST7735_BLACK);    
  
}

void tft_clean(void)
{

  // tft.setRotation(1);
  // tft.setTextWrap(true);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);    
  tft.setCursor(0, 0);

}

void loop() {
  static int vstat = 0;
  static int run_var = 0;
  static int mode = 0;
  
  run_var++;
  
  if(++vstat == 5)
  {
    vstat=0;
  }

  if(run_var == 5)
  {
    mode++;
  }
  else
  {
    if(run_var == 10)
    {
      mode++;
    }
    else
    {
      if(run_var == 15)
      {
        run_var = 0;
      }
    }
  }
  
  switch(mode)
  {
    case 0:
Serial.println("CASE 0");
      tft_clean();
      rotateCross(vstat);
      break;
    case 1:
Serial.println("CASE 1");   
      tft_clean();
      runningPoint(20);
      break;
    case 2:
Serial.println("CASE 2");
      tft_clean();
      tft.setTextSize(3);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);    
      tft.setCursor(0, 0);
      //tft.drawChar( 50, 5, 'x', ST7735_WHITE, ST7735_BLACK, 1);
      for (int i = 0; i < 8; i++)
        tft.print("x");
      delay(500);

      for (int i = 0; i < 8; i++)
        tft.print("x");
      delay(500);

      // Fontsize = 1 -> 26 Zeilen, 16 Spalten
      // Fontsize = 2 -> 13 Zeilen, 8 Spalten
      // Fontsize = 3 -> 8 Zeilen, 5 Spalten
      // Pixels: 6x8, 12x16, 18x24
      
      tft.setTextSize(3);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);    
      tft.setCursor(0, 0);

      for(int i = 0; i < 5; i++)
        tft.println(i);
      delay(500);
      
      mode = 0;
      break;
  }
}


void tftPrintTest() {
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setTextColor(ST7735_GREEN);
  //tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST7735_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST7735_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST7735_WHITE);
  tft.print(" seconds.");
}

void printDirectory(File dir, int numTabs) {
  // Begin at the start of the directory
  dir.rewindDirectory();
  
  while(true) {
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');   // we'll have a nice indentation
     }
     // Print the 8.3 name
     Serial.print(entry.name());
     // Recurse for directories, otherwise print the file size
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}



