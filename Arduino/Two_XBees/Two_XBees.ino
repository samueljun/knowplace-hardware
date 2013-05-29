/**
 * Copyright (c) 2009 Andrew Rapp. All rights reserved.
 *
 * This file is part of XBee-Arduino.
 *
 * XBee-Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XBee-Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <SoftwareSerial.h>
#include "XBee.h"

HardwareSerial Uart = HardwareSerial();
/*
This example is for Series 1 (10C8 or later firmware) or Series 2 XBee radios
Sends a few AT command queries to the radio and checks the status response for success

This example uses the NewSoftSerial library to view the XBee communication.  I am using a 
Modern Device USB BUB board (http://moderndevice.com/connect) and viewing the output
with the Arduino Serial Monitor.    
You can obtain the NewSoftSerial library here http://arduiniana.org/libraries/NewSoftSerial/
*/

#if defined(CORE_TEENSY)
	#define nss Serial
	uint8_t ssRX = 14;
// Connect Arduino pin 10 to RX of usb-serial device
	uint8_t ssTX = 13;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
	SoftwareSerial ss(ssRX, ssTX);
#else
// Define NewSoftSerial TX/RX pins
// Connect Arduino pin 9 to TX of usb-serial device
	uint8_t ssRX = 9;
// Connect Arduino pin 10 to RX of usb-serial device
	uint8_t ssTX = 10;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
	SoftwareSerial ss(ssRX, ssTX);
#endif

const int resetPin = 12;
int serialToggle = 0;

//XBee xbeeuart = XBee(Uart);
XBee xbeeuart = XBee();
XBee xbeess = XBee(ss);

// serial high
uint8_t shCmd[] = {'S','H'};
// serial low
uint8_t slCmd[] = {'S','L'};
// association status

uint8_t idCmd[] = {'I','D'};
uint8_t assocCmd[] = {'A','I'};

AtCommandRequest atRequest = AtCommandRequest(shCmd);

AtCommandResponse atResponse = AtCommandResponse();

void sendXbeeCommands(XBee &xbee);
boolean sendAtCommand(XBee &xbee);

void setup() { 
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);
  
  ss.begin(9600);
  Uart.begin(9600);
//  xbeess.setSerial(ss);
  xbeeuart.begin(9600);
//xbee.setSerial(ss);
// start soft serial
  nss.begin(9600);
  
  // Startup delay to wait for XBee radio to initialize.
  // you may need to increase this value if you are not getting a response
  delay(5000);
}

void loop() {
  Serial.println();
Serial.print("attempt:  ");
Serial.println(millis());

if(serialToggle == 0)
{
  serialToggle = 1;
//  xbeeuart.setSerial(Uart);
  nss.println("UART!");
  sendXbeeCommands(xbeeuart);
}
else
{
  serialToggle = 0;
//  xbee.setSerial(ss);
 nss.println("SoftwareSerial!");
while(digitalRead(resetPin) == HIGH);
nss.println("Button Released");
while(digitalRead(resetPin) == LOW);
delay(2000);
sendXbeeCommands(xbeess);
}
//ss.println("testUart");

//  digitalWrite(resetPin, LOW);
//delay(20);
//digitalWrite(resetPin, HIGH);
//delay(5000);


  
}

void sendXbeeCommands(XBee &xbee)
{
  // get SH
  atRequest.setCommand(shCmd);
//   sendAtCommand(xbee); 
while(!sendAtCommand(xbee));
  Serial.println("*********************");
 //set command to SL
  atRequest.setCommand(slCmd);  
//   sendAtCommand(xbee); 
while(!sendAtCommand(xbee));
  Serial.println("*********************");
    atRequest.setCommand(idCmd);  
//   sendAtCommand(xbee); 
while(!sendAtCommand(xbee));
Serial.println("*********************");
  // set command to AI
  atRequest.setCommand(assocCmd); 
//   sendAtCommand(xbee); 
while(!sendAtCommand(xbee));
  Serial.println("*********************");
  // we're done.  Hit the Arduino reset button to start the sketch over
//delay(5000);
}

boolean sendAtCommand(XBee &xbee) {
  boolean ret = false;
  nss.println("Sending command to the XBee");

  // send the command
  xbee.send(atRequest);

  // wait up to 5 seconds for the status response
  if (xbee.readPacket(1000)) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(atResponse);

      if (atResponse.isOk()) {
        nss.print("Command [");
        nss.print(atResponse.getCommand()[0]);
        nss.print(atResponse.getCommand()[1]);
        nss.println("] was successful!");

        if (atResponse.getValueLength() > 0) {
          nss.print("Command value length is ");
          nss.println(atResponse.getValueLength(), DEC);

          nss.print("Command value: ");
          
          for (int i = 0; i < atResponse.getValueLength(); i++) {
            nss.print(atResponse.getValue()[i], HEX);
            nss.print(" ");
          }

          nss.println("");
          
        }
        ret = true;
      } 
      else {
        nss.print("Command return error code: ");
        nss.println(atResponse.getStatus(), HEX);
      }
    } else {
      nss.print("Expected AT response but got ");
      nss.print(xbee.getResponse().getApiId(), HEX);
    }   
  } else {
    // at command failed
    if (xbee.getResponse().isError()) {
      nss.print("Error reading packet.  Error code: ");  
      nss.println(xbee.getResponse().getErrorCode());
    } 
    else {
      nss.println("No response from radio"); 
    }
  }
  return ret;
}


