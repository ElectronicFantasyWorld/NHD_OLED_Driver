
Newhaven Display Slim OLED Driver - Version 1.1
===============================================================================



What is this?
=========================-=--=---=----=-----=------=-------=--------=---------=

This is a comprehensive driver for running Newhaven's slim OLED character 
displays from just about any Arduino with enough free pins to connect to one. 
The interface being used is SPI, but it's bit-banged on two Arduino pins so
there's no requirement for the Arduino to have hardware SPI support. Check the
datasheet for your display to wire it to the Arduino in SPI mode.

Note that as of the 1.1 version, a chip-select pin is no longer required.
Simply tie the CS line on the display to ground.

In order to save on precious memory, the library uses very little global RAM.
Variables are mainly declared inside functions so memory use is far more local
than global.

The driver was designed and tested against a Newhaven NHD-0420CW-AB3 slim
OLED display module connected to an Arduino Mega 2560 v3, and an Arduino Pro
Mini 8MHz/3.3v was also successfully implemented. The library should work with
few changes with any Newhaven slim OLED module and any Arduino, although
some unused functions might need to be cut if memory space is at a premium.



How do I wire up the display?
=========================-=--=---=----=-----=------=-------=--------=---------=

This driver is designed to use bit-banged SPI to drive the display, which has
two benefits:

1. Since it's a bit-bang drive, your Arduino doesn't have to have hardware
   SPI. All you need are two available pins. This means that this driver
   should work with practically any Arduino and Arduino derivative, e.g. 
   Adafruit's Trinkets.
2. Since we're only using two pins, this code can work with projects that
   make heavier use of the rest of the Arduino.

Connecting the display is actually pretty straightforward:

1. Connect the VSS pin (pin 1 on the display) to VSS and the ground pin (pin 
   20 on the display) to ground.
2. Connect the VDD pin (display pin 2) to the VSS pin (display pin 1).
3. Connect the REGVDD pin (display pin 3) to EITHER VSS if your project uses
   5VDC power, or to ground if you're using 3.3V.
4. Connect the SCLK pin (display pin 7) to the Arduino, and note the pin
   number on the Arduino that you're using as the driver must be told to use
   this pin for SPI clock (SCLK) line.
5. Connect the SDI pin (display pin 8) to the Arduino, and again note the pin
   number - this will the SPI master-out-serial-in (MOSI) line.
6. Leave pin 9 disconnected, as we are not using bidirectional SPI in this
   driver and won't need to read data from the display.
7. Connect the /RES pin (display pin 16) to VSS.
8. Connect all other pins (except pin 9) on the display to ground.

This can be breadboarded quite easily, but for ease of use when converting 
your project into a permanent hardwired device, you may want to create an
adapter board to "backpack" onto the display and convert its connections into 
something easier to work with that's appropriate for your project, such as a 
five-pin header. We leave this as "an exercise for the reader."



How do I use this?
=========================-=--=---=----=-----=------=-------=--------=---------=

1. Create a folder in your Arduino libraries folder, which is usually (on
   Windows) in Users\<YourUsername>\Documents\Arduino\libraries. Copy the
   entire contents of the archive into this folder. Relaunch the Arduino IDE 
   if it's running so it'll see the library.
2. Open your project in the Arduino IDE and add the library to it. To do this,
   click the "Sketch" menu, then click "Include Library," then scroll down the
   list to find and click "NHD_OLED_Driver." This will add an #include to your
   project.
3. Declare a global instance of the library, outside any functions or 
   procedures and before setup()/main(). Example:
   
     NHD_OLED oled;
     
4. In setup(), call begin() to set up the display's size, set which pins are 
   connected to the display, and initialize the display so it's ready to use.
5. Make your project use the display.
6. Bask in the glory of bright, crisp OLED displays.



Now how do I USE this?
=========================-=--=---=----=-----=------=-------=--------=---------=

Assuming the display is wired correctly to work as a SPI slave to the Arduino,
and assuming the pin and size information is correct, using the display should
be a simple case of calling the begin() procedure from setup():

  // Initialize the driver and display in a single line of code.
  oled.begin(1,2,2,16);
  
The above sets up the driver to work with a 2-row/16-character display whose
SCLK pin is connected to pin 1 on the Arduino, and SDI pin is connected to
Arduino pin 2.


If you prefer separating the statrtup calls, you can accomplish the same as
above by calling these three startup functions from setup():

  // Set up the size of the display. This example is 2 rows, 16 characters per
  // row.
  oled.setupDisplay(2,16)

  // Set up the pins the display uses. These will be the Arduino pins
  // connected to the SCLK and SDIN pins on the display, respectively.
  oled.setupPins(1,2);
  
  // Initialize the display.
  oled.setupInit();


Note that the above assumes you've declared an object named "oled" via:

  NHD_OLED oled;


The display will still be "off" at this point because OLED displays only light
up their pixels if they have something to show. So, send these commands after
the above:

  // Clear the display.
  oled.textClear();
  
  // Set cursor to home (top-left).
  oled.cursorHome();
  
  // Print something.
  oled.print("Hello World!", 12);
  
The display should show baby's first print. If not, check your wiring and
make sure you've passed the correct pins in the correct order to the begin() 
function.

Note that we're sending a string and a number to the oled.print() function.
This is necessary because Arduino treats strings as char arrays and arrays
are passed via pointers, but C/C++ doesn't have a way to get the SIZE OF a
char array from a POINTER TO a char array. So, you'll have to tell the print
function how many characters you want to print since it cannot determine this
on its own. (Or, you can use a loop over the string and send it byte-by-byte
if you want, as there's an overloaded print function that only takes one char.)


For the sake of clarity, functions are grouped by name. Functions that
configure the driver are given names that start with "setup," e.g. "setupPins."
Functions that control the display start with "display," e.g., "displayOn."
Functions that namipulate the display's internal cursor position start with
"cursor," e.g., "cursorHome." And of course, text manipulation functions
start with "text," e.g., "textClear." The exception to this pattern are the 
"print" functions, which are just "print" since it's assumed that "printing" 
to a device would involve send text to it that it should subsequently display.


The example sketch highlights several common operations performed with
character displays, as well as a few special things like text justification,
basic animations, and using string tables stored in program memory.



IMPORTANT NOTE ON STRINGS/CHARS

C/C++ strings end in nulls (0x00), but to a LCD or OLED display, character 
code 0x00 is an actual character. When sending text to the display, remember
to NOT send a 0x00 unless displaying character 0 is the desired result!



Functions you might want to use
=========================-=--=---=----=-----=------=-------=--------=---------=

Functions included within the driver are:

begin(byte pinSCLK, byte pinSDIN, byte rows = 2, byte columns = 16)
  Configures the driver and initialize the display, all in one command. This
  procedure calls the setupDisplaySize, setupPins, and setupInit procedures
  in sequence. Call this before using the display.

SPIBitBang(byte data, byte commdata);
  SPI bit-bang driver designed to use any three available Arduino pins.
  Hardware SPI is not required, and if the Arduino has hardware SPI this
  driver can, but doesn't have to, use its pins. This function doesn't 
  generally need to be called directly.
  
sendCommand(byte command);
  Sends a single command byte to the display. This function doesn't generally
  need to be called directly.
  
sendData(byte data);
  Sends a single data byte to the display. This function doesn't generally
  need to be called directly.
  
setupDisplaySize(byte rows = 2, byte columns = 16);
  Configures the driver to understand the size of the display, in rows and 
  columns. Call either this or begin() before using the display.
  
setupPins(byte pinSCLK, byte pinSDIN);
  Configures the driver to know which two pins to use to communicate with
  the display. Call either this or begin () before using the display.
  
setupInit();
  Initializes the display's hardware for use. Call either this or begin()
  before using the display.
  
displayControl(byte display, byte cursor, byte block);
  Control whether the display is on or off (byte display), whether to show the
  cursor (byte cursor), and whether to show a block cursor instead of a line 
  (byte block).
  
displayOn();
  Turns the display ON, showing whatever text is currently stored in the
  display's memory.
  
displayOff();
  Turns the display OFF. Text and commands can still be sent to the display,
  but nothing will be shown until an on() or display_control() command is used
  to turn the display back on.
  
textClear();
  Clears the display.
  
cursorHome();
  Moves the cursor position to the top-left of the display.
  
cursorMoveToRow(byte rowNumber);
  Moves the cursor to the start of the given row/line. Rows/lines are zero-
  indexed, so the topmost row/line is 0.
  
textClearLine(byte rowNumber);
  Clears the given row/line. Rows/lines are zero-indexed, so the topmost row/
  line is 0.
  
shift(byte dc, byte rl);
  Shifts either the cursor or the text in the display (byte dc) right or left 
  (byte rl) one position.
  
pos(byte r, byte c);
  Sets the cursor to the given position, as rows (byte r) and columns (byte 
  c). Lines and columsn within lines are zero-indexed, so the topmost line is
  0 and the leftmost column in a line is also 0.
  
print(char *text, byte len);
  Prints a specific number of chars (byte len) in a char array (char *text)
  starting at the current cursor position. The cursor position moves as needed
  to the end of the text.
  
print(char text);
  Prints a single char (char text) at the current cursor position. The cursor
  position then moves one step to the right, wrapping to the start of the next
  line as needed.
  
print(char *text, byte len, byte r, byte c);
  Prints a specific number of chars (byte len) in a char array (char *text)
  starting at the given row and column (byte r, byte c). The cursor position
  moves as needed to the end of the text.
  
print(char text, byte r, byte c);
  Prints a single char (char text) at the given row and column (byte r, byte 
  c). The cursor position moves one step to the right, wrapping to the start
  of the next line as needed.
  
textPrintCentered(char *text, byte length, byte row);
  Automatically center a specific number of chars (byte length) in a char 
  array (char *text), and print it to the given line (byte row). The cursor 
  position moves to the start of the next line.

textPrintRightJustified(char *text, byte length, byte row);
  Automatically right-justify a specific number of chars (byte length) in a 
  char array (char *text), and print it to the given line (byte row). The 
  cursor position moves to the start of the next line.
  
textPrintTextFromProgmem(int ptrStringTableEntry);
  Retrieves a string table entry from program memory (PROGMEM) and displays it
  starting at the current cursor position. The cursor position moves as needed 
  to the end of the text. The string table entry (int ptrStringTableEntry) 
  must be passed via pointer, which can be obtained via "&(_table_[_index_])".  

textPrintTextFromProgmemCentered(int ptrStringTableEntry, byte row);
  Retrieves a string table entry from program memory (PROGMEM) and displays it
  centered on the gien line (byte r). The cursor position moves as needed 
  to the start of the next line. The string table entry (int 
  ptrStringTableEntry) must be passed via pointer, which can be obtained via 
  "&(_table_[_index_])".  

textSweep(char *text, byte length, byte row, char leftSweepChar, 
          char rightSweepChar, unsigned int timeDelay);
  Performs a simple animation that uses a specific number of chars (byte 
  length) in a char array (char *text) that sweeps two characters (byte 
  leftSweepChar, byte rightSweepChar) from the outside to the center and back 
  to the outside, leaving the text behind. This is performed on a given line 
  (byte row). An adjustable time delay also occurs between each step (byte 
  timeDelay). Useful for flashscreens or alert messages because the movement
  catches the eye.



Who made this?
=========================-=--=---=----=-----=------=-------=--------=---------=

Base SPI bit-bang code was provided by Saurabh Baxi, Applications Engineer at 
Newhaven Display International, Inc. This is the magic bit that lots of other
OLED drivers seem to get wrong.

The rest of this mess of code came from some guy named Tom Honaker.



What's the version history?
=========================-=--=---=----=-----=------=-------=--------=---------=

1.0: Initial release



What are the license terms?
=========================-=--=---=----=-----=------=-------=--------=---------=

Software License Agreement (BSD License)

Copyright (c) 2015-2017 by Newhaven Display International, Inc.
Copyright (c) 2017-2018 by Tom Honaker.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



=========================-=--=---=----=-----=------=-------=--------=---------=
END!