
////////////////////////////////////////////////////////////////////////////
//
//  XOBXOB_ScrollingMessage :: Ethernet Shield :: MAX7219 LED Matrix
// 
//  This sketch connects to the XOBXOB IoT platform using an Arduino Ethernet shield. 
// 
//
//  The MIT License (MIT)
//  
//  Copyright (c) 2013 Robert W. Gallup, XOBXOB
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.// 

#include <SPI.h>
#include <Ethernet.h>
#include <XOBXOB.h>

#include <LEDMatrix7219.h>
#include <vincent90.h>

///////////////////////////////////////////////////////////
//
// Change these for your Ethernet Shield and your APIKey
//
// NOTE: the MAC address for your Ethernet Shield might be
// printed on a label on the bottom of the shield. Your
// APIKey will be found on your account dashboard when you
// login to XOBXOB)
//

byte mac[]    = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
String APIKey = "xxxx-xxxx-xxxx-xxxx-xxxx";

///////////////////////////////////////////////////////////


// Create ethernet and XOBXOB clients
EthernetClient eClient;
XOBXOB xClient (&eClient, APIKey);

// Response processing
boolean lastResponseReceived = true;
long lastRequestTime = -20000;

long timerStart;

// Message variables
int charNum = -1;
int nextCharNum;
String message = " Waiting... ";
String newMessage;

// Create the display object (use default parameters: 1, 3, 4, 5)
LEDMatrix7219 LEDMatrix(1, 15);

void setup() {

  // Begin Ethernet. If not successful, flash LED and hang
  if (Ethernet.begin(mac) == 0) {
    byte light = 0; while (true) {digitalWrite (8, light^=1); delay (100);}
  }

}

void loop() {

  ////////////////////////////////////////////////////////
  //
  // XOBXOB GET Request

  // Request every 10 seconds (but, only if done with previous response)
  if (lastResponseReceived && ( (millis() - lastRequestTime) > 10*1000)) {

    // Connect to XOBXOB, if necessary
    while (!xClient.connected()) xClient.connect();

    // Now, update lastResponseReceived flag and timer, and
    // request information from XOB named "XOB"
    lastResponseReceived = false;
    lastRequestTime = millis();
    xClient.requestXOB("XOB");
    
  }
  
  ////////////////////////////////////////////////////////
  //
  // Display message
  
  charNum = (charNum+1) % message.length();
  nextCharNum = (charNum+1) % message.length();

  for (int column=0; column<8; column++) {
    
    timerStart = millis();
    
    int c = 0;
    for (int i=column; i<8; i++) {
      LEDMatrix.writeDeviceRegister(1, MAX7219_REG_COL0 + c, pgm_read_byte_near(*vincent90 + int(message[charNum])*8 + i) );
      c++;
    };
    for (int i=0; i<column; i++) {
      LEDMatrix.writeDeviceRegister(1, MAX7219_REG_COL0 + c, pgm_read_byte_near(*vincent90 + int(message[nextCharNum])*8 + i) );
      c++;
    };
  
    ////////////////////////////////////////////////////////
    //
    // Load response
  
    // Check the response (for 75 milliseconds).
    // "True" means a completed JSON object has been received, so extract the message
    
    while (abs(millis() - timerStart) < 75) {
      if (!lastResponseReceived && xClient.checkResponse()) {
	      lastResponseReceived = true;
	      newMessage = xClient.getMessage("text");
	      newMessage = newMessage.substring(1, newMessage.length()-1);
	      if (!message.equals(newMessage)) {
	      	charNum = -1;
	      	message = newMessage;
	      }
      }
    }
  }

}


