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
#include <Cosm.h>

//push-notifications
//#include <Avviso.h>


//TODO I really don't like this method, want to do it in the constructor
//star bad method

static char cosmShareStreamId[] = COSM_SHARE_STREAM_ID;
static CosmDatastream cosmShareDataStreams[] = {
    CosmDatastream(cosmShareStreamId, strlen(cosmShareStreamId), DATASTREAM_INT),
};

static char cosmControlStreamId[] = COSM_CONTROL_STREAM_ID;
static CosmDatastream cosmControlDataStreams[] = {
    CosmDatastream(cosmControlStreamId, strlen(cosmControlStreamId), DATASTREAM_INT),
};
//end bad method


class SrSryrsHub {
public:
    
	//Default Constructor
	SrSryrsHub();
    void init();
    ////////////////////////////////////
    //////     Microcontroller     /////
    ////////////////////////////////////
    Stream &hubSerial;
    
	////////////////////////
    //////     LCD     /////
    ////////////////////////
#if defined(__Adafruit_Character_OLED__)
    Adafruit_CharacterOLED lcd;
#elif defined(__Standard_Character_LCD__)
    LiquidCrystal lcd;
#endif
    
	////////////////////////
    /////     XBee     /////
    ////////////////////////
    int analogValue[4]; 	//received either from "IS" command or I/O received

    
    void xbeeSendRemoteAtCommand();
    boolean xbeeReceiveRemoteAtResponse();
    void xbeeForceSampleRequest(XBeeAddress64 *remoteAddress);
    void xbeeReceiveIOData();
    void xbeeSwitchNodePin0(XBeeAddress64 *remoteAddress, boolean onOff);
    void xbeeSwitchNodePins(XBeeAddress64 *remoteAddress, char pins /*stored in first four bits*/);
    
	////////////////////////////
    /////     Ethernet     /////
    ////////////////////////////
	IPAddress ip; //TODO check if this can be private
	EthernetClient client;
    
    
    ////////////////////////
    /////     COSM     /////
    ////////////////////////
    int cosmControlVar;
    String cosmControlString;
    
    void cosmSendData(int thisData);
    void cosmRequestData();
	
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
    //used for storing incomming IO address before sending to database
    //probably redundant because the request and response variable
    //  have an address component
//	XBeeAddress64 receivedRemoteAddress; 
    RemoteAtCommandRequest remoteAtRequest;
	RemoteAtCommandResponse remoteAtResponse;
    ZBRxIoSampleResponse ioSample;

	    
    void xbeeSetCommand(uint8_t cmd_set[2]);
    void xbeeSetCommand(uint8_t cmd_set[2], uint8_t *value_set, uint8_t valueLength_set);

    ////////////////////////////
    /////     Ethernet     /////
    ////////////////////////////
	byte mac[6];
    
    
    ////////////////////////
    /////     COSM     /////
    ////////////////////////
    
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
    /*
    boolean lastConnected;
    unsigned long cosmLastPutTime;
    unsigned long cosmLastGetTime;
    const unsigned long postingInterval;
    
    int cosmDigitalState[10];//TODO fit to need, this is from example
    int cosmAnalogState[4];//TODO fit to needs, this is from example
    */
    
    
};

/////////////////////////////////////
//////     Helper Functions     /////
/////////////////////////////////////


#endif //SRSRYRS_HUB_h
