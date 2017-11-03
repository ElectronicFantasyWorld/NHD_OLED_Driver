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



#include "Arduino.h"
#include "NHD_OLED_Driver.h"



// NHD_OLED::begin
//
// Performs all required initialization steps in a single command.
//
// Parameters:
//    pinSCLK: pin to use for SPI clock - connect to display's SCLK pin.
//    pinSDIN: pin to use for SPI MOSI - connect to display's SDI pin.
//    pinC_S: pin to use for SPI chip select - connect to display's /CS pin.
//    rows: number of rows/lines on the display.
//    columns: number of columns/characters per line on the display.
//
void NHD_OLED::begin(byte pinSCLK, byte pinSDIN, byte pinC_S, byte rows = 2, 
                     byte columns = 16) {
    setupPins(pinSCLK, pinSDIN, pinC_S);
    setupDisplaySize(rows, columns);
    setupInit();
}
       
// NHD_OLED::SPIBitBang
//
// This function performs a simple send-only SPI connection using any three 
// pins on the Arduino. These pins don't have to be hardware SPI pins, but do
// have to be available and not used by other hardware like UARTs or ADC
// inputs. Unless helper functions have been removed to reduce memory usage,
// there's no real reason to call this directly.
//
// Parameters:
//   data: byte to send to display
//   isCommand: command/data flag, where 0 = data and <>0 = command
//
void NHD_OLED::SPIBitBang(byte data, byte isCommand) {
  byte i, cb; 

  // If var "isCommand" is zero, we're sending a data byte.
  // If it's NON-zero, we're sending a command byte.
  if (isCommand == 0)
    cb = 0xFA; // Var "data" is just that, data.
  else
    cb = 0xF8; // Var "data" is a command/control byte.

  // Pull the chip-select line low.
  digitalWrite(C_S, LOW);

  // Send the command-or-data type specifier...
  for(i=0;i<8;i++)
  {
    digitalWrite(SCLK, LOW);
    digitalWrite(SDIN, ((cb & 0x80) >> 7));
    cb = cb << 1;
    digitalWrite(SCLK, HIGH);
  }

  // Then send the lowest 4 bits of the data byte, in little-endian order...
  for(i=0;i<4;i++)
  {
    digitalWrite(SCLK, LOW);
    digitalWrite(SDIN, (data & 0x01));    
    data = data >> 1;
    digitalWrite(SCLK, HIGH);
  }

  // Then send four zero bits...
  for(i=0;i<4;i++)
  {
    digitalWrite(SCLK, LOW);
    digitalWrite(SDIN, LOW);
    digitalWrite(SCLK, HIGH);
  }

  // Then send the highest 4 bits of the data byte, in little-endian order...
  for(i=0;i<4;i++)
  {
    digitalWrite(SCLK, LOW);
    digitalWrite(SDIN, (data & 0x01)); 
    data = data >> 1;
    digitalWrite(SCLK, HIGH);
  }

  // And wrap up the send with four zero bits...
  for(i=0;i<4;i++)
  {
    digitalWrite(SCLK, LOW);
    digitalWrite(SDIN, LOW);
    digitalWrite(SCLK, HIGH);
  }  
}


// NHD_OLED::sendCommand
//
// Provides an alternative means to send a command byte to the display.
//
// Parameters:
//   command: command byte to send.
//
void NHD_OLED::sendCommand(byte command) {
  SPIBitBang(command, 1);
}


// NHD_OLED::sendData
//
// Provides an alternative means to send a data byte to the display.
//
// Parameters:
//   data: command byte to send.
//
void NHD_OLED::sendData(byte data) {
  SPIBitBang(data, 0);
}


// NHD_OLED::setupDisplaySize
//
// Instructs this driver on the geometry of the display, in row and column
// count. For example, a 16-character x 2-line display would be a rows value
// of 2 and a columns value of 16.
//
// Parameters:
//    rows: number of rows/lines on the display.
//    columns: number of columns/characters per line on the display.
//
void NHD_OLED::setupDisplaySize(byte rows = 2, byte columns = 16) {
  DISP_ROWS = rows;
  DISP_COLUMNS = columns;
}


// NHD_OLED::setupPins
//
// Configures pins for driving the display. Note that these don't have to be
// hardware SPI - any three available pins should work as long as they're not
// doing something else.
//
// Parameters:
//    pinSCLK: pin to use for SPI clock - connect to display's SCLK pin.
//    pinSDIN: pin to use for SPI MOSI - connect to display's SDI pin.
//    pinC_S: pin to use for SPI chip select - connect to display's /CS pin.
//
void NHD_OLED::setupPins(byte pinSCLK, byte pinSDIN, byte pinC_S) {
  SCLK = pinSCLK;
  SDIN = pinSDIN;
  C_S = pinC_S;
    
  pinMode(SCLK, OUTPUT);
  pinMode(SDIN, OUTPUT);
  pinMode(C_S,  OUTPUT);
  
  digitalWrite(SCLK, HIGH);
  digitalWrite(SDIN, HIGH);
  
  delay(30);
}


// NHD_OLED::setupInit
//
// Initializes and configures the display. Note that the command set provided
// below is for Newhaven's slim OLED line, and individual products may require
// different settings.
//
void NHD_OLED::setupInit() {
    // Internal voltage regulator configuration
    sendCommand(0x2A);     // Function set select > extended command set enable (RE = 1)
    sendCommand(0x71);     // Internal Vdd regualtor control (function selection A) - command
    sendData(0x5C);        // Internal Vdd regualtor control (function selection A) - value

    // Display off
    sendCommand(0x28);     // Function set select > fundamental (default) command set (RE = 0)
    sendCommand(0x08);     // Display off, cursor off, blink off

    // Timing configuration
    sendCommand(0x2A);     // Function set select > extended command set enable (RE = 1)
    sendCommand(0x79);     // Function set select > OLED command set enable (SD = 1)
    sendCommand(0xD5);     // Set display clock divide ratio/oscillator frequency - command
    sendCommand(0x70);     // Set display clock divide ratio/oscillator frequency - value
    sendCommand(0x78);     // Function set > OLED command set disable (SD = 0)

    // Row count configuration
    sendCommand(0x28);     // Function set select > fundamental (default) command set (RE = 0)
    if (DISP_ROWS < 3){
      sendCommand(0x08);   // Set row/line count (extended function set) - 1-/2-lines
      if (DISP_ROWS == 1)
        sendCommand(0x20); // Set row/line count (extended function set) - 1 line
      else
        sendCommand(0x28); // Set row/line count (extended function set) - 2 lines
    }
    else{
      sendCommand(0x09);   // Set row/line count (extended function set) - 3-/4-lines
      if (DISP_ROWS == 3)
        sendCommand(0x20); // Set row/line count (extended function set) - 3 lines
      else
        sendCommand(0x28); // Set row/line count (extended function set) - 4 lines
    }

    // CGROM select configuration
    sendCommand(0x2A);     // Function set select > extended command set enable (RE = 1)    
    sendCommand(0x72);     // CGROM selection (function selection B) - command
    sendData(0x00);        // CGROM selection (function selection B) - value

    // Segment control configuration
    sendCommand(0x06);     // Direction control - Incrementing-COM/decrementing-SEG
    sendCommand(0x20);     // Display order - Normal (non-mirrored)

    // Internal hardware configuration
    sendCommand(0x79);     // Function set select > OLED command set enable (SD = 1)
    sendCommand(0xDA);     // SEG pins hardware configuration - command
    sendCommand(0x10);     // SEG pins hardware configuration - value - NOTE: Some displays require 0x00 here.
    sendCommand(0xDC);     // VSL & GPIO control (function selection C) - command
    sendCommand(0x00);     // VSL & GPIO control (function selection C) - value
    sendCommand(0x81);     // Contrast control - command
    sendCommand(0x7F);     // Contrast control - value
    sendCommand(0xD9);     // Phase length - command
    sendCommand(0xF1);     // Phase length - value
    sendCommand(0xDB);     // VCOMH deselect level - command
    sendCommand(0x40);     // VCOMH deselect level - value
    sendCommand(0x78);     // Function set select > OLED command set disable (SD = 0)

    
    // Wrapping up and switching on
    sendCommand(0x28);     // Function set select > fundamental (default) command set (RE = 0)
    sendCommand(0x01);     // Clear display
    sendCommand(0x80);     // Set DDRAM address to 0x00 (home on topmost row/line)
    sendCommand(0x0C);     // Display ON
    
    delay(100);
}


// NHD_OLED::displayControl
//
// En/disables the entire display, the cursor, and whether the cursor is a
// flashing block.
//
// Parameters:
//   display: display on/off - non-zero for "on."
//   cursor: cursor on/off - non-zero for "on."
//   block: cursor block/line - non-zero for block.
//
void NHD_OLED::displayControl(byte display, byte cursor, byte block) {
  byte value = 0x08;

  if (display != 0)
    value = value + 4;

  if (cursor != 0)
    value = value + 2;

  if (block != 0)
    value = value + 1;

  sendCommand(value);
}


// NHD_OLED::displayOn
//
// Switches the entire display on. Cursor and blink are disabled - to enable
// these, use the displayControl() function.
//
void NHD_OLED::displayOn() {
  sendCommand(0x0C);
  delay(10);
}


//NHD_OLED::dispOff
//
// Switches the entire display off.
//
void NHD_OLED::displayOff() {
  sendCommand(0x08);
  delay(10);
}


// NHD_OLED::textClear
//
// Sends a "clear" command to the display.
//
void NHD_OLED::textClear() {
  sendCommand(0x01);
  delay(10);
}


// NHD_OLED::cursorHome
//
// Sends a "home" command to the display, which moves the cursor without
// removing text from the display.
//
void NHD_OLED::cursorHome() {
  sendCommand(0x02);
  delay(10);
}


// NHD_OLED::cursorMoveToRow
//
// Move cursor to start of selected line.
//
// Parameters:
//   rowNumber: row/line number to move to (zero-indexed, where 0 is topmost).
//
void NHD_OLED::cursorMoveToRow(byte rowNumber) {
  byte row_command[4] = {0x80, 0xA0, 0xC0, 0xE0};

  sendCommand(row_command[rowNumber]);
  delay(10);
}


// NHD_OLED::textClearRow
//
// Clears a line on the display by writing spaces to the entire row/line.
//
// Parameters:
//   rowNumber: row/line number to clear (zero-indexed, where 0 is topmost).
//
void NHD_OLED::textClearRow(byte rowNumber) {
  char temp[DISP_COLUMNS];

  for (int i; i < DISP_COLUMNS; i++)
    temp[i] = 0x20;

  cursorMoveToRow(rowNumber);
  print(temp, DISP_COLUMNS);
}


// NHD_OLED::shift
//
// Shifts the cursor or the displayed text one position right or left.
//
// Parameters:
//   dc: display or cursor - non-zero for display.
//   rl: direction right/left - non-zero for right.
//
void NHD_OLED::shift(byte dc, byte rl) {
  byte value = 0x10;

  if (dc != 0)
    value = value + 8;

  if (rl != 0)
    value = value + 4;

  sendCommand(value);
}


// NHD_OLED::cursorPos
//
// Moves the cursor to the given column on the given row/line.
//
// Parameters:
//   row: row/line number (0-1/2/3).
//   column: column number (0-16/20).
//
void NHD_OLED::cursorPos(byte row, byte column) {
  byte row_command[4] = {0x80, 0xA0, 0xC0, 0xE0};

  if (row >= DISP_ROWS)
    row = DISP_ROWS - 1;
  if (column >= DISP_COLUMNS)
    column = DISP_COLUMNS - 1;

  sendCommand(row_command[row] + column);
}


// NHD_OLED::print
//
// Sends text to the display to be, well, displayed. Note that the text is
// printed from the current cursor position, so calling home() pos(),
// or cursorMoveToLine() functions first would probably be a good idea.
//
// Parameters:
//   text: text to display. This should be a full string if a length is
//         provided.
//   len: length of text to print, in characters.
//
void NHD_OLED::print(char *text, byte len) {
  for (byte i = 0; i < len; i++)
    sendData(text[i]);  
}


// NHD_OLED::print - OVERLOAD
// 
// This overload of NHD_OLED::print accepts a single character. No length
// parameter is needed. Again, printing happens where the cursor is
// positioned.
//
// Parameters:
//   text: text to display. This must be a single character.
//
void NHD_OLED::print(char text) {
  sendData(text);
}


// NHD_OLED::print - OVERLOAD
//
// This overload of NHD_OLED::print accepts row/line and column numbers (zero-
// indexed - 0, 0 is home) and moves the cursor to the given position before
// printing the supplied text.
//
// Parameters:
//   text: text to display. This should be a full string.
//   len: length of text to print, in characters.
//   r: row/line number (0-1/2/3).
//   c: column number (0-16/20).
//
// ADDITIONAL USAGE NOTES:
//
// To right-justify text, determine the length of the text, e.g., with 
// sizeof(text), and use (DISP-COLUMNS - length) as the column number.
// (Or, use the helper function print_r().)
//
// To center text, use ((DISP-COLUMNS - length) / 2) as the column number.
// (There's also a helper function for this as well: print_c().)
//
// In either of the above usage cases, clearing the line with clear()
// before printing text may be deisrable.
//
void NHD_OLED::print(char *text, byte len, byte r, byte c) {
  cursorPos(r, c);

  for (byte i = 0; i < len; i++)
    sendData(text[i]);
}


