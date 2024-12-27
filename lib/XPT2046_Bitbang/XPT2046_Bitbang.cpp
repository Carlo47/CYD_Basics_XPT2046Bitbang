/**
 * Class        XPT2046_Bitbang.cpp
 *
 * Author       2024-12-24 Charles Geiser (https://www.dodeka.ch)
 *
 * Purpose      A software SPI class for the touchpad XPT2046 of the CYD ESP32-2432S028R. 
 *              It is an extension of the class by Claus Näveke nitek/XPT2046_Bitbang_Slim
 *              and also implements detection of
 *                - shortTouch
 *                - longTouch
 *                - swipes right, up, right, down
 *                - and a loop() function which can be called in the main loop. 
 *              To react to events in the loop function, 6 callback functions can be installed:
 *                - onShortTouch
 *                - onLongTouch
 *                - onSwipeRight
 *                - onSwipeUp
 *                - onSwipeLeft
 *                - onSwipeDown
 * 
 * Caveats      This class replaces the touch functions in the Lovayn LGFX library.
 *              Therefore, the section for the touchscreen in the configuration 
 *              file lgfx_ESP32_2432S028.h must be commented out
 *              
 * Board        ESP32-2432S028R
 * 
 * Remarks      The orientation of the display depends on the direction in which the USB port 
 *              of the CYD is facing: LANDSCAPE_USB_RIGHT = 0, PORTRAIT_USB_UP   = 1
 *                                    LANDSCAPE_USB_LEFT  = 2, PORTRAIT_USB_DOWN = 3 
 *              and thats why the 1st parameter of the constructor is a reference to the display instance     
 * 
 * References   https://registry.platformio.org/libraries/nitek/XPT2046_Bitbang_Slim
 *              https://github.com/lovyan03/LovyanGFX
 */

#include "XPT2046_Bitbang.h"


XPT2046_Bitbang::XPT2046_Bitbang(LGFX &lcd, uint8_t mosiPin, uint8_t misoPin, uint8_t clkPin, uint8_t csPin) : 
                                 _lcd(lcd), _mosiPin(mosiPin), _misoPin(misoPin), _clkPin(clkPin), _csPin(csPin) 
{
    _cal = TouchCalibration{40, 40, 280, 200, 650, 1040, 3365, 3165}; // Values from my CYD W=320 x H=240, W is the larger dimension
    //                      x0  y0   x1   y1  vx0   vy0   vx1   vy1   // The reference points here are 40/40 and 280/200 and the 
                                                                      // corresponding values detected are 650/1040 and 3365/3165
}


/**
 * Initialize the pins and 
 * get the screen orientation
 */
void XPT2046_Bitbang::begin() 
{
    pinMode(_mosiPin, OUTPUT);
    pinMode(_misoPin, INPUT);
    pinMode(_clkPin, OUTPUT);
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    digitalWrite(_clkPin, LOW);
    _rotation = _lcd.getRotation();
}


/**
 * Set the calibration data 
 */
void XPT2046_Bitbang::setCalibration(TouchCalibration& tc) 
{
    _cal = tc;
}


/**
 * Write a command to the SPI
 */
void XPT2046_Bitbang::_writeSPI(byte command) {
    for(int i = 7; i >= 0; i--) {
        digitalWrite(_mosiPin, command & (1 << i));
        digitalWrite(_clkPin, LOW);
        delayMicroseconds(DELAY);
        digitalWrite(_clkPin, HIGH);
        delayMicroseconds(DELAY);
    }
    digitalWrite(_mosiPin, LOW);
    digitalWrite(_clkPin, LOW);
}


/**
 * Read data from the SPI
 */
uint16_t XPT2046_Bitbang::_readSPI(byte command) {
    _writeSPI(command);

    uint16_t result = 0;

    for(int i = 15; i >= 0; i--) {
        digitalWrite(_clkPin, HIGH);
        delayMicroseconds(DELAY);
        digitalWrite(_clkPin, LOW);
        delayMicroseconds(DELAY);
        result |= (digitalRead(_misoPin) << i);
    }

    return result >> 4;
}


/**
 * Determines the coordinates of the touched point
 * and returns true if the pressure was strong enough.
 * The coordinates are returned in the reference variable tp.
 */
 bool XPT2046_Bitbang::getTouch(TouchPoint& tp) 
 {
    digitalWrite(_csPin, LOW);
    tp.zValue = _readSPI(CMD_READ_Z1) + 4095 - _readSPI(CMD_READ_Z2);

    if(tp.zValue < 100) { return false; }

    tp.xValue = _readSPI(CMD_READ_X);
    tp.yValue = _readSPI(CMD_READ_Y & ~((byte)1));
    digitalWrite(_csPin, HIGH);

    int x = map((int)tp.xValue, (int)_cal.xMinValue, (int)_cal.xMaxValue, _cal.xMinScreen, _cal.xMaxScreen); 
    int y = map((int)tp.yValue, (int)_cal.yMinValue, (int)_cal.yMaxValue, _cal.yMinScreen, _cal.yMaxScreen);

    // Limit the coordinates to the screen dimensions
    x = x < 0 ? 0 : x;
    x = x > TFT_WIDTH ? TFT_WIDTH : x;
    y = y < 0 ? 0 : y;
    y = y > TFT_HEIGHT ? TFT_HEIGHT : y;

    // Calculates the coordinates taking into account the screen  
    // orientation. The origin is always the top left corner.   
    switch (_rotation)
    {
        case 0: 
            // LANDSCAPE_USB_RIGHT
            tp.x = (uint16_t)x;
            tp.y = (uint16_t)y;
        break;

        case 1:
            // PORTRAIT_USB_UP
            tp.x = (uint16_t)y;
            tp.y = (uint16_t)(TFT_WIDTH - x);
        break;

        case 2:
             // LANDSCAPE_USB_LEFT
            tp.x = (uint16_t)(TFT_WIDTH - x);
            tp.y = (uint16_t)(TFT_HEIGHT - y);
        break;

        case 3:
            // PORTRAIT_USB_DOWN
            tp.x = (uint16_t)(TFT_HEIGHT - y);
            tp.y = (uint16_t)x;
        break;
    }

    //log_i("rot = %d, x = %d, y = %d, xValue = %d, yValue = %d", _rotation, tp.x, tp.y, tp.xValue, tp.yValue);
    return true;
}


/**
 * Returns the coordinates into x and y
 */
 bool XPT2046_Bitbang::getTouch(int& x, int& y) 
 {
    bool res = false;
    TouchPoint tp;
    res = getTouch(tp);
    x = tp.x;
    y = tp.y;
    return res;
 }


/**
 * Detects a touch without storing the coordinates
 */
  bool XPT2046_Bitbang::getTouch() 
  {
    TouchPoint tp = {0, 0, 0, 0, 0};
    return getTouch(tp);
  }


/**
 * Determines whether the touched point lies within the 
 * tolerance rectangle around the target point x0,y0
 */
  bool XPT2046_Bitbang::touchedAt(int x, int y, int x0, int y0, int dx, int dy)
  {
    return (x > x0-dx && x < x0+dx && y > y0-dy && y < y0+dy);
  }

/**                                   
 *                                 up
 *  Determines the swipe          \   /
 *  direction relative to     left  o  right
 *  the starting point            /   \
 *                                 down 
 */
  int XPT2046_Bitbang::_getSwipeDir(TouchPoint tpPenDown, TouchPoint tpPenUp)
  {
    // -1 = not swiped, 1 = right, 2 = up, 3 = left, 4 = down;
    float dx = tpPenUp.x - tpPenDown.x;
    float dy = tpPenUp.y - tpPenDown.y;
    int result;
    if (abs(dx) < _minSwipeDiff && abs(dy) < _minSwipeDiff) result = 0;

    else if (((dx > 0 && dy > 0) || (dx > 0 && dy < 0)) && fabs(dx/dy) > 1)
      result = 1;

    else if (((dx > 0 && dy < 0) || (dx < 0 && dy < 0)) && fabs(dx/dy) < 1)
      result = 2;

    else if (((dx < 0 && dy < 0) || (dx < 0 && dy > 0)) && fabs(dx/dy) > 1)
      result = 3;

    else if (((dx < 0 && dy > 0) || (dx > 0 && dy > 0)) && fabs(dx/dy) < 1)
      result = 4;

    else result = -1;
    //log_i("swipe result = %d", result);
    return result;
  }

  void XPT2046_Bitbang::loop()
  {
    if (getTouch(_tp)) 
    { // pen is down
      if (_msPenDown == 0) 
      {
      _msPenDown = millis(); // save time and position as soon as the pen goes down
      _tpPenDown = _tp;
      }
      else
      {
      _msPenUp = millis(); // save time and position as long as the pen is touching, 
      _tpPenUp = _tp;      // so the last time and position are saved when the pen moves upwards
      } 
    } 
    else 
    { // pen is up
      if (_msPenUp > 0)
      {
        _msTouchDuration = _msPenUp - _msPenDown;
        _swipeDir = _getSwipeDir(_tpPenDown, _tpPenUp);
        //log_i("touchDuration = %d",_msTouchDuration);
        if (_msTouchDuration > _msLongTouchMinDuration)
        {
          switch(_swipeDir)
          {
            case 0:
              if (_onLongTouch) _onLongTouch(_tpPenUp.x, _tpPenUp.y);
            break;

            case 1:
              if (_onSwipeRight) _onSwipeRight(_tpPenUp.x, _tpPenUp.y);
            break;
            
            case 2:
              if (_onSwipeUp) _onSwipeUp(_tpPenUp.x, _tpPenUp.y);
            break;

            case 3:
              if (_onSwipeLeft) _onSwipeLeft(_tpPenUp.x, _tpPenUp.y);
            break;

            case 4:
              if (_onSwipeDown) _onSwipeDown(_tpPenUp.x, _tpPenUp.y);
            break;
          }
          
        }
        else if (_msTouchDuration > _msShortTouchMinDuration)
        {
          if (_onShortTouch) _onShortTouch(_tpPenUp.x, _tpPenUp.y);
        }
        _msPenDown = 0;
        _msPenUp = 0;
        _tpPenDown.x = 0;
        _tpPenDown.y = 0;
        _tpPenUp.x = 0;
        _tpPenUp.y = 0;

      }
    }
    delay(10);
  }


void XPT2046_Bitbang::addShortTouchCb(Callback cb)
{ _onShortTouch = cb; }

void XPT2046_Bitbang::addLongTouchCb(Callback cb)
{ _onLongTouch = cb; }

void XPT2046_Bitbang::addSwipeLeftCb(Callback cb)
{ _onSwipeLeft = cb; }

void XPT2046_Bitbang::addSwipeRightCb(Callback cb)
{ _onSwipeRight = cb; }

void XPT2046_Bitbang::addSwipeUpCb(Callback cb)
{ _onSwipeUp = cb; }

void XPT2046_Bitbang::addSwipeDownCb(Callback cb)
{ _onSwipeDown = cb; }