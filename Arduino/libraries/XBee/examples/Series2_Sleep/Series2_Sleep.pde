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

/*
This example is for Series 2 (ZigBee) XBee Radios only
This example demonstrates the XBee pin sleep setting, by allowing the Arduino
to sleep/wake the XBee.  
The XBee sleep mode must be set to 1 (SM=1), to enable pin sleep.
I'm using the NewSoftSerial library to communicate with the Arduino since the Arduino's Serial is being used by the XBee
How it works:
When you send a "1", the Arduino will sleep the XBee.
Sending "2" wakes the XBee and "3" will send an arbitrary TX packet.
Of course if the XBee is sleeping, the TX packet will not be sent.
Connect the Arduino Serial Monitor to the usb-serial device to send the commands.
Connect an LED to the XBee Module Status (pin 13).  This will turn on when the XBee is awake and off when it's sleeping
Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
*/

// TODO: monitor XBee current consumption while sleeping
// TODO: send packets while sleeping

// create the XBee object
XBee xbee = XBee();

// create an arbitrary payload -- what we're sending is not relevant
uint8_t payload[] = { 0, 0 };

// SH + SL Address of a remote XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x403e0f30);
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// note: xbee sleep pin doesn't need 3.3. to sleep -- open circuit also will sleep it, but of course needs 0V to wake!
// connect Arduino digital 8 to XBee sleep pin (9) through a voltage divider.  I'm using 10K resistors. 
uint8_t sleepPin = 8;

#if defined(CORE_TEENSY)
	#define nss Serial
#else
// Define SoftwareSerial TX/RX pins
// Connect Arduino pin 9 to TX of usb-serial device
	uint8_t ssRX = 9;
// Connect Arduino pin 10 to RX of usb-serial device
	uint8_t ssTX = 10;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
	SoftwareSerial nss(ssRX, ssTX);
#endif

void setup() {
  pinMode(sleepPin, OUTPUT);
  // set to LOW (wake)
  digitalWrite(sleepPin, LOW);
  
  // start XBee communication
  xbee.begin(9600);
  // start soft serial
  nss.begin(9600);
}

void sendPacket() {

    nss.println("Sending a packet");
      
    xbee.send(zbTx);
    
    // after sending a tx request, we expect a status response
    // wait up to 5 seconds for the status response
    if (xbee.readPacket(5000)) {
        // got a response!

        // should be a znet tx status            	
    	if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
    	   xbee.getResponse().getZBTxStatusResponse(txStatus);
    		
    	   // get the delivery status, the fifth byte
           if (txStatus.getDeliveryStatus() == SUCCESS) {
            	// success.  time to celebrate
             	nss.println("packet was delivered");
           } else {
            	// the remote XBee did not receive our packet. is it powered on?
                nss.println("packet delivery failed");
           }
        }      
    } else {
      // local XBee did not provide a timely TX Status Response -- should not happen
       nss.println("timeout waiting for tx status response");
    }  
}

void loop() {   
  if (nss.available()) {
    int cmd = nss.read();
    
    // ascii 1 == 49
    if (cmd == 49) {   // 1 (ASCII)
      // sleep XBee
      digitalWrite(sleepPin, HIGH);
      nss.println("sleeping xbee");
    } else if (cmd == 50) {  // 2 (ASCII)
      digitalWrite(sleepPin, LOW);
      nss.println("waking xbee");
    } else if (cmd == 51) {  // 3 (ASCII)
      // send packet
      sendPacket();
    } else {
       nss.println("I didn't understand"); 
    }
  }
  
  readPacket();
}

// when XBee powers up it sends a modem status 0 (hardware reset), followed by a 2 (Joined Network), assumming it's configured correctly
// when XBee is woken up it sends a modem status 2 (Joined Network)

void readPacket() {
  xbee.readPacket();
  
    if (xbee.getResponse().isAvailable()) {
        // got something.. print packet to nss
        nss.print("API=");
        nss.print(xbee.getResponse().getApiId(), HEX);
        nss.print(",frame=");
        
        // print frame data
        for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) {
          nss.print(xbee.getResponse().getFrameData()[i], HEX);
          nss.print(" ");
        }
        
        nss.println("");
    } else if (xbee.getResponse().isError()) {
     nss.print("XBee error. error code is");
     nss.println(xbee.getResponse().getErrorCode(), DEC);
    }
}