// NHD_OLED::print - OVERLOAD
// 
// This overload of NHD_OLED::print accepts row/line and column numbers (zero-
// indexed - 0, 0 is home) and moves the cursor to the given position before
// printing the supplied character. Only one character is printed - no length
// parameter is needed.
//
// Parameters:
//   text: text to display. This must be a single character.
//   r: row/line number (0-1/2/3).
//   c: column number (0-16/20).
//
void NHD_OLED::print(char text, byte r, byte c) {
  cursorPos(r, c);

  sendData(text);
}


// NHD_OLED::textPrintCentered
//
// Prints the supplied text, centered, on the selected row/line. The row/line
// is cleared before printing.
//
// Parameters:
//   text: text to display. This should be a full string.
//   length: length of text to print, in characters.
//   row: row/line number (0-1/2/3).
//
void NHD_OLED::textPrintCentered(char *text, byte length, byte row) {
  byte i;
  char line[DISP_COLUMNS];

  // Clear the text buffer.
  for (i = 0; i < DISP_COLUMNS; i++)
    line[i] = 0x20;

  // Copy text into buffer, centering it relative to the line's width.
  for (i = 0; i < length; i++)
    line[((DISP_COLUMNS - length) / 2) + i] = text[i]; 

  // Clear the line.
  textClearRow(row);

  // Move the cursor to the row/line.
  cursorMoveToRow(row);
  
  // Print the centered text.
  print(line, DISP_COLUMNS);
}


// NHD_OLED::textPrintRightJustified
//
// Prints the supplied text, right-justified, on the selected row/line. The
// row/line is cleared before printing.
//
// Parameters:
//   text: text to display. This should be a full string.
//   length: length of text to print, in characters.
//   row: row/line number (0-1/2/3).
//
void NHD_OLED::textPrintRightJustified(char *text, byte length, byte row) {
  byte i;
  char line[DISP_COLUMNS];

  // Clear the text buffer.
  for (i = 0; i < DISP_COLUMNS; i++)
    line[i] = 0x20;

  // Copy text into buffer, centering it relative to the line's width.
  for (i = 0; i < length; i++)
    line[(DISP_COLUMNS - length) + i] = text[i]; 

  // Clear the row/line.
  textClearRow(row);

  // Move the cursor to the row/line.
  cursorMoveToRow(row);

  // Print the centered text.
  print(line, DISP_COLUMNS);
}


// NHD_OLED::textPrintTextFromProgmem
//
// Retrieve and displays text stored in a string table in program memory.
//
// NOTE: Set the cursor position for printing BEFORE calling this function!
//
// Parameters:
//   ptrStringTableEntry: POINTER to text strings in PROGMEM to display.
//
// Note: use "&(_table_[_index_])" to pass a pointer to the specific PROGMEM-
// stored string table entry to display.
//
void NHD_OLED::textPrintTextFromProgmem(int ptrStringTableEntry) {
  // Create a buffer.
  char buf[DISP_COLUMNS + 1];
  
  // Retrieve the st5ring table entry into the buffer.
  strcpy_P(buf, (char*)pgm_read_word(ptrStringTableEntry));
  
  // Find the length of the buffer - NOTE: This assumes that 0x00 is a null
  // end-of-string marker and NOT a custom character in CGROM slot 0x00.
  byte len = 0;
  while ((buf[len] != 0) and (len < DISP_COLUMNS))
    len++;

  // Print the text.
  print(buf, len);
}


// NHD_OLED::textPrintTextFromProgmemCentered
//
// Retrieve and displays text stored in a string table in program memory.
// The text is centered on the selected row/line.
//
// Parameters:
//   ptrStringTableEntry: POINTER to text strings in PROGMEM to display.
//   row: row/line number to print text to (0-1/2/3).
//
// Note: use "&(_table_[_index_])" to pass a pointer to the specific PROGMEM-
// stored string table entry to display.
//
void NHD_OLED::textPrintTextFromProgmemCentered(int ptrStringTableEntry, byte row) {
  // Create a buffer.
  char buf[DISP_COLUMNS + 1];
  
  // Retrieve the st5ring table entry into the buffer.
  strcpy_P(buf, (char*)pgm_read_word(ptrStringTableEntry));
  
  // Find the length of the buffer - NOTE: This assumes that 0x00 is a null
  // end-of-string marker and NOT a custom character in CGROM slot 0x00.
  byte len = 0;
  while ((buf[len] != 0) and (len < DISP_COLUMNS))
    len++;

  // Print the text.
  textPrintCentered(buf, len, row);
}


// NHD_OLED::textSweep
//
// Performs a simple animation that sweeps two characters into the center from
// the outermost columns, then back outward, drawing centered text between
// them as they separate. It's a simple but surprisingly eye-catching effect.
//
// Parameters:
//   text: text to display. 
//         Note that this should be equal to or less than DISP_COLUMNS in
//         character count, or unexpected results like garbled wrapping text
//         can ensue.
//   length: length of text to print, in characters.
//   row: row/line number (0-1/2/3).
//   leftSweepChar: left-to-right sweep character. 
//   rsc: right-to-left sweep character.
//   td: time delay between steps (in milliseconds).
//
void NHD_OLED::textSweep(char *text, byte length, byte row, char leftSweepChar, 
                         char rightSweepChar, unsigned int timeDelay) {
  byte stepnum = 0;
  byte outer, inner, start;
  byte i, ii;
  char line[DISP_COLUMNS];

  // Clear the text buffer.
  for (i = 0; i < DISP_COLUMNS; i++)
    line[i] = 0x20;

  // Copy text into buffer, centering it relative to the line's width.
  for (i = 0; i < length; i++)
    line[((DISP_COLUMNS - length) / 2) + i] = text[i];
    

  // Time to do work. First things first: clear the line we'll print to.
  textClearRow(row);
  

  // The first half of the process: sweep into the center.
  for (i = 0; i < (DISP_COLUMNS / 2); i++){
    cursorMoveToRow(row);

    // Work out some starting positions and widths.
    outer = stepnum;
    inner = DISP_COLUMNS - 2 - (stepnum * 2);
    start = stepnum + 2;
    
    // Print leading spaces to pad the start of the line...
    for (ii = 0; ii < outer; ii++)
      print(0x20);

    // ... then print the left-to-right character...
    print(leftSweepChar);

    // ... followed by spaces in the middle...
    for (ii = 0; ii < inner; ii++)
      print(0x20);

    // ... then the right-to-left character...
    print(rightSweepChar);    

    // ... and finish with more spaces to clear the rest of the line.
    for (ii = 0; ii < outer; ii++)
      print(0x20);

    delay(timeDelay);
    
    stepnum++;
  }


  // Decrement the step number so things stay properly aligned.
  stepnum--;
  

  // The second half: sweep out from the center, leaving text behind.
  for (i = (DISP_COLUMNS / 2); i < DISP_COLUMNS; i++){
    cursorMoveToRow(row);

    // More starts and widths.
    outer = stepnum;
    inner = DISP_COLUMNS - 2 - (stepnum * 2);
    start = stepnum + 1;

    // This pass is similar to the first, starting with padding...
    for (ii = 0; ii < outer; ii++)
      print(0x20);

    // ... then the right-to-left character, as though it passed through its
    // opposite...
    print(rightSweepChar);

    // ... then the centermost part of the text...
    for (ii = 0; ii < inner; ii++)
      print(line[start + ii]);

    // ... then the left-to-right character...
    print(leftSweepChar);    

    // ... then spaces to clear the line.
    for (ii = 0; ii < outer; ii++)
      print(0x20);

    delay(timeDelay);
    
    stepnum--;
  }

  // Finish by printing the text by itself with no side-to-side characters.
  cursorMoveToRow(row);
  print(line, DISP_COLUMNS);
}



// LiquidCrystal Library Compatibility

void NHD_OLED::clear() {
  textClear();
}

void NHD_OLED::home() {
  cursorHome();
}

void NHD_OLED::setCursor(byte col, byte row) {
  cursorPos(row, col);
}

void NHD_OLED::write(byte data) {
  print(data);
}

void NHD_OLED::cursor() {
  displayControl(1, 1, 0);
}

void NHD_OLED::noCursor() {
  displayControl(1, 0, 0);
}

void NHD_OLED::blink() {
  displayControl(1, 1, 1);
}

void NHD_OLED::noBlink() {
  displayControl(1, 1, 0);
}

void NHD_OLED::display() {
  displayOn();
}

void NHD_OLED::noDisplay() {
  displayOff();
}

void NHD_OLED::scrollDisplayLeft() {
  shift(1, 0);
}

void NHD_OLED::scrollDisplayRight() {
  shift(1, 1);
}

void NHD_OLED::autoscroll() {
  // This LiquidCrystal function is not supported in this library.
}

void NHD_OLED::noAutoscroll() {
  // This LiquidCrystal function is not supported in this library.
}

void NHD_OLED::leftToRight() {
  // This LiquidCrystal function is not supported in this library.
}

void NHD_OLED::rightToLeft() {
  // This LiquidCrystal function is not supported in this library.
}

void NHD_OLED::createChar(byte num, char* data) {
  // This LiquidCrystal function is not supported in this library.
}



/*
 * End of file!
 */