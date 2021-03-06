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
#include <SoftwareSerial.h>
#include <string.h>
//#include "aJSON.h"
#include "JsonArduino.h"

//XBee
#include <XBee.h>

//Display
#define USING_LCD
#ifdef USING_LCD
#if defined(__Adafruit_Character_OLED__)
#include "Adafruit_CharacterOLED.h"
#else
#include <LiquidCrystal.h>
#endif //__Aadafruit_Character_OLED__
#endif //USING_LCD


//Ethernet
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>
#include <HttpClient.h>
#include <SPI.h>

#define USING_PROCESSING_AS_INTERNET

//Cosm
//#define USING_COSM
#ifdef USING_COSM
#include <Cosm.h>
#endif
//push-notifications
//#include <Avviso.h>


//TODO I really don't like this method, want to do it in the constructor
//star bad method
#ifdef USING_COSM
//static char cosmShareStreamId[] = COSM_SHARE_STREAM_ID;
//static CosmDatastream cosmShareDataStreams[] = {
//    CosmDatastream(cosmShareStreamId, strlen(cosmShareStreamId), DATASTREAM_INT),
//};
//
//static char cosmControlStreamId[] = COSM_CONTROL_STREAM_ID;
//static CosmDatastream cosmControlDataStreams[] = {
//    CosmDatastream(cosmControlStreamId, strlen(cosmControlStreamId), DATASTREAM_INT),
//};
//end bad method
#endif //USING_COSM


class XBeeNodeMessage {
public:
    XBeeNodeMessage();
    void resetChangedPin(uint8_t pin);
    void resetChangedBitmap();
    void setAddressH(uint32_t xbeeAddressH);
    void setAddressL(uint32_t xbeeAddressL);
    void setAddress(uint32_t xbeeAddressH, uint32_t xbeeAddressL);
    void setData(uint8_t pin, uint8_t type, uint8_t data);
    
    boolean checkChanged(uint8_t pin);
    uint16_t getChangedBitmap();
    uint32_t getAddressH();
    uint32_t getAddressL();
    XBeeAddress64 getAddress();
    int getData(uint8_t pin);
    int getDataType(uint8_t pin);
    
private:
    XBeeAddress64 m_xba64;
    uint16_t m_changed; //bitmap so only changed pin requests get sent to xbee
    uint8_t m_type[16];
    uint8_t m_data[16];
    
};

class XBeeIOData {
public:
    XBeeIOData(); //initializes to -1
    void init();
    void clear();
    
    int digital[12];//could change to two bytes to save memory
    int analog[4];
};

class KnowPlaceHub {
public:
    
	//Default Constructor
	KnowPlaceHub();
    void init();
    ////////////////////////////////////
    //////     Microcontroller     /////
    ////////////////////////////////////
    Stream &hubSerial;
    int internalLed;
    
	////////////////////////
    //////     LCD     /////
    ////////////////////////
#ifdef USING_LCD
#if defined(__Adafruit_Character_OLED__)
    Adafruit_CharacterOLED lcd;
#elif defined(__Standard_Character_LCD__)
    LiquidCrystal lcd;
#endif

    
    void lcdPrintAnalog(int analog);
#endif //USING_LCD
	////////////////////////
    /////     XBee     /////
    ////////////////////////
    int xbeeAddPin;
    
    void xbeeSendAtCommand(XBee &xbee);
    boolean xbeeReceiveAtResponse(XBee &xbee);
//    boolean xbeeGetCoordinatorPanID(uint8_t *idVal, uint8_t valSize);
//    boolean xbeeSetNodePanID(uint8_t *idVal, uint8_t valSize);
    
    void xbeeSendRemoteAtCommand();
    void xbeeReceiveRemoteAtResponse();
    void xbeeReceiveRemoteAtResponse(int &ioData);
//    void xbeeReceiveIOData(int &ioData);
    void xbeeForceSampleRequest(XBeeAddress64 remoteAddress, int ioData);
    void xbeeControlRemotePins(XBeeAddress64 remoteAddress, int ioData);
    boolean xbeePwmTxRequest(XBeeAddress64 remoteAddress, uint16_t pwmVal);
    
    void addNodeToWeb();
    
    ////////////////////////////
    /////     Internet     /////
    ////////////////////////////
    
	////////////////////////////
    /////     Ethernet     /////
    ////////////////////////////

	EthernetClient client;
    byte mac[6];
    IPAddress ip; //TODO check if this can be private
    
    boolean ethernetConnect();
    void ethernetCloseConnection();
    void ethernetDisconnect();
    boolean ethernetGetRequest();
    boolean ethernetPostNewNodeAddress(XBeeAddress64 &nodeAddress);
    
    boolean ethernetReadPage(XBeeNodeMessage &dataPacket);
    
    boolean ethernetPostDataJson(XBeeNodeMessage &dataPacket);
    
    void ethernetScrapeWebsite();
    
    int getDeviceStatus(/*uint32_t hubApiKey, XBeeAddress64*/int node_address);
    
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
    ////////////////////////////////////////
    //////     Microcontroller/Hub     /////
    ////////////////////////////////////////
    uint32_t hubApiKey;
    int internetSource;
    void internetDisconnect();

    ////////////////////////
    //////     LCD     /////
    ////////////////////////
    
    
    ////////////////////////
    /////     XBee     /////
    ////////////////////////
	XBee xbee;
    XBee xbeeSS; //software Serial for adding a node
    SoftwareSerial ssXbee;

    
    AtCommandRequest atRequest;
    AtCommandResponse atResponse;
    
    XBeeAddress64 xba64;
    XBeeNodeMessage nodeMessage;
    RemoteAtCommandRequest remoteAtRequest;
	RemoteAtCommandResponse remoteAtResponse;
    
//    uint16_t payload[2];
//    
//    ZBTxRequest zbTx;
//    ZBTxStatusResponse txStatus;
    ZBRxIoSampleResponse ioSample;

    void storeXBeeAddress64(uint8_t addrH[4], uint8_t addrL[4]);
    void xbeeSetAtCommand(uint8_t *cmd_set);
    void xbeeSetAtCommand(uint8_t *cmd_set, uint8_t *value_set, uint8_t valueLength_set);
    
    void xbeeSetRemoteAtCommand(uint8_t *cmd_set);
    void xbeeSetRemoteAtCommand(uint8_t *cmd_set, uint8_t *value_set, uint8_t valueLength_set);

    ////////////////////////////
    /////     Internet     /////
    ////////////////////////////
        char internetReadChar();
    
    ////////////////////////////
    /////     Ethernet     /////
    ////////////////////////////
    //Roger's function
    boolean startRead;
    boolean readingFirst;
    char inString[32];
    char server[38];
    String location;
    String variable[32];
    String value[32];
    int stringPos;
    int count;
    
//    int getDeviceStatus(int id); //MOVED TO PUBLIC SECTION
    void ethernetClientPrintAddress64(XBeeAddress64 node_address);

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
    char *cosmShareStreamId;
    CosmDatastream cosmShareDataStream1;
    CosmDatastream *cosmShareDataStreams;
    char *cosmControlStreamId;
    CosmDatastream cosmControlDataStream1;
    CosmDatastream *cosmControlDataStreams;
#endif //USING_COSM
};

/////////////////////////////////////
//////     Helper Functions     /////
/////////////////////////////////////


#endif //SRSRYRS_HUB_h
