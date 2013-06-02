

#include "KnowPlaceHub.h"

//custom values used by hardware
#include "HubVars.h"
//Microcontroller
#include <TimerOne.h>
#ifndef CORE_TEENSY
#include <SoftwareSerial.h>
#endif
//#include "aJSON.h"
#include "JsonArduino.h"
//xbee
#include <XBee.h>
//Display
//#if defined(__Adafruit_Character_OLED__)
//#include <Adafruit_CharacterOLED.h>
//#else
//#include <LiquidCrystal.h>
//#endif
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



KnowPlaceHub hub;
XBeeAddress64 *xbeeNodeAddress = NULL;
//XBeeAddress64 xbeeNodeAddress[] = {
//                                  XBeeAddress64(0x0013a200, 0x40315565) //#2
//                                 ,XBeeAddress64(0x0013a200, 0x40315568) //#4
//                                 ,XBeeAddress64(0x00131200, 0x40321AB1) //#0
//                                  };



const int analogThresh = 512;

void setup()
{
  //starts the serial port, lcd, xbee, and ethernet
  hub.init();
//  hub.lcd.print("KnowPlace v1.0");
pinMode(INTERNAL_LED, OUTPUT);


//delay(5000);

//initialize the pin0 of the sensor node to analog input
//int pinVal = 0x2;
//hub.xbeeControlRemotePins(xbeeNodeAddress[0], pinVal);
}

void loop()
{

//senseAndControl();
//hub.ethernetScrapeWebsite(1);

//controlFromWeb(1);
//delay(10000);

//if the button is pressed, an attempt will be made to add the node
hub.addNodeToWeb();
//hub.hubSerial.println("Button not pressed");
delay(1000);
}

void controlFromWeb(int node_address)
{
  hub.ethernetScrapeWebsite(node_address);
//  hub.ethernetConnectAndRead(node_address);
  int pinVal = 0x4 + hub.getDeviceStatus(node_address);
  hub.hubSerial.print("pinVal will be: ");
  hub.hubSerial.println(pinVal);
  if(pinVal == 4 || pinVal == 5)
  {
    hub.xbeeControlRemotePins(xbeeNodeAddress[node_address], pinVal );
  }
}
int senseAndControl()
{
digitalWrite(INTERNAL_LED, HIGH);
//Requesting and reciving analog values from node 2
hub.hubSerial.println("*");
hub.hubSerial.println("attempting IS");

int ioData = -1;
hub.xbeeForceSampleRequest(xbeeNodeAddress[0], ioData);
//  hub.lcd.setCursor(0,1);
//  hub.lcd.print("I:");
//  hub.lcdPrintAnalog(ioData);
#ifdef USING_COSM
  hub.cosmSendData(ioData);
  hub.cosmRequestData();  
#endif //USING_COSM
  delay(125);
  digitalWrite(INTERNAL_LED, LOW);
  
  
#ifdef USING_COSM
 ioData = hub.cosmControlVal;
#endif //USING_COSM

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
  
  delay(125);
  return ioData;
}

