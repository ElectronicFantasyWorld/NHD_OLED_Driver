/*
 * Newhaven Display Slim OLED Driver
 * ---------------------------------
 * 
 * Base SPI Code by Saurabh Baxi, Applications Engineer at Newhaven Display* *   International, Inc.
 * Modifications and extensions by Tom Honaker.
 * 
 * 
 * 
 * This is a comprehensive driver for running Newhaven's slim OLED character 
 * displays from just about any Arduino with enough free pins to connect to one. 
 * The interface being used is SPI, but it's bit-banged on four Arduino pins so
 * there's no requirement for the Arduino to have hardware SPI support. Check the
 * datasheet for your display to wire it to the Arduino in SPI mode.
 * 
 * In order to save on precious memory, the library uses very little global RAM.
 * Variables are mainly declared inside functions so memory use is far more local
 * than global.
 * 
 * The driver was designed and tested against a Newhaven NHD-0420CW-AB3 slim
 * OLED display module connected to an Arduino Mega 2560 v3, and an Arduino Pro
 * Mini 8MHz/3.3v was also successfully implemented. The library should work with
 * few changes with any Newhaven slim OLED module and any Arduino, although
 * some unused functions might need to be cut if memory space is at a premium.
 * 
 * 
 * 
 * IMPORTANT NOTE ON STRINGS/CHARS
 * 
 * C/C++ strings end in nulls (0x00), but to a LCD or OLED display, character 
 * code 0x00 is the first of eight custom character pattern slots in the display's 
 * character generator ROM. When sending text to the display, remember to NOT send 
 * a 0x00 unless displaying custom character #1 is the desired result!
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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */



// The all-important #include...
#include <NHD_OLED_Driver.h>



// PROGMEM String Table Example
//
// YES, you CAN build string tables and store them in program memory instead of eating up SRAM!
// And, as an added plus, it's actually pretty easy to do.
//
// To start, define the strings and PROGMEM them, which will tell the compiler to store them
// in program memory and not copy them to SRAM at program start. This can save multiple
// kilobytes of SRAM on text-heavy projects.
const char strfuse00[] PROGMEM = "::bang.::           ";
const char strfuse01[] PROGMEM = "*                   ";
const char strfuse02[] PROGMEM = "-*                  ";
const char strfuse03[] PROGMEM = "--*                 ";
const char strfuse04[] PROGMEM = "---*                ";
const char strfuse05[] PROGMEM = "----*               ";
const char strfuse06[] PROGMEM = "-----*              ";
const char strfuse07[] PROGMEM = "------*             ";
const char strfuse08[] PROGMEM = "-------*            ";
const char strfuse09[] PROGMEM = "--------*           ";
const char strfuse10[] PROGMEM = "---------*          ";
const char strfuse11[] PROGMEM = "----------*         ";
const char strfuse12[] PROGMEM = "-----------*        ";
const char strfuse13[] PROGMEM = "------------*       ";
const char strfuse14[] PROGMEM = "-------------*      ";
const char strfuse15[] PROGMEM = "--------------*     ";
const char strfuse16[] PROGMEM = "---------------*    ";
const char strfuse17[] PROGMEM = "----------------*   ";
const char strfuse18[] PROGMEM = "-----------------*  ";
const char strfuse19[] PROGMEM = "------------------* ";
const char strfuse20[] PROGMEM = "-------------------*";

// And now we wrap the strings above into an indexable table...
const char* const fuse_table[] PROGMEM = {
  strfuse00,
  strfuse01,
  strfuse02,
  strfuse03,
  strfuse04,
  strfuse05,
  strfuse06,
  strfuse07,
  strfuse08,
  strfuse09,
  strfuse10,
  strfuse11,
  strfuse12,
  strfuse13,
  strfuse14,
  strfuse15,
  strfuse16,
  strfuse17,
  strfuse18,
  strfuse19,  
  strfuse20
};
// We can now reach into program memory with a special function, strcpy_P, and
// fetch any one string by sending a pointer to the driver's progmem-retrieval
// functions.

// Let's set up some additional text...
const char strt00[] PROGMEM = "PROGMEM Strings!";
const char strt01[] PROGMEM = "Save that RAM!";
const char strt02[] PROGMEM = "And do simple";
const char strt03[] PROGMEM = "animations";
const char strt04[] PROGMEM = "as well.";
const char* const string_table[] PROGMEM = {
  strt00,
  strt01,
  strt02,
  strt03,
  strt04
};



// ** SETUP & LOOP ROUTINES FOLLOW **



// Create a global display driver instance. 
NHD_OLED oled;



void setup() {
  // One-shot setup command!
  oled.begin(52,51,53,4,20);
  // Parameters:
  //    pinSCLK: pin to use for SPI clock - connect to display's SCLK pin.
  //    pinSDIN: pin to use for SPI MOSI - connect to display's SDI pin.
  //    pinC_S: pin to use for SPI chip select - connect to display's /CS pin.
  //    rows: number of rows/lines on the display.
  //    columns: number of columns/characters per line on the display.

/* 
 * Don't want to use a single command? begin() calls these next three lines...
 * 

  // Set up the display's size in terms of row/column count.  
  oled.setupDisplaySize(4,20);

  // Set up the display's pins. Pass the three pins that are connected to the display as
  // SPI clock (SCLK on the display datasheet's pinout), SPI MOSI (SDI), and SPI chip-select 
  // (/CS), in that order.
  oled.setupPins(52,51,53);

  // Initialize the display.
  oled.setupInit();
*/
}


void loop() {
  
  // Create a buffer large enough to hold a full row's worth of characters, and let's not
  // forget the trailing null!
  char buf[oled.DISP_COLUMNS + 1];
  
  // Clear the display.
  oled.textClear();

  // Set the cursor to home (top-left corner).
  oled.cursorHome();

  // Let's start with a simple animation: the textSweep function.
  switch(oled.DISP_COLUMNS){
    case 8:
      oled.textSweep("Newhaven", 8, 0, 0x15, 0x14, 15);
      break;
    case 12: 
      oled.textSweep("NewhavenOLED", 12, 0, 0x15, 0x14, 15);
      break;
    case 16:
      oled.textSweep("Newhaven  OLED", 14, 0, 0x15, 0x14, 15);
      break;
    case 20:
      oled.textSweep("Newhaven OLED Test", 18, 0, 0x15, 0x14, 15);
      break;
    default:
      oled.textSweep("NHD!", 4, 0, 0x15, 0x14, 15);
      break;
  }

  oled.textSweep("        ", 8, 1, 0x15, 0x14, 15);

  if ((oled.DISP_COLUMNS >= 16) && (oled.DISP_ROWS == 4)){
    oled.textSweep("16x2 to 20x4", 12, 2, 0x15, 0x14, 15);
    oled.textSweep("OLEDs!", 6, 3, 0x15, 0x14, 15);
  }  

  delay(3000);

  if (oled.DISP_COLUMNS > 8)
  {
    // Demonstrate some text positioning
    //
    // Note that we're defining the char-array size so that we don't end up with
    // a 0x00 as the last character, which is what happens when you allow a char-array
    // to size itself by the string - C/C++ strings _always_ end with a null!
    char mt[7] = "PrintAt";
    char lt[6] = "L-Just";
    char ct[6] = "Center";    
    char rt[6] = "R-Just";
    byte i, ii;
    
    for (i = 0; i < 3; i++){
      // Clear the second row/line (these are zero-indexed, so the topmost line is 0).
      oled.textClearRow(1);

      // Move the cursor to the start of the row/line.
      oled.cursorMoveToRow(1);

      // Print some text.
      oled.print(lt, sizeof(lt));

      // Wait half a second.
      delay(500);

      // Print some centered text. Then, wait another half-second.
      oled.textPrintCentered(ct, sizeof(ct), 1);
      delay(500);

      // Print some right-justified text. And wait again.
      oled.textPrintRightJustified(rt, sizeof(rt), 1);
      delay(500);

      // Back to centered.
      oled.textPrintCentered(ct, sizeof(ct), 1);
      delay(500);

      // And back to left.
      oled.textClearRow(1);
      oled.cursorMoveToRow(1);
      oled.print(lt, sizeof(lt));
      delay(500);

      // Centered again.
      oled.textPrintCentered(ct, sizeof(ct), 1);
      delay(500);

      //And end on the right side.
      oled.textPrintRightJustified(rt, sizeof(rt), 1);
      delay(500);
      
      for (ii = 0; ii < oled.DISP_COLUMNS - sizeof(mt); ii++){
        // Move text to the right via position control and not via scrolling.
        oled.textClearRow(1);
        oled.print(mt, sizeof(mt), 1, ii);
  
        delay(150);
      }    
  
      for (ii = oled.DISP_COLUMNS - sizeof(mt); ii > 0; ii--){
        // Move text back to the left, again without using scrolling.
        oled.textClearRow(1);
        oled.print(mt, sizeof(mt), 1, ii);
  
        delay(150);
      } 

      for (ii = 0; ii < 20; ii++){
        // Jump around randomly for a bit.
        oled.textClearRow(1);
        oled.print(mt, sizeof(mt), 1, random(0, oled.DISP_COLUMNS - sizeof(mt)));
        delay(50);
      }      
    }


    // Demonstrate retrieving and showing text from PROGMEM-stored string tables

    if (oled.DISP_COLUMNS >= 16)
    {
      // Scroll through some demo text.
      oled.textClear();
      for (ii = 0; ii < 5; ii++)
      {
        if (oled.DISP_ROWS < 3)
        {
          oled.textPrintTextFromProgmemCentered(&(string_table[ii]), 0);
        } else {
          oled.textPrintTextFromProgmemCentered(&(string_table[ii]), 1);
        }
        
        delay(2000);
      }
      
      oled.textClear();

      // Light the fuse... (Simple animation/progress-bar text)
      for (ii = oled.DISP_COLUMNS; ii > 0; ii--)
      {
        if (oled.DISP_ROWS < 3)
        {
          oled.textPrintTextFromProgmemCentered(&(fuse_table[ii]), 0);
        } else {
          oled.textPrintTextFromProgmemCentered(&(fuse_table[ii]), 1);
        }
        delay(250);
      }
      // Bang!
      if (oled.DISP_ROWS < 3)
      {
        oled.textPrintTextFromProgmemCentered(&(fuse_table[0]), 0);
      } else {
        oled.textPrintTextFromProgmemCentered(&(fuse_table[0]), 1);
      }
      delay(2500);
    }
  }

  // Shut down the display.
  oled.textClear();
  oled.displayOff();



  // Wait 10 seconds, then turn the display back on and do it all again.
  delay(10000);
  oled.displayOn();
}

