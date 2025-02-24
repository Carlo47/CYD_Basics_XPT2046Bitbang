/**
 * Program      CYD_Basics with ESP32_2432S028R aka "Cheap Yellow Display (CYD)"
 * 
 * Author       2024-12-10 Charles Geiser, most code from Bodmer and Lovyan
 * 
 * Purpose      Shows how to configure the board and use lcd, touchscreen and SD card
 * 
 * Board        ESP32-2432S028 with touchscreen and SD card from AITEXM ROBOT
 *              https://www.aliexpress.com/item/1005005616073472.html?gps-id=pcStoreJustForYou&scm=1007.23125.137358.0&scm_id=1007.23125.137358.0&scm-url=1007.23125.137358.0&pvid=629012e6-491d-40f0-b41b-033335bc0c49&_t=gps-id:pcStoreJustForYou,scm-url:1007.23125.137358.0,pvid:629012e6-491d-40f0-b41b-033335bc0c49,tpp_buckets:668%232846%238114%231999&pdp_npi=4%40dis%21CHF%2110.65%218.62%21%21%2112.03%219.74%21%40210324bf17060488843367930ea758%2112000033759549673%21rec%21CH%21767770434%21&spm=a2g0o.store_pc_home.smartJustForYou_2007716161329.1005005616073472
 * 
 * Remarks      From what I have found out, the display, touchpad and SD card cannot
 *              be used independently of each other, although the SD card can bee 
 *              configured for both the HSPI_HOST or the VSPI_HOST.
 *              Either LCD and touchpad or LCD and SD card work together, but never 
 *              all three in conjunction.
 *              For example, it is not possible for me to write a screenshot to the  
 *              SD card, triggered by a touch.
 * 
 *              When I compare the saved screenshots with the displayed bitmaps, 
 *              the colors do not match, except the base colors R,G,B. Similarly, a  
 *              screen filled with TFT_BROWN appears more like TFT_OLIVE.
 * 
 * Eureka!      Now the problems are solved. The touchpad is now controlled by a 
 *              software SPI (see line 79) and the color problem could be solved by 
 *              swapping the basic colors before saving (see file saveBMPtoSD.cpp).
 *  
 * 
 * SPI pins                    SPI_HOST        SPIClass  MISO   MOSI   SCLK  CS  IRQ   Type
 *              LCD       SPI2_HOST=HSPI_HOST    HSPI     12     13     14   15   -    ILI9341  
 *              SD Card   SPI3_HOST=VSPI_HOST    VSPI     19     23     18    5   -    ENC28J60 
 *              Touchpad  SPI3_HOST=VSPI_HOST    VSPI     39     32     25   33   36   XPT2046
 * 
 * References   https://github.com/lovyan03/LovyanGFX
 *              https://github.com/bodmer
 *              https://github.com/rzeldent/platformio-espressif32-sunton/
 *              https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display
 *              https://www.youtube.com/watch?app=desktop&v=6JCLHIXXVus  (fixing the audio issues)
 *              https://chrishewett.com/blog/true-rgb565-colour-picker/
 * 
 *              https://registry.platformio.org/libraries/nitek/XPT2046_Bitbang_Slim (software SPI)
 * 
*/
#define LGFX_ESP32_2432S028

#include <Arduino.h>
#include <SD.h>
#include <LovyanGFX.hpp>
#include "XPT2046_Bitbang.h"
#include "lgfx_ESP32_2432S028.h"
#include "PulseGen.h"
#include "Turtle.h"
#include "Calibri12pt8b.h"
#include "Courier12pt8b.h"
#include "DejaVuSans10pt8b.h"
#include "OswaldMedium14pt8b.h"

/** CYD rotation definitions. The origin is always upper left corner
o-------------.    o---|¨|--.    o-------------.    o--------.
|  LANDSCAPE  |    |        |   -|  LANDSCAPE  |    |        |
|  USB RIGHT  |-   |PORTRAIT|   -|  USB LEFT   |    |PORTRAIT|
|             |-   | USB UP |    |             |    |USB DOWN|
'-------------'    |        |    '-------------'    |        |
                   |        |                       |        |
                   '--------'                       '--|_|---'
*/
enum class ROTATION { LANDSCAPE_USB_RIGHT, PORTRAIT_USB_UP, 
                      LANDSCAPE_USB_LEFT,  PORTRAIT_USB_DOWN };

using Action   = void(&)(LGFX &lcd);
using Activity = struct act{const char *name; Action f;};
extern GFXfont defaultFont;
GFXfont myFont = fonts::DejaVu12;
//GFXfont myFont = Calibri12pt8b;
//GFXfont myFont = Courier12pt8b;
//GFXfont myFont = DejaVuSans10pt8b;
//GFXfont myFont = OswaldMedium14pt8b;

LGFX lcd;
XPT2046_Bitbang touchpad(lcd, TP_MOSI, TP_MISO, TP_SCLK, TP_CS);

// These functions are defined in separate files
extern void calibrateTouchPad(LGFX &lcd);
extern void nop(LGFX &lcd);
extern void framedCrosshair(LGFX &lcd);
extern void grid(LGFX &lcd);
extern void initDisplay(LGFX &lcd, uint8_t rotation=0, GFXfont *theFont=&defaultFont, Action greet=nop);
extern void initSDCard(SPIClass &spi);
extern void lcdInfo(LGFX &lcd);
extern void listFiles(File dir, int indent=0);
extern void printSDCardInfo();
extern void printSystemInfo();
extern void rgb2hsv(uint8_t r, uint8_t g, uint8_t b, uint32_t &h, uint32_t &s, uint32_t &v);
extern bool saveBmpToSD_16bit(LGFX &lcd, const char *filename);
extern bool saveBmpToSD_24bit(LGFX &lcd, const char *filename);

// Examples defined in graphicPatterns.cpp
extern void barnsleyFern(LGFX &lcd);
extern void circles(LGFX &lcd);
extern void colorTiles(LGFX &lcd);
extern void colorGradients(LGFX &lcd);
extern void hsvColorCircle(LGFX &lcd);
extern void mandelbrot(LGFX &lcd);
extern void rainbowStripes(LGFX &lcd);
extern void randomDots(LGFX &lcd);
extern void rectangles(LGFX &lcd);
extern void rgbTiles(LGFX &lcd);
extern void rgbFrame(LGFX &lcd);
extern void roundRectangles(LGFX &lcd);
extern void sierpinskiTriangle(LGFX &lcd);
extern void triangles(LGFX &lcd);

// Recursive turtle graphics defined in fractals.cpp
extern void sevenSpirals(LGFX &lcd);
extern void fiveKochSnowflakes(LGFX &lcd);
extern void cCurves1(LGFX &lcd);
extern void cCurves2(LGFX &lcd);
extern void cCurves3(LGFX &lcd);
extern void dragonCurves1(LGFX &lcd);
extern void dragonCurves2(LGFX &lcd);
extern void dragonCurves3(LGFX &lcd);
extern void sierpinskiTriangles01(LGFX &lcd);
extern void sierpinskiTriangles23(LGFX &lcd);
extern void sierpinskiTriangles45(LGFX &lcd);
extern void shamrocks02(LGFX &lcd);
extern void shamrocks3(LGFX &lcd);
extern void shamrocks4(LGFX &lcd);

char promptText[] = " Touch to continue <>";

