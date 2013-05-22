

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
XBeeAddress64 xbeeNodeAddress[] = {
                                  XBeeAddress64(0x0013a200, 0x40315565) //#2
                                 ,XBeeAddress64(0x0013a200, 0x40315568) //#4
                                 ,XBeeAddress64(0x00131200, 0x40321AB1) //#0
                                  };



const int analogThresh = 512;

char server[] ="limitless-headland-1164.herokuapp.com"; //Address of the server you will connect to
//char server[] ="mrlamroger.bol.ucla.edu";
//The location to go to on the server
//make sure to keep HTTP/1.0 at the end, this is telling it what type of file it is
String location = "/testlamp HTTP/1.0";
//String location = "/arduino HTTP/1.0";
//EthernetClient client;
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

char inString[32]; // string for incoming serial data
String variable[32];
String value[32];
int stringPos = 0; // string index counter
boolean startRead = false; // is reading?
boolean readingFirst = true;
int count = 0; //number of variables


void setup()
{
  //starts the serial port, lcd, xbee, and ethernet
  hub.init();
  hub.lcd.print("KnowPlace v1.0");
pinMode(INTERNAL_LED, OUTPUT);
    if (Ethernet.begin(hub.mac) == 0) {
        hub.hubSerial.println("Failed to configure Ethernet using DHCP");
        // DHCP failed, so use a fixed IP address:
        Ethernet.begin(hub.mac, hub.ip);
    }
//delay(5000);

//initialize the pin0 of the sensor node to analog input
int pinVal = 0x2;
//hub.xbeeControlRemotePins(xbeeNodeAddress[0], pinVal);
}

void loop()
{

//senseAndControl();
ethernetScrapeWebsite();
delay(10000);
}


int senseAndControl()
{
digitalWrite(INTERNAL_LED, HIGH);
//Requesting and reciving analog values from node 2
hub.hubSerial.println("*");
hub.hubSerial.println("attempting IS");

int ioData = -1;
hub.xbeeForceSampleRequest(xbeeNodeAddress[0], ioData);
  hub.lcd.setCursor(0,1);
  hub.lcd.print("I:");
  hub.lcdPrintAnalog(ioData);
#ifdef USING_COSM
  hub.cosmSendData(ioData);
  hub.cosmRequestData();  
#endif //USING_COSM
  delay(125);
  digitalWrite(INTERNAL_LED, LOW);
  
  
#ifdef USING_COSM
 ioData = hub.cosmControlVal;
#endif //USING_COSM
  hub.lcd.setCursor(8,1);
  hub.lcd.print("O:");
  hub.lcdPrintAnalog(ioData);
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


String ethernetConnectAndRead(){
    //connect to the server
    
    //Initialize client
    hub.client.stop();
    hub.client.flush();
    hub.hubSerial.println("connecting...");
    
    //port 80 is typical of a www page
    if (hub.client.connect(server, 80)) {
        hub.hubSerial.println("connected");
        hub.client.println("GET /testlamp HTTP/1.1");
        hub.client.println("Host: limitless-headland-1164.herokuapp.com");
        //    hub.client.println("GET /arduino HTTP/1.1");
        //    hub.client.println("Host: mrlamroger.bol.ucla.edu");
        hub.client.println("Connection: close");
        hub.client.println();
        
        //Connected - Read the page
        hub.hubSerial.println("calling ethernetReadPage()");
        return ethernetReadPage(); //go and read the output
        
        
    }else{
        hub.hubSerial.println("connection failed");
    }
    
}

String ethernetReadPage(){
    //read the page, and capture & return everything between '<' and '>'
    
    stringPos = 0;
    memset( &inString, 0, 32 ); //clear inString memory
    count = 0;
    boolean settingVariable = true;
    
    while(true){
        if (hub.client.available()) {
            hub.hubSerial.println("Client is available");
            while (char c = hub.client.read()) {
                if (c == '>') {
                    hub.client.stop();
                    hub.client.flush();
                    hub.hubSerial.println("disconnecting.");
                    return "End";
                } else
                    
                    if (c == '[' ) { //'<' is our begining character
                        startRead = true; //Ready to start reading the part
                        memset( &inString, 0, 32 );
                        stringPos = 0;
                    } else if(startRead){
                        if(c != ']'){ //'>' is our ending character
                            //Serial.print(c);
                            inString[stringPos] = c;
                            stringPos ++;
                        } else {
                            //          hubSerial.println(inString);
                            //          //got what we need here! We can disconnect now
                            //          if(readingFirst){
                            //            String temp = inString;
                            //            hubSerial.println(temp);
                            //            variable[count] = inString;
                            //            readingFirst = false;
                            //          } else {
                            //            String temp = inString;
                            //            hubSerial.println(temp);
                            //            value[count] = temp;
                            //            readingFirst = true;
                            //          }            
                            //          startRead = false;
                            
                            if (settingVariable == true) {
                                variable[count] = inString;
                                startRead = false;
                                settingVariable = false;
                            } else {
                                value[count] = inString;
                                startRead = false;
                                count = count + 1;
                                settingVariable = true;
                            }
                            //Serial.println(variable[count]);
                            //count = count + 1;
                            //memset( &inString, 0, 32 );
                            //Serial.println("Test");
                            //startRead = false;
                            //hub.client.stop();
                            //hub.client.flush();
                            //Serial.println("disconnecting.");
                            //return variable[count - 1];
                        } 
                    }
            }
        }
    }
}

void ethernetScrapeWebsite()
{
    String pageValue = ethernetConnectAndRead();
    hub.hubSerial.println(pageValue);
    for (int i = 0; i < count; i++)
    {
        hub.hubSerial.println(variable[i]);
        hub.hubSerial.println(value[i]);
    }
}
