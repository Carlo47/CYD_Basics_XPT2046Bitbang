/**
 * Header       Turtle.h
 * Author       2024-03-10 Charles Geiser (https://www.dodeka.ch)
 * 
 * Purpose      Declaration of the class Turtle
 *              Implements some Turtle Graphic primitives
 * 
 *              Turtle origin (0,0) is at upper left corne of the lcd
 *              Turtle heading 0.0 degrees is to the right in positive x direction 
 * 
 * Constructor
 * arguments    &lcd        reference to LGFX display
 *              x0          turtles x position
 *              y0          turtles y position
 *              heading     turtles heading
 *              penColor    color of the turtles trail
 */

#pragma once
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "lgfx_ESP32_2432S028.h"

class Turtle
{
    static constexpr float RAD = 3.14159265359 / 180.0;

    public:
        Turtle(LGFX &lcd, int x0, int y0, float heading, int penColor=TFT_WHITE) : 
            _lcd(lcd), _x(x0), _y(y0), _heading(heading), _penColor(penColor)
        { lcd.fillScreen(_screenColor); }

        LGFX  &_lcd;
        void clear();
        void forward(float step);
        void backward(int step);
        void right(float angle);
        void left(float angle);
        void home(int x, int y, float heading);
        void moveTo(int x, int y);
        void penDown();
        void penUp();
        void penColor(int color);
        void screenColor(int color);
        void showValues();
        void bresenham(int x, int y, int x1, int y1);

    private:
        float _heading; // heading in degrees
        int   _x;       // x position on screen
        int   _y;       // y position on screen 
        int   _penColor;
        int   _screenColor = TFT_BLACK;
        bool  _penDown = true;  
};
