#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "lgfx_ESP32_2432S028.h"
#include <SPI.h>


using Action = void(&)(LGFX &lcd);

GFXfont defaultFont = fonts::DejaVu18;
void nop(LGFX &lcd){};


/**
 * Draw a framed crosshair
*/
void framedCrosshair(LGFX &lcd)
{
  char str[24]; 
  lcd.fillScreen(TFT_BLACK);
  lcd.drawRect(0,0, lcd.width(), lcd.height(), TFT_RED);
  lcd.drawLine(0,0, lcd.width(), lcd.height(), TFT_GREEN);
  lcd.drawLine(lcd.width(),0, 0, lcd.height(), TFT_BLUE);
  lcd.fillRect(0,0, 20, 20, TFT_GREEN);
  lcd.fillRect(lcd.width()-10,lcd.height()-10, 10, 10, TFT_RED);;
  lcd.setTextSize(1.0);
  sprintf(str, "(0,0) origin, rot=%d", lcd.getRotation());
  lcd.drawString(str, 25,0);
}

/**
 * Draw a grid 20 x 20 
*/
void grid(LGFX &lcd)
{
  int x = 0, y = 0, d = 20;
  lcd.fillScreen(TFT_BLACK);
  while (y < lcd.height())
  {
    lcd.drawLine(0, y, lcd.width(), y, TFT_WHITE);
    y += d;
  }

  while (x < lcd.width())
  {
    lcd.drawLine(x, 0, x, lcd.height(), TFT_WHITE);
    x += d;
  }
  
}

/**
 * Show some facts about the display
*/
void lcdInfo(LGFX &lcd)
{
  Serial.printf(R"(
LCD Info
--------
width x height = %d x %d 
rotation       = %d 
color depth    = %d 
text size X    = %4.2f
text size Y    = %4.2f
)", lcd.width(), lcd.height(), lcd.getRotation(), lcd.getColorDepth(),
    lcd.getTextSizeX(), lcd.getTextSizeY());
  Serial.printf("\n");
}


/**
 * Initialize display and call the greeting function.
 * The default for greeting is nop(). To calibrate the 
 * touchscreen call it as initDisplay(lcd, calibrateTouchScreen).
 * The greeting function takes as argument the passed lcd
*/
void initDisplay(LGFX &lcd, uint8_t rotation=0, GFXfont *theFont=&defaultFont, Action greet=nop)
  {
    if (lcd.begin())
    {
      lcd.clear();
      lcd.setColorDepth(24);
      lcd.setTextSize(1.0);
      lcd.setTextDatum(lgfx::textdatum::TL_DATUM);
      lcd.setFont(theFont);
      lcd.setRotation(rotation);
      lcd.setBrightness(255);
      greet(lcd);
    }
    log_i("==> done");
  }
