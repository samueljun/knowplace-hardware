#ifndef HUBVARS_h
#define HUBVARS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <SoftwareSerial.h>

///////////////////////////////////
/////     Microcontroller     /////
///////////////////////////////////
//interrupt timers
//#define CHECK_TIME 750000 //TODO decide if this should be completely up to user

//Serial baud rate
#define SERIAL_BAUD 9600

//SoftwareSerial
#define SSRX 2
#define SSTX 3

//board specific attributes
//Serial pins
//internal LEDs

// Arduino Mega - has choice of 4 serial ports
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
//#define DEFAULT_SERIAL Serial
#define DEFAULT_SERIAL Serial1
//#define DEFAULT_SERIAL Serial2
//#define DEFAULT_SERIAL Serial3

#define INTERNAL_LED 13

// Teensy - one serial port, but "Serial" is USB virtual serial
#elif defined(CORE_TEENSY)
#define DEFAULT_SERIAL Uart
static HardwareSerial Uart = HardwareSerial();

#define INTERNAL_LED 11
//TODO
//add the three teensy3 serial ports

// Sanguino - choice of 2 serial ports
#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__)
//#define DEFAULT_SERIAL Serial
#define DEFAULT_SERIAL Serial1

//not confirmed
//  #define INTERNAL_LED 1

// Arduino Uno, Duemilanove, Lilypad, Nano, etc - only one serial port
#else
#define DEFAULT_SERIAL Serial

#define INTERNAL_LED 13
#endif

#define HUB_API_KEY 31415 //todo: should change to a string

/////////////////////////////////////////
/////     LCD Character Display     /////
/////////////////////////////////////////

//using the Adafruit 16x2, 3.3v, OLED
#define __Adafruit_Character_OLED__
//regular 16x2, 5v, LCD
//#define __Standard_Character_LCD__

#define LCD_ROWS  2
#define LCD_COLS 16

#if defined(__Adafruit_Character_OLED__)
//need to update this once I make a fixed hub
   #define LCD_RS        21
   #define LCD_RW        15
   #define LCD_Enable 	 20
   #define LCD_D4        19
   #define LCD_D5        18
   #define LCD_D6        17
   #define LCD_D7        16
#elif defined(__Standard_Character_LCD__)
   #define LCD_RS         4
   #define LCD_RW       255
   #define LCD_Enable  	  2
   #define LCD_D4         5
   #define LCD_D5         6
   #define LCD_D6         7
   #define LCD_D7         8
#endif

////////////////////////
/////     XBee     /////
////////////////////////
//XBee baud rate
#define XBEE_BAUD 9600
#define XBEE_ADD_PIN 6
#define XBEE_ADD_RX 14
#define XBEE_ADD_TX 13
static SoftwareSerial xbeeSoftwareSerial = SoftwareSerial(XBEE_ADD_RX, XBEE_ADD_TX);

////////////////////////////
/////     Ethernet     /////
////////////////////////////
#define IP0      192
#define IP1      168
#define IP2        1
#define IP3      177

#define MAC_0 	0xDE
#define MAC_1	0xAD
#define MAC_2	0xBE
#define MAC_3	0xEF
#define MAC_4	0xFE
#define MAC_5	0xED

#define SERVER_NUM 80

#define KNOWPLACE_SERVER "limitless-headland-1164.herokuapp.com"

////////////////////////
/////     COSM     /////
////////////////////////

//#define COSM_API_KEY         "9b3CoOAC2a1qZX0VvL_mKKLI5waSAKx0VjlUQUVWYmd6Yz0g" // replace your pachube api key here
//#define COSM_API_KEY "-mr8-P9iC5MspLd4BZ_g6g_2wrCSAKxzU2wvZ2VSSVRqYz0g"
#define COSM_API_KEY "-mr8-P9iC5MspLd4BZ_g6g_2wrCSAKxzU2wvZ2VSSVRqYz0g"
#define COSM_SHARE_FEED_ID      126870 // replace your feed ID
#define COSM_SHARE_STREAM_ID    "share"
//#define COSM_CONTROL_FEED_ID    126820 //use this while figuring out the control feed
#define COSM_CONTROL_FEED_ID    126870
#define COSM_CONTROL_STREAM_ID    "control"
#define COSM_USER_AGENT   "My Project" // user agent is the project name

#define COSM_IP0 216
#define COSM_IP1  52
#define COSM_IP2 233
#define COSM_IP3 122

#define COSM_POSTINGINTERVAL 10000 //10seconds

#endif //HUBVARS_h