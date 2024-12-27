## A further step with the ESP32-2432S028R called CYD
My earlier experiments with the CYD were unsatisfactory in two respects.
 - only 2 of the 3 SPI devices could be used simultaneously:
   -  Display and Touchpad
   -  Display and SD card 
   - Touchpad and SD card
 -  The colors of a screenshot saved on the SD card had to be swapped
    afterwards with an image editing program, e.g. XnView.

### Solving the Touchpad Problem
I was advised that a software SPI should be used for the touchpad and I 
found the touchpad library 
[nitek/XPT2046_Bitbang_Slim](https://registry.platformio.org/libraries/nitek/XPT2046_Bitbang_Slim) 
by Claus Näveke. I extended this class so that it now also includes the events 
  - shortTouch
  - longTouch
  - swipes right, up, right, down
  - and a loop() function which can be called in the main loop. 

 To react to events in the loop function, 6 callback functions can be installed:
  - onShortTouch
  - onLongTouch
  - onSwipeRight
  - onSwipeUp
  - onSwipeLeft
  - onSwipeDown
  
  **Caution**: If you use this class, the touchpad section in the configuration
  file lgfx_ESP32_2432S028.h must be commented out.


### Solving the color Problem
I was able to solve the color problem of saved screenshots by swapping the colors in the 
[saveBMPtoSD](https://github.com/lovyan03/LovyanGFX/tree/master/examples/Standard/SaveBMP) 
function of the LovyanGFX example before saving them.

