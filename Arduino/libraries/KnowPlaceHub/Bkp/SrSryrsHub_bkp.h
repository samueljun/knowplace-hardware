#ifndef SRSRYRS_HUB_h
#define SRSRYRS_HUB_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <inttypes.h>

//custom values used by hardware
#include "HubVars.h"

//Microcontroller
//timer interrupts
#include <TimerOne.h>
#ifndef CORE_TEENSY
#include <SoftwareSerial.h>
#endif

//XBee
#include <XBee.h>

//Display
#if defined(__Adafruit_Character_OLED__)
#include "Adafruit_CharacterOLED.h"
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
#include <HttpClient.h>

//Cosm
//#define USING_COSM
#ifdef USING_COSM
#include <Cosm.h>
#endif
//push-notifications
//#include <Avviso.h>

//#define USINGETHERNET

//TODO I really don't like this method, want to do it in the constructor
//star bad method
#ifdef USING_COSM
static char cosmShareStreamId[] = COSM_SHARE_STREAM_ID;
static CosmDatastream cosmShareDataStreams[] = {
    CosmDatastream(cosmShareStreamId, strlen(cosmShareStreamId), DATASTREAM_INT),
};

static char cosmControlStreamId[] = COSM_CONTROL_STREAM_ID;
static CosmDatastream cosmControlDataStreams[] = {
    CosmDatastream(cosmControlStreamId, strlen(cosmControlStreamId), DATASTREAM_INT),
};
//end bad method
#endif //USING_COSM

class XBeeIOData {
public:
    XBeeIOData(); //initializes to -1
    void init();
    void clear();
    
    int digital[12];//could change to two bytes to save memory
    int analog[4];
};

class SrSryrsHub {
public:
    
	//Default Constructor
	SrSryrsHub();
    void init();
    ////////////////////////////////////
    //////     Microcontroller     /////
    ////////////////////////////////////
    Stream &hubSerial;
    int internalLed;
    
	////////////////////////
    //////     LCD     /////
    ////////////////////////
#if defined(__Adafruit_Character_OLED__)
    Adafruit_CharacterOLED lcd;
#elif defined(__Standard_Character_LCD__)
    LiquidCrystal lcd;
#endif
    
    void lcdPrintAnalog(int analog);
    
	////////////////////////
    /////     XBee     /////
    ////////////////////////
    
    void xbeeSendRemoteAtCommand();
    void xbeeReceiveRemoteAtResponse();
    void xbeeReceiveRemoteAtResponse(int &ioData);
//    void xbeeReceiveIOData(int &ioData);
    void xbeeForceSampleRequest(XBeeAddress64 &remoteAddress, int &ioData);
    void xbeeControlRemotePins(XBeeAddress64 &remoteAddress, int &ioData);
    
	////////////////////////////
    /////     Ethernet     /////
    ////////////////////////////

	EthernetClient client;
    
    
    ////////////////////////
    /////     COSM     /////
    ////////////////////////
#ifdef USING_COSM
    int cosmControlVal;
    String cosmControlString;
    
    void cosmSendData(int thisData);
    void cosmRequestData();
#endif //USING_COSM
    
private:
    ////////////////////////////////////
    //////     Microcontroller     /////
    ////////////////////////////////////
    
    
    ////////////////////////
    //////     LCD     /////
    ////////////////////////
    
    
    ////////////////////////
    /////     XBee     /////
    ////////////////////////
	XBee xbee;
    XBeeAddress64 xba64;
    RemoteAtCommandRequest remoteAtRequest;
	RemoteAtCommandResponse remoteAtResponse;
    ZBRxIoSampleResponse ioSample;

	    
    void xbeeSetCommand(uint8_t *cmd_set);
    void xbeeSetCommand(uint8_t *cmd_set, uint8_t *value_set, uint8_t valueLength_set);

    ////////////////////////////
    /////     Ethernet     /////
    ////////////////////////////
	byte mac[6];
    IPAddress ip; //TODO check if this can be private
    
    ////////////////////////
    /////     COSM     /////
    ////////////////////////
#ifdef USING_COSM
    char *cosmAPIKey;
    int cosmShareFeedID;
    int cosmControlFeedID;
    char *cosmUserAgent;
    
    CosmClient cosmClient;

    CosmFeed cosmShareFeed;
    CosmFeed cosmControlFeed;
    IPAddress cosmServer;
         
    //made these global constanst for initializing reasons
    //    CosmDatastream cosmShareDataStreams[1];
    //    CosmDatastream cosmControlDataStreams[1];
#endif //USING_COSM
};

/////////////////////////////////////
//////     Helper Functions     /////
/////////////////////////////////////


#endif //SRSRYRS_HUB_h
