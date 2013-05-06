

#include "SrSryrsHub.h"

//custom values used by hardware
#include "HubVars.h"
//Microcontroller
#include <TimerOne.h>
#ifndef CORE_TEENSY
#include <SoftwareSerial.h>
#endif
//xbee
#include <XBee.h>
//Display
#if defined(__Adafruit_Character_OLED__)
#include <Adafruit_CharacterOLED.h>
#else
#include <LiquidCrystal.h>
#endif
//Ethernet
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>
#include <SPI.h>
#include <HttpClient.h>
//Cosm
#include <Cosm.h>
//push-notifications
//#include <Avviso.h>



SrSryrsHub hub;
XBeeAddress64 xbeeNodeAddress[2] = {XBeeAddress64(0x0013a200, 0x40315565)
                                 ,XBeeAddress64(0x0013a200, 0x40315568)};


int ioData;
const int analogThresh = 512;

void setup()
{
  //starts the serial port, lcd, xbee, and ethernet
  hub.init();
  hub.lcd.print("cosm works");
pinMode(INTERNAL_LED, OUTPUT);

delay(5000);
int pinVal = 0x2;
hub.xbeeControlRemotePins(xbeeNodeAddress[0], pinVal);
}

void loop()
{
  digitalWrite(INTERNAL_LED, HIGH);
//  ioData = -1;
  ioData = senseAndControl();
//  hub.cosmSendData(millis()/1000);
//  hub.cosmRequestData();
  delay(200);
  
  digitalWrite(INTERNAL_LED, LOW);
  hub.lcd.setCursor(0,1);
//  hub.lcd.print(hub.cosmControlVal);
  //in the case that a two digit# like 14 changes to one digit like 0,
  //the 4 would remain, showing 04
//  hub.lcd.print("                "); 

  hub.lcdPrintAnalog(ioData);

  delay(250);

}

int senseAndControl()
{

//Requesting and reciving analog values from node 2
hub.hubSerial.println("*");
hub.hubSerial.println("attempting IS");


hub.xbeeForceSampleRequest(xbeeNodeAddress[0], ioData);

hub.hubSerial.println("*");
hub.hubSerial.println("attempting D0");
//Requesting pin set on 
int pinVal = 4;
if(ioData > analogThresh){
    pinVal = 0x5;
   }
  else{
    pinVal = 0x4;
  } 
hub.xbeeControlRemotePins(xbeeNodeAddress[1], pinVal);
return ioData;
}
