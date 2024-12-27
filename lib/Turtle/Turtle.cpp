/**
 * Class        Turtle
 *
 * Author       2024-03-03 Charles Geiser (https://www.dodeka.ch)
 *
 * Purpose      Implementation of some Turtle Graphics primitives
 * 
 * Board        ESP32 DoIt DevKit V1
 * 
 * Remarks      
 * 
 * References   
 */

#include "Turtle.h"

/**
 * Clear the screen with the set color
 */
void Turtle::clear()
{
    _lcd.fillScreen(_screenColor);
}


/**
 * Set turtles home position to x,y,heading 
 */
void Turtle::home(int x, int y, float heading)
{
    _x = x;
    _y = y;
    _heading = heading;
}


/**
 * Move turtle forward one step
 */
void Turtle::forward(float step)
{
    float h = _heading * RAD;
    int x = _x;
    int y = _y;
    _x += round(step * cos(h));
    _y += round(step * sin(h));
    //log_i("_x=%3d, _y=%3d\n", _x, _y);
    if (_penDown) _lcd.drawLine(x, y, _x, _y, _penColor);
}


/**
 * Move turtle backwars one step
 */
void Turtle::backward(int step)
{
    forward(-step);
}


/**
 * Rotate the turtle clockwise by the specified angle 
 */
void Turtle::right(float angle) // degrees)
{
    _heading += angle;
    if (_heading > 0.0) 
        while (_heading >= 360.0) { _heading -= 360.0; }
    else
        while (_heading < 0.0) { _heading += 360.0; }
    //log_i("heading=%4.1f", _heading);
}


/**
 * Rotate the turtle counterclockwise by the specified angle
 */
void Turtle::left(float angle)
{
    right(-angle);
}


/**
 * Put the pen down
 */
void Turtle::penDown()
{
    _penDown = true;
}


/**
 * Lift the pen
 */
void Turtle::penUp()
{
    _penDown = false;
}


/**
 * Bresenham's line algorithm
 */
void Turtle::bresenham(int x0, int y0, int x1, int y1)
{
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    while (true) 
    {
        _lcd.drawPixel(x0, y0, _penColor);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}


/**
 * Move the turtle to position x,y
 */
void Turtle::moveTo(int x, int y)
{
    if (_penDown)
    {
        //Bresenham(_x, _y, x, y);
        _lcd.drawLine(_x, _y, x, y, _penColor);
        _x = x; _y = y;          
    }
    else
    {
        _x = x;
        _y = y;
    }
}


/**
 * Set the pens color
 */
void Turtle::penColor(int color)
{
    _penColor = color;
}


/**
 * Set the screens color
 */
void Turtle::screenColor(int color)
{
    _screenColor = color;
    _lcd.fillScreen(_screenColor);
}


/**
 * show position and heading of the turtle
 */
void Turtle::showValues()
{
    log_i("x=%3d, y=%3d, heading=%3d", _x, _y, _heading);
}