// All defined TFT-Colors
int color[] = {  TFT_BLACK,       TFT_RED,       TFT_MAROON,    TFT_BROWN,
                 TFT_ORANGE,      TFT_GOLD,      TFT_YELLOW,    TFT_OLIVE,
                 TFT_GREENYELLOW, TFT_GREEN,     TFT_DARKGREEN, TFT_DARKGREY,
                 TFT_DARKCYAN,    TFT_CYAN,      TFT_SKYBLUE,   TFT_BLUE,
                 TFT_NAVY,        TFT_VIOLET,    TFT_MAGENTA,   TFT_PURPLE,
                 TFT_LIGHTGREY,   TFT_SILVER,    TFT_PINK,      TFT_WHITE,
              };
int nbrOfColors = sizeof(color) / sizeof(color[0]);

// These RGB representations of the TFT-Colors are not used in this example program
uint8_t colorsRGB[][3] =
{ // R   G   B      Name                H     S      V
  {  0,  0,  0}, // TFT_BLACK           0    0.0    0.0
  {255,  0,  0}, // TFT_RED             0  100.0  100.0
  {123,  0,  0}, // TFT_MAROON          0  100.0   48.2
  {156, 77,  0}, // TFT_BROWN          30  100.0   61.2
  {255,182,  0}, // TFT_ORANGE         43  100.0  100.0
  {255,215,  0}, // TFT_GOLD           51  100.0  100.0
  {255,255,  0}, // TFT_YELLOW         60  100.0  100.0
  {123,125,  0}, // TFT_OLIVE          61  100.0   49.0
  {181,255,  0}, // TFT_GREENYELLOW    77  100.0  100.0
  {  0,255,  0}, // TFT_GREEN         120  100.0  100.0
  {  0,125,  0}, // TFT_DARGREEN      120  100.0   49.0
  {123,125,123}, // TFT_DARKGREY      120    1.6   49.0
  {  0,125,123}, // TFT_DARKCYAN      179  100.0   49.0
  {  0,255,255}, // TFT_CYAN          180  100.0  100.0
  {132,206,239}, // TFT_SKYBLUE       199   44.8   93.7
  {  0,  0,255}, // TFT_BLUE          240  100.0  100.0
  {  0,  0,123}, // TFT_NAVY          240  100.0   48.2
  {148, 40,230}, // TFT_VIOLET        274   82.6   90.2  
  {255,  0,255}, // TFT_MAGENTA       300  100.0  100.0
  {123,  0,123}, // TFT_PURPLE        300  100.0   48.2
  {214,210,214}, // TFT_LIGHTGREY     300    1.9   83.9      
  {197,194,197}, // TFT_SILVER        300    1.5   77.3
  {255,194,206}, // TFT_PINK          348   23.9  100.0
  {255,255,255}, // TFT_WHITE           0    0.0  100.0 
};
constexpr int nbrOfColorsRGB = sizeof(colorsRGB)/sizeof(colorsRGB[0]);

// https://github.com/newdigate/rgb565_colors
int rainbowColor[] = 
{
    TFT_RED,    // = 0xF800
    0xFD20,     // TFT_ORANGE=0xFDA0
    TFT_YELLOW, // = 0xFFE0
    TFT_GREEN,  // = 0x07E0
    TFT_BLUE,   // = 0x001F
    0x4810,     // INDIGO,
    0x781F,     // TFT_VIOLET=0x915C 
};
constexpr int nbrOfRainbowColors = sizeof(rainbowColor) / sizeof(rainbowColor[0]);

Activity activity[] = {  
                        {"RGB_Tiles",        rgbTiles},
                        {"Rainbow_Stripes",  rainbowStripes},
                        {"Color_Tiles",      colorTiles},
                        {"Color_Gradients",  colorGradients},
                        {"Circles",          circles},
                        {"Rectangles",       rectangles},
                        {"Round_Rectangles", roundRectangles},
                        {"Triangles",        triangles},
                        {"HSV_ColorCircle",  hsvColorCircle},
                        {"Random_Dots",      randomDots},
                        {"Barnsley_Fern",    barnsleyFern}, 
                        {"Mandelbrot",       mandelbrot},
                        {"Sierpinski",       sierpinskiTriangle},
                        {"Spirals",          sevenSpirals},
                        {"Snowflakes",       fiveKochSnowflakes},
                        {"C_Curves1",        cCurves1},
                        {"C_Curves2",        cCurves2},
                        {"C_Curves3",        cCurves3},
                        {"Dragon_Curves1",   dragonCurves1},
                        {"Dragon_Curves2",   dragonCurves2},
                        {"Dragon_Curves3",   dragonCurves3},
                        {"Sierpinski_01",    sierpinskiTriangles01},
                        {"Sierpinski_23",    sierpinskiTriangles23},
                        {"Sierpinski_45",    sierpinskiTriangles45},
                        {"Shamrocks_02",     shamrocks02},
                        {"Shamrocks_3",      shamrocks3},
                        {"Shamrocks_4",      shamrocks4},
                      };
constexpr int nbrActivities = sizeof(activity) / sizeof(activity[0]);

SPIClass sdcardSPI(VSPI); // Saved bitmaps on SD card are OK, but LGFX touchpad doesn't work
/**
 * LCD    SD    TS      Img write   Touch
 * HSPI   HSPI  HSPI      white      nok 
 * HSPI   HSPI  VSPI      white       ok
 * HSPI   VSPI  HSPI      ok         nok    Since none of the possible combinations work 
 * HSPI   VSPI  VSPI      ok         nok    for all 3 devices (LCD, SD CARD, TOUCHPAD),
 *                                          it is best to use a software SPI solution
 * VSPI   VSPI  VSPI      black      nok    for the touchpad (class XPT2046_Bitbang).
 * VSPI   VSPI  HSPI      white       ok
 * VSPI   HSPI  VSPI      black       ok
 * VSPI   HSPI  HSPI      ok         nok
*/


/**
 * Generates 3 pulse generators, each of which  
 * causes one of the RGB LEDs to flash.
 * Runs as an independent task on the second core.
*/
void blinkTask(void* arg)
{
  PulseGen pulseGenR(RGB_LED_R, 3000000, 100000, 0);
  PulseGen pulseGenG(RGB_LED_G, 3000000, 100000, 3000000/3);
  PulseGen pulseGenB(RGB_LED_B, 3000000, 100000, 2*3000000/3);
    // Enable the pulse generators
  pulseGenR.on();
  pulseGenG.on();
  pulseGenB.on();
  while (true)
  {
    // Let the LEDs blink
    pulseGenR.loop();
    pulseGenG.loop();
    pulseGenB.loop();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}


/**
 * Save a screenshot as 24 bit bmp image
 */
void takeScreenshot(const char *filename)
{   
    saveBmpToSD_24bit(lcd, filename);
    Serial.printf("Screenshot saved: %s\n", filename);
}


void promptAt(int x, int y, const char txt[])
{
  lcd.setCursor(x, y);
  lcd.print(txt);
  while (! touchpad.getTouch()) delay(100);
}


void readJpgFile()
{
  lcd.clear();
  promptAt(0, 0, R"(
  Read a jpg color swatch and
  save it as rgb565 bitmap
  to SD card. The saved colors
  are not correct and must be 
  rotated from rgb to brg with
  a program like XnView <>)");
  lcd.drawJpgFile("/sd/saved/jpg/Tentakel.jpg", 0,0) ? log_e("file opened") : log_e("file not found");
  takeScreenshot("/Screenshots/screen00.bmp");
  delay(1000);
  promptAt(0, 300, promptText);
}

void readBmpFile()
{
  lcd.clear();
  promptAt(0, 0, R"(
  Read a bmp picture and save
  it as rgb565 bitmap to SD
  card. The saved colors are
  not correct and must be 
  rotated from rgb to brg with
  a program like XnView <>)");
  lcd.drawBmpFile("/sd/saved/bmp/dodeka.bmp", 0,0) ? log_e("file opened") : log_e("file not found");
  takeScreenshot("/screenshot/sreen01.bmp");
  delay(1000);
  promptAt(0, 300, promptText);
}


void showInstructions()
{
  promptAt(0, 100, R"(
  Touch the screen to exercise
  touch operations:
  
  - short touch
  - long touch
  - swipe right
  - swipe up
  - swipe left
  - swipe down
  
  or touch here [ ] to start
  the demo.)");
}


void demo()
{
  readJpgFile();
  readBmpFile();
  lcd.clear();
  lcd.setCursor(0,10);
  promptAt(0, 0, R"(
Touch the screen to start
a pattern show. After each
pattern touch again to get
to the next. <>)");

  // Show all activities (graphic patterns)
  for( int i = 0; i < nbrActivities; i++)
  {
    char buf[64];
    Serial.printf("%s\n", activity[i].name);
    activity[i].f(lcd);  // call the activity
    snprintf(buf, 64, "/Screenshots/16bit/%02d_%s_16.bmp", i, activity[i].name);
    saveBmpToSD_16bit(lcd, buf);  // save the screen to SD card as RGB565
    snprintf(buf, 64, "/Screenshots/24bit/%02d_%s_24.bmp", i, activity[i].name);
    saveBmpToSD_24bit(lcd, buf); // save the screen to SD card as RGB888
    promptAt(0, 300, "");
  }
  delay(200);
  lcd.clear();
  promptAt(0, 280, " The End!\n Touch to continue <>"); 
  lcd.clear();
  showInstructions();
}


void shortTouch(int x, int y)
{
  if (touchpad.touchedAt(x, y, 120, 255, 10, 10))
    {
      demo();
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.printf("%*c", 50, ' ');
      lcd.setCursor(0,0);
      lcd.printf("short touched at %4d / %4d   ", x, y);
    }
}


void longTouch(int x, int y)
{
  lcd.setCursor(0,0);
  lcd.printf("%*c", 50, ' ');
  lcd.setCursor(0,0);
  lcd.printf("long touched at %4d / %4d   ", x, y);
}

void swipeRight(int x, int y)
{
  lcd.setCursor(0,0);
  lcd.printf("%*c", 50, ' ');
  lcd.setCursor(0,0);
  lcd.print("swipe right");
}

void swipeLeft(int x, int y)
{
  lcd.setCursor(0,0);
  lcd.printf("%*c", 50, ' ');
  lcd.setCursor(0,0);
  lcd.print("swipe left");
}


void swipeDown(int x, int y)
{
  lcd.setCursor(0,0);
  lcd.printf("%*c", 50, ' ');
  lcd.setCursor(0,0);
  lcd.print("swipe down");
}

void swipeUp(int x, int y)
{
  lcd.setCursor(0,0);
  lcd.printf("%*c", 50, ' ');
  lcd.setCursor(0,0);
  lcd.print("swipe up");
}


void setup() 
{
  Serial.begin(115200);

  // Starts the blinking task, which causes the RGB LED to flash 
  // red, green and blue alternately every second
  xTaskCreate(blinkTask, "blinkTask", 1024, NULL, 10, NULL);
  printSystemInfo();
  initDisplay(lcd, static_cast<uint8_t>(ROTATION::PORTRAIT_USB_UP), &myFont, grid);
  lcd.clear();
  touchpad.begin();
  touchpad.addShortTouchCb(shortTouch);
  touchpad.addLongTouchCb(longTouch);
  touchpad.addSwipeRightCb(swipeRight);
  touchpad.addSwipeUpCb(swipeUp);
  touchpad.addSwipeLeftCb(swipeLeft);
  touchpad.addSwipeDownCb(swipeDown);
  initSDCard(sdcardSPI);
  printSDCardInfo();
  listFiles(SD.open("/"));
  showInstructions();
  log_e("==> done");
}

void loop()
{
  touchpad.loop();
}
