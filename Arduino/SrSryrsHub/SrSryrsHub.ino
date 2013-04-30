

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
XBeeAddress64 xbeeNodeAddress[] =
  {XBeeAddress64(0x0013a200, 0x40315565)
  ,XBeeAddress64(0x0013a200, 0x40315568)};


void setup()
{
  hub.init();
  hub.lcd.print("cosm works");
pinMode(INTERNAL_LED, OUTPUT);
}

//int iter = 0;
void loop()
{
  digitalWrite(INTERNAL_LED, HIGH);
  hub.xbeeForceSampleRequest(xbeeNodeAddress[]);
  hub.cosmSendData(millis()/1000);
  hub.cosmRequestData();
  delay(200);
  
  digitalWrite(INTERNAL_LED, LOW);
  hub.lcd.setCursor(0,1);
  hub.lcd.print(hub.cosmControlVal);
  //in the case that 14 changes to zero, the 4 would remain ->04
  hub.lcd.print("                "); 
  hub.xbeeSendRemoteAtCommand(xbeeNodeAdderss[]);
//  iter++;
  delay(6000);
  
}
