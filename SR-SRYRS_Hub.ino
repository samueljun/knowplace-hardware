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

#include <XBeeT2.h>
#include <Adafruit_CharacterOLED.h>

/*
This example is for Series 1 (10C8 or later firmware) or Series 2 XBee
Sends two Remote AT commands to configure the remote radio for I/O line monitoring

This example uses the NewSoftSerial library to view the XBee communication.  I am using a 
Modern Device USB BUB board (http://moderndevice.com/connect) and viewing the output
with the Arduino Serial Monitor.    
You can obtain the NewSoftSerial library here http://arduiniana.org/libraries/NewSoftSerial/
*/

Adafruit_CharacterOLED lcd(21, 15, 20, 19, 18, 17, 16);
    #define INTERNAL_LED 11
    
 #define APP_NAME "Mail Center"

// Define NewSoftSerial TX/RX pins
// Connect Arduino pin 9 to TX of usb-serial device
//uint8_t ssRX = 9;
// Connect Arduino pin 10 to RX of usb-serial device
//uint8_t ssTX = 10;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
//NewSoftSerial nss(ssRX, ssTX);
#define analogThresh 512

#define nss Serial
XBee xbee = XBee();


// Turn on I/O sampling
uint8_t isCmd[] = {'I','S'};
uint8_t irCmd[] = {'I','R'};
// Set sample rate to 65 seconds (0xffff/1000)
uint8_t irValue[] = { 0x03, 0xE8 };

// Set DIO0 (pin 20) to Analog Input
uint8_t d0Cmd[] = { 'D', '0' };
uint8_t d0Value[] = { 0x2 };

//global analog value pin
int analogValue[4] = {0,0,0,0};

// SH + SL of your remote radio
XBeeAddress64 remoteAddress[2] = {XBeeAddress64(0x0013a200, 0x40315565)
                                 ,XBeeAddress64(0x0013a200, 0x40315568)};

// Create a remote AT request with the IR command
//RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(remoteAddress[0], irCmd, irValue, sizeof(irValue));
RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(remoteAddress[0], d0Cmd, d0Value, sizeof(d0Value));
//RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(remoteAddress[0], isCmd);
  
// Create a Remote AT response object
RemoteAtCommandResponse remoteAtResponse = RemoteAtCommandResponse();
ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();

void setup() { 
  xbee.begin(9600);
  // start soft serial
  nss.begin(9600);
  
  lcd.begin(16,2);
  lcd.home();
  lcd.print(APP_NAME);
 // When powered on, XBee radios require a few seconds to start up
 // and join the network.
 // During this time, any packets sent to the radio are ignored.
 // Series 2 radios send a modem status packet on startup.
 
 // it took about 4 seconds for mine to return modem status.
 // In my experience, series 1 radios take a bit longer to associate.
 // Of course if the radio has been powered on for some time before the sketch runs,
 // you can safely remove this delay.
 // Or if you both commands are not successful, try increasing the delay.
 
 delay(5000);
 
  sendRemoteAtCommand();
  receiveIOData(); 
}

void loop() {
  forceSampleRequest();
  lcd.setCursor(0,1);
  if(analogValue[0] > analogThresh){
    lcd.print("Mail Time! ");
  }
  else{
    lcd.print("No Mail :( ");
  }
  lcd.setCursor(12,0);
      if(analogValue[0] < 1000) lcd.print("0");
      if(analogValue[0] < 100 ) lcd.print("0");
      if(analogValue[0] < 10  ) lcd.print("0");
      lcd.print(analogValue[0]);
      
  delay(250);
  
  // now reuse same object for DIO0 command
//  remoteAtRequest.setCommand(d0Cmd);
//  remoteAtRequest.setCommandValue(d0Value);
//  remoteAtRequest.setCommandValueLength(sizeof(d0Value));
//
//  sendRemoteAtCommand();
  
  // it's a good idea to clear the set value so that the object can be reused for a query
  
  // we're done
//  while (1) {};
}

void sendRemoteAtCommand() {
  nss.println("Sending command to the XBee");

  // send the command
  xbee.send(remoteAtRequest);

  // wait up to 5 seconds for the status response
  if (xbee.readPacket(5000)) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE) {
      xbee.getResponse().getRemoteAtCommandResponse(remoteAtResponse);

      if (remoteAtResponse.isOk()) {
        nss.print("Command [");
        nss.print(remoteAtResponse.getCommand()[0]);
        nss.print(remoteAtResponse.getCommand()[1]);
        nss.println("] was successful!");

        if (remoteAtResponse.getValueLength() > 0) {
          nss.print("Command value length is ");
          nss.println(remoteAtResponse.getValueLength(), DEC);

          nss.print("Command value: ");
          
          for (int i = 0; i < remoteAtResponse.getValueLength(); i++) {
            nss.print(remoteAtResponse.getValue()[i], HEX);
            nss.print(" ");
          }
          
          int analogHigh = remoteAtResponse.getValue()[4] ;
          int analogLow = remoteAtResponse.getValue()[5] ;
          analogValue[0] = (analogHigh<<8) + analogLow;
          
          nss.print("analogValue: ");
          nss.print(analogValue[0]);
         
          nss.println("");
        }
      } else {
        nss.print("Command returned error code: ");
        nss.println(remoteAtResponse.getStatus(), HEX);
      }
    } else {
      nss.print("Expected Remote AT response but got ");
      nss.print(xbee.getResponse().getApiId(), HEX);
    }    
  } else {
    // remote at command failed
    if (xbee.getResponse().isError()) {
      nss.print("Error reading packet.  Error code: ");  
      nss.println(xbee.getResponse().getErrorCode());
    } else {
      nss.print("No response from radio");  
    }
  }
}

void receiveIOData()
{
    xbee.readPacket();

  if (xbee.getResponse().isAvailable()) {
    // got something

    if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE) {
      xbee.getResponse().getZBRxIoSampleResponse(ioSample);

      nss.print("Received I/O Sample from: ");
      
      nss.print(ioSample.getRemoteAddress64().getMsb(), HEX);  
      nss.print(ioSample.getRemoteAddress64().getLsb(), HEX);  
      nss.println("");
      
      if (ioSample.containsAnalog()) {
        nss.println("Sample contains analog data");
      }

      if (ioSample.containsDigital()) {
        nss.println("Sample contains digtal data");
      }      

      // read analog inputs
      for (int i = 0; i <= 4; i++) {
        if (ioSample.isAnalogEnabled(i)) {
          nss.print("Analog (AI");
          nss.print(i, DEC);
          nss.print(") is ");
          nss.println(ioSample.getAnalog(i), DEC);
          analogValue[i] = ioSample.getAnalog(i);
        }
      }

      // check digital inputs
      for (int i = 0; i <= 12; i++) {
        if (ioSample.isDigitalEnabled(i)) {
          nss.print("Digital (DI");
          nss.print(i, DEC);
          nss.print(") is ");
          nss.println(ioSample.isDigitalOn(i), DEC);
        }
      }
      
      // method for printing the entire frame data
      //for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) {
      //  nss.print("byte [");
      //  nss.print(i, DEC);
      //  nss.print("] is ");
      //  nss.println(xbee.getResponse().getFrameData()[i], HEX);
      //}
    } 
    else {
      nss.print("Expected I/O Sample, but got ");
      nss.print(xbee.getResponse().getApiId(), HEX);
    }    
  } 
  else if (xbee.getResponse().isError()) {
    nss.print("Error reading packet.  Error code: ");  
    nss.println(xbee.getResponse().getErrorCode());
  }
}

void setAtRequestToIS()
{
  remoteAtRequest.clearCommandValue();
  remoteAtRequest.setCommand(isCmd);
  remoteAtRequest.setRemoteAddress64(remoteAddress[0]);
  
//  remoteAtRequest.setCommandValue(0);
//  remoteAtRequest.setCommandValueLength(0);
}

void setAtRequestToD0()
{
  remoteAtRequest.clearCommandValue();
   remoteAtRequest.setRemoteAddress64(remoteAddress[1]);
  remoteAtRequest.setCommand(d0Cmd);
 
   if(analogValue[0] > analogThresh){
    d0Value[0] = 0x5;
   }
  else{
    d0Value[0] = 0x4;
  } 
  remoteAtRequest.setCommandValue(d0Value);
  remoteAtRequest.setCommandValueLength(sizeof(d0Value));
}
void forceSampleRequest()
{
//Requesting and reciving analog values from node 2
nss.println("*");
nss.println("attempting IS");
setAtRequestToIS();
 sendRemoteAtCommand();
//  receiveIOData(); 

nss.println("*");
nss.println("attempting D0");
//Requesting pin set on 
  setAtRequestToD0();
   sendRemoteAtCommand();
     //  receiveIOData(); 
}
