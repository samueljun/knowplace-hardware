#include "HubVars.h"
#include "SrSryrsHub.h"


SrSryrsHub::SrSryrsHub() 
		      : lcd(LCD_RS, LCD_RW, LCD_Enable, LCD_D4, LCD_D5, LCD_D6, LCD_D7)
			,ip(IP0, IP1, IP2, IP3)
            ,cosmAPIKey(COSM_API_KEY)
            ,cosmUserAgent(COSM_USER_AGENT)
//            ,cosmServer(COSM_IP0, COSM_IP1, COSM_IP2, COSM_IP3)//(216,52,233,122)
            ,cosmClient(client)
            ,cosmShareFeed(COSM_SHARE_FEED_ID, cosmShareDataStreams, 1)
//            ,cosmShareFeed(COSM_SHARE_FEED_ID, &CosmDatastream("what", strlen("what"), DATASTREAM_INT), 1)
            ,cosmControlFeed(COSM_CONTROL_FEED_ID, cosmControlDataStreams, 1)
//            ,receivedRemoteAddress(0x0013a200, 0x40315565)
            ,hubSerial(Serial)
//            ,postingInterval(COSM_POSTINGINTERVAL)
//			,server(SERVER_NUM)
{
    ///////////////////////////////////
    /////     Microcontroller     /////
    ///////////////////////////////////
    
    
    ////////////////////////
    //////     LCD     /////
    ////////////////////////


    ////////////////////////
    /////     XBee     /////
    ////////////////////////
    for(int iter = 0; iter < 4; iter++){
        analogValue[iter] = 0;
    }
    
    ////////////////////////////
    /////     Ethernet     /////
    ////////////////////////////
    mac[0] = MAC_0;
    mac[1] = MAC_1;
    mac[2] = MAC_2;
    mac[3] = MAC_3;
    mac[4] = MAC_4;
    mac[5] = MAC_5;

    
    
    ////////////////////////
    /////     COSM     /////
    ////////////////////////
//    lastConnected = false;
//    cosmLastPutTime = 0;
//    cosmLastGetTime = 0;
//    cosmAPIKey = COSM_API_KEY;
    cosmControlVar = 0;
    cosmShareFeedID = COSM_SHARE_FEED_ID;
    cosmControlFeedID = COSM_CONTROL_FEED_ID;
//    cosmUserAgent = COSM_USER_AGENT;
    
//    char temperatureId[] = "temperature";
//    datastreams[1] = CosmDatastream(temperatureId, strlen(temperatureId), DATASTREAM_FLOAT);
    /*//start pachube example copy
    found_status_200 = false;
    found_session_id = false;
    found_control_values = false;

    successes = 0;
    failures = 0;
    ready_to_update = true;
    reading_pachube = false;
    
    request_pause = false;
    found_content = false;
    
    found_d = false;
    found_a = false;
    
    *///end pachube exapmle copy

}

void SrSryrsHub::init()
{
    //Microcontroller
    pinMode(INTERNAL_LED, OUTPUT);
#if defined(CORE_TEENSY)
    Serial.begin(SERIAL_BAUD);
    hubSerial = Serial;
#else
    SoftwareSerial ss(SSRX,SSTX); //SSRX == 2, SSTX == 3 by default
    ss.begin(SERIAL_BAUD);
    hubSerial = ss;//TODO verify that these pins are open
#endif
    
    //Display
    lcd.begin(LCD_ROWS, LCD_COLS);
    
    //XBee
    xbee.begin(XBEE_BAUD);
    //xbee.setSerial(); //here just so you know it's an option 
    
    //Ethernet
    if (Ethernet.begin(mac) == 0) {
        hubSerial.println("Failed to configure Ethernet using DHCP");
        // DHCP failed, so use a fixed IP address:
        Ethernet.begin(mac, ip);
    }


    
    
}

void SrSryrsHub::xbeeSetCommand(uint8_t cmd[2])
{
    remoteAtRequest.clearCommandValue();
    remoteAtRequest.setCommand(cmd);
}

void SrSryrsHub::xbeeSetCommand(uint8_t cmd[2], uint8_t *value, uint8_t valueLength)
{
    remoteAtRequest.clearCommandValue();
    remoteAtRequest.setCommand(cmd);
    remoteAtRequest.setCommandValue(value);
    remoteAtRequest.setCommandValueLength(valueLength);
}

void SrSryrsHub::xbeeSendRemoteAtCommand() {
    hubSerial.println("Sending command to the XBee");
    
    // send the command
    xbee.send(remoteAtRequest);
    
}

//this is a specific form of response
//It should only be called when no other response types are cared for.
boolean SrSryrsHub::xbeeReceiveRemoteAtResponse(){
    // wait up to 5 seconds for the status response
    //TODO make this more general to allow for receiving I/O
    boolean ret = false;
    
    if (xbee.readPacket(5000)) {
        // got a response!
        
        // should be an AT command response
        if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE) {
            xbee.getResponse().getRemoteAtCommandResponse(remoteAtResponse);
            
            if (remoteAtResponse.isOk()) {
                hubSerial.print("Command [");
                hubSerial.print(remoteAtResponse.getCommand()[0]);
                hubSerial.print(remoteAtResponse.getCommand()[1]);
                hubSerial.println("] was successful!");
                
                if (remoteAtResponse.getValueLength() > 0) {
                    hubSerial.print("Command value length is ");
                    hubSerial.println(remoteAtResponse.getValueLength(), DEC);
                    
                    hubSerial.print("Command value: ");
                    
                    for (int i = 0; i < remoteAtResponse.getValueLength(); i++) {
                        hubSerial.print(remoteAtResponse.getValue()[i], HEX);
                        hubSerial.print(" ");
                    }
                    
                    int analogHigh = remoteAtResponse.getValue()[4] ;
                    int analogLow = remoteAtResponse.getValue()[5] ;
                    analogValue[0] = (analogHigh<<8) + analogLow;
                    
                    hubSerial.print("analogValue: ");
                    hubSerial.print(analogValue[0]);
                    
                    hubSerial.println("");
                }
                ret = true;
            } else {
                hubSerial.print("Command returned error code: ");
                hubSerial.println(remoteAtResponse.getStatus(), HEX);
            }
        } else {
            hubSerial.print("Expected Remote AT response but got ");
            hubSerial.print(xbee.getResponse().getApiId(), HEX);
        }
    } else {
        // remote at command failed
        if (xbee.getResponse().isError()) {
            hubSerial.print("Error reading packet.  Error code: ");
            hubSerial.println(xbee.getResponse().getErrorCode());
        } else {
            hubSerial.print("No response from radio");
        }
    }
    return ret;
}

void SrSryrsHub::xbeeForceSampleRequest(XBeeAddress64 *remoteAddress)
{
    remoteAtRequest.setRemoteAddress64(*remoteAddress);
    uint8_t isCmd[2] = {'I', 'S'};
    xbeeSetCommand(isCmd);
    xbeeSendRemoteAtCommand();
    xbeeReceiveRemoteAtResponse();
}

void SrSryrsHub::xbeeReceiveIOData()
{
    xbee.readPacket();
    
    if (xbee.getResponse().isAvailable()) {
        // got something
        
        if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE) {
            xbee.getResponse().getZBRxIoSampleResponse(ioSample);
            
            hubSerial.print("Received I/O Sample from: ");
            
            hubSerial.print(ioSample.getRemoteAddress64().getMsb(), HEX);
            hubSerial.print(ioSample.getRemoteAddress64().getLsb(), HEX);
            hubSerial.println("");
            
            if (ioSample.containsAnalog()) {
                hubSerial.println("Sample contains analog data");
            }
            
            if (ioSample.containsDigital()) {
                hubSerial.println("Sample contains digtal data");
            }
            
            // read analog inputs
            for (int i = 0; i <= 4; i++) {
                if (ioSample.isAnalogEnabled(i)) {
                    hubSerial.print("Analog (AI");
                    hubSerial.print(i, DEC);
                    hubSerial.print(") is ");
                    hubSerial.println(ioSample.getAnalog(i), DEC);
                    analogValue[i] = ioSample.getAnalog(i);
                }
            }
            
            // check digital inputs
            for (int i = 0; i <= 12; i++) {
                if (ioSample.isDigitalEnabled(i)) {
                    hubSerial.print("Digital (DI");
                    hubSerial.print(i, DEC);
                    hubSerial.print(") is ");
                    hubSerial.println(ioSample.isDigitalOn(i), DEC);
                }
            }
            
            // method for printing the entire frame data
            //for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) {
            //  hubSerial.print("byte [");
            //  hubSerial.print(i, DEC);
            //  hubSerial.print("] is ");
            //  hubSerial.println(xbee.getResponse().getFrameData()[i], HEX);
            //}
        } 
        else {
            hubSerial.print("Expected I/O Sample, but got ");
            hubSerial.print(xbee.getResponse().getApiId(), HEX);
        }    
    } 
    else if (xbee.getResponse().isError()) {
        hubSerial.print("Error reading packet.  Error code: ");  
        hubSerial.println(xbee.getResponse().getErrorCode());
    }
}

void SrSryrsHub::xbeeSwitchNodePin0(XBeeAddress64 remoteAddress,boolean onOff)
{
    char cmd[] = {'D','0'};
    //input 0 gives command value 4 (digital low)
    //input 1 gives command value 5 (digital high)
    char cmdVal[] = {4 + onOff};
    do{
    xbeeSetCommand(cmd, cmdVal, sizeof(cmdVal));
    }while(xbeeReceiveRemoteAtResponse() == 0);
}

void SrSryrsHub::xbeeSwitchNodePin0(XBeeAddress64 remoteAddress, char pins)
{
    //TODO complete
//    char cmd[] = {'D','0'};
//    char cmdVal[] = {0x4};
//    for (int iter = 0; iter < 4; iter++, cmd[1]++) {
//        xbeeSetCommand(cmd, &(cmd[0] + ));
//    }
}
                           
////////////////////////
/////     COSM     /////
////////////////////////
// this method makes a HTTP connection to the cosmServer:
void SrSryrsHub::cosmSendData(int thisData)
{
    int sensorValue = thisData;//analogRead(sensorPin);
    cosmShareDataStreams[0].setInt(sensorValue);
    
    hubSerial.print("Read sensor value ");
    hubSerial.println(cosmShareDataStreams[0].getInt());
    
    hubSerial.println("Uploading it to Cosm");
    int ret = cosmClient.put(cosmShareFeed, cosmAPIKey);
    hubSerial.print("cosmclient.put returned ");
    hubSerial.println(ret);
    
    hubSerial.println();
}

void SrSryrsHub::cosmRequestData()
{
    hubSerial.println("Get data from Cosm...");
    int ret = cosmClient.get(cosmControlFeed, cosmAPIKey);
    hubSerial.print("cosmClient.get returned ");
    hubSerial.println(ret);
    
    if (ret > 0)
    {
        hubSerial.println("Datastream is...");
        hubSerial.println(cosmControlFeed[0]);
        
        hubSerial.print("cosmControlStream is: ");
        hubSerial.println(cosmControlFeed[0].getString());
//        cosmControlVar = cosmControlFeed[0].getSTRING();
        cosmControlString = cosmControlFeed[0].getString();
    }
    
    hubSerial.println();
}

/////////////////////////////////////
//////     Helper Functions     /////
/////////////////////////////////////
