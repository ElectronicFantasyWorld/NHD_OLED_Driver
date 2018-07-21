/*
 * Newhaven Display Slim OLED Driver
 * ---------------------------------
 * 
 * Base SPI Code by Saurabh Baxi, Applications Engineer at Newhaven Display 
 *   International, Inc.
 * Modifications and extensions by Tom Honaker.
 * 
 * 
 * 
 * This is a comprehensive driver for running Newhaven's slim OLED character 
 * displays from just about any Arduino with enough free pins to connect to 
 * one. The interface being used is SPI, but it's bit-banged on four Arduino 
 * pins so there's no requirement for the Arduino to have hardware SPI support.
 * Check the datasheet for your display to wire it to the Arduino in SPI mode.
 * 
 * In order to save on precious memory, the library uses very little global 
 * RAM. Variables are mainly declared inside functions so memory use is far 
 * more local than global.
 * 
 * The driver was designed and tested against a Newhaven NHD-0420CW-AB3 slim
 * OLED display module connected to an Arduino Mega 2560 v3, and an Arduino
 * Pro Mini 8MHz/3.3v was also successfully implemented. The library should
 * work with few changes with any Newhaven slim OLED module and any Arduino,
 * although some unused functions might need to be cut if memory space is at
 * a premium.
 * 
 * 
 * 
 * IMPORTANT NOTE ON STRINGS/CHARS
 * 
 * C/C++ strings end in nulls (0x00), but to a LCD or OLED display, character 
 * code 0x00 is the first of eight custom character pattern slots in the 
 * display's character generator ROM. When sending text to the display,
 * remember to NOT send a 0x00 unless displaying custom character #1 is the 
 * desired result!
 * 
 * 
 * 
 * Software License Agreement (BSD License)
 * 
 * Copyright (c) 2015-2017 by Newhaven Display International, Inc.
 * Copyright (c) 2017-2018 by Tom Honaker.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */



#ifndef NHD_OLED_DRIVER_H
#define NHD_OLED_DRIVER_H

#include "Arduino.h"

class NHD_OLED
{
  public:
    // Here's the good stuff...
    //void begin(byte pinSCLK, byte pinSDIN, byte pinC_S, byte rows = 2, 
    //           byte columns = 16);
    void begin(byte pinSCLK, byte pinSDIN, byte rows = 2, 
               byte columns = 16);
    void sendCommand(byte command);
    void sendData(byte data);
    void setupDisplaySize(byte rows = 2, byte columns = 16);
    //void setupPins(byte pinSCLK, byte pinSDIN, byte pinC_S);
    void setupPins(byte pinSCLK, byte pinSDIN);
    void setupInit();
    void displayControl(byte display, byte cursor, byte block);
    void displayOn();
    void displayOff();
    void textClear();
    void cursorHome();
    void cursorMoveToRow(byte rowNumber);
    void textClearRow(byte rowNumber);
    void shift(byte dc, byte rl);
    void cursorPos(byte row, byte column);
    void print(char *text, byte len);
    void print(char text);
    void print(char *text, byte len, byte r, byte c);
    void print(char text, byte r, byte c);
    void textPrintCentered(char *text, byte length, byte row);
    void textPrintRightJustified(char *text, byte length, byte row);
    void textPrintTextFromProgmem(int ptrStringTableEntry);    
    void textPrintTextFromProgmemCentered(int ptrStringTableEntry, byte row);
    void textSweep(char *text, byte length, byte row, char leftSweepChar, 
                   char rightSweepChar, unsigned int timeDelay);

    // LiquidCrystal Library Compatibility
    void clear();
    void home();
    void setCursor(byte col, byte row);
    void write(byte data);
    void cursor();
    void noCursor();
    void blink();
    void noBlink();
    void display();
    void noDisplay();
    void scrollDisplayLeft();
    void scrollDisplayRight();
    void autoscroll();
    void noAutoscroll();
    void leftToRight();
    void rightToLeft();
    void createChar(byte num, char* data);

    // Pin Designations
    byte SCLK = 0;
    byte SDIN = 1;
    //byte C_S =  2;

    // Display Geometry
    byte DISP_ROWS = 2;
    byte DISP_COLUMNS = 16;
  private:
    // SPI Bit-Bang - This procedure shouldn't be called directly.
    void SPIBitBang(byte data, byte isCommand);    
};

#endif



/*
 * End of file!
 */