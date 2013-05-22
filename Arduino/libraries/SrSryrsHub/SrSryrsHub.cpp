#include "HubVars.h"
#include "SrSryrsHub.h"

XBeeIOData::XBeeIOData()
{
    init();
//    for(int i = 0; i < 12; i++)
//    {
//        digital[iter] = -1;
//    }
//    for(int i = 0; i < 4; i++)
//    {
//        analog[i] = -1;
//    }
}

void XBeeIOData::init()
{
    for(int i = 0; i < 12; i++)
    {
        digital[i] = -1;
    }
    for(int i = 0; i < 4; i++)
    {
        analog[i] = -1;
    }
}

void XBeeIOData::clear()
{
    //just for readability 
    init();
}
SrSryrsHub::SrSryrsHub()
		      :hubSerial(Serial)
            ,lcd(LCD_RS, LCD_RW, LCD_Enable, LCD_D4, LCD_D5, LCD_D6, LCD_D7)
            ,xbee(XBee())
            ,remoteAtRequest(RemoteAtCommandRequest(xba64, NULL, NULL, sizeof(NULL)))
            ,remoteAtResponse(RemoteAtCommandResponse())
            ,ioSample(ZBRxIoSampleResponse())
			,ip(IP0, IP1, IP2, IP3)
            ,server(KNOWPLACE_SERVER)
#ifdef USING_COSM
            ,cosmAPIKey(COSM_API_KEY)
            ,cosmUserAgent(COSM_USER_AGENT)
            ,cosmServer(COSM_IP0, COSM_IP1, COSM_IP2, COSM_IP3)//(216,52,233,122)
            ,cosmClient(client)
            ,cosmShareStreamId(COSM_SHARE_STREAM_ID)
            ,cosmControlStreamId(COSM_CONTROL_STREAM_ID)
            ,cosmShareDataStream1(cosmShareStreamId, strlen(cosmShareStreamId), DATASTREAM_INT)
            ,cosmControlDataStream1(cosmControlStreamId, strlen(cosmControlStreamId), DATASTREAM_INT)
            ,cosmShareDataStreams(&cosmShareDataStream1)
            ,cosmControlDataStreams(&cosmControlDataStream1)
            ,cosmShareFeed(COSM_SHARE_FEED_ID, cosmShareDataStreams, 1)
            ,cosmControlFeed(COSM_CONTROL_FEED_ID, cosmControlDataStreams, 1)
#endif //USING_COSM

//			,server(SERVER_NUM)
{
    ///////////////////////////////////
    /////     Microcontroller     /////
    ///////////////////////////////////
    internalLed = INTERNAL_LED;
    
    ////////////////////////
    //////     LCD     /////
    ////////////////////////


    ////////////////////////
    /////     XBee     /////
    ////////////////////////
//
    
    ////////////////////////////
    /////     Ethernet     /////
    ////////////////////////////
    mac[0] = MAC_0;
    mac[1] = MAC_1;
    mac[2] = MAC_2;
    mac[3] = MAC_3;
    mac[4] = MAC_4;
    mac[5] = MAC_5;
    
    startRead = false;
    readingFirst = true;
    stringPos = 0;
    count = 0;
    
    ////////////////////////
    /////     COSM     /////
    ////////////////////////
#ifdef USING_COSM
    cosmControlVal = 0;
    cosmShareFeedID = COSM_SHARE_FEED_ID;
    cosmControlFeedID = COSM_CONTROL_FEED_ID;
    
//    cosmShareDataStreams[0] = malloc(sizeof(CosmDatastream));
//    CosmDatastream CosmDatastreamTemp1(cosmShareStreamId, strlen(cosmShareStreamId), DATASTREAM_INT);
//    cosmShareDataStreams = &CosmDatastreamTemp1;
//    
//    cosmControlDataStreams[0] = malloc(sizeof(CosmDatastream));
//    CosmDatastream CosmDatastreamTemp2(cosmControlStreamId, strlen(cosmControlStreamId), DATASTREAM_INT);
//    cosmControlDataStreams = &CosmDatastreamTemp2;

#endif USING_COSM

}

void SrSryrsHub::init()
{
    //Microcontroller
    pinMode(INTERNAL_LED, OUTPUT);
#if defined(CORE_TEENSY)
    Serial.begin(SERIAL_BAUD);
    hubSerial = Serial;
#else
    //because the xbee uses the regular serial port, create a software serial connection for debuggin
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
#ifdef USINGETHERNET
    if (Ethernet.begin(mac) == 0) {
        hubSerial.println("Failed to configure Ethernet using DHCP");
        // DHCP failed, so use a fixed IP address:
        Ethernet.begin(mac, ip);
    }
#endif //USINGETHERNET
}
////////////////////////
//////     LCD     /////
////////////////////////

//prints leading zeros of four digit analog values
//should only be used for 4 digits (assumed max of 1023)
//could be made more general
//maybe there's already an lcd print function that prints leading zeros
void SrSryrsHub::lcdPrintAnalog(int analog)
{
    if(analog < 0)
    {
        lcd.print("-NEG");
    }
    else if(analog < 10000)
    {
        if(analog < 1000) lcd.print("0");
        if(analog < 100 ) lcd.print("0");
        if(analog < 10  ) lcd.print("0");
        lcd.print(analog);
    }
    else lcd.print(">MAX");
}

////////////////////////
/////     XBee     /////
////////////////////////

void SrSryrsHub::xbeeSetCommand(uint8_t *cmd)
{
    remoteAtRequest.clearCommandValue();
    remoteAtRequest.setCommand(cmd);
}

void SrSryrsHub::xbeeSetCommand(uint8_t *cmd, uint8_t *value, uint8_t valueLength)
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
    //todo check for command sent
}


//for the cases where the response is just a confirmation, no data included
void SrSryrsHub::xbeeReceiveRemoteAtResponse() {
    int tempVar = 0;
    xbeeReceiveRemoteAtResponse(tempVar);
}

//for use when issuing and IS remoteAtCommand and data is sent back as a reponse
void SrSryrsHub::xbeeReceiveRemoteAtResponse(int &ioData) {
    //  hubSerial.println("Sending command to the XBee");
    
    // send the command
    //  xbee.send(remoteAtRequest);
    
    // wait up to 5 seconds for the status response
    if (xbee.readPacket(5000)) {
        // got a response!
        
        // should be an AT command response
        if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE) {
            xbee.getResponse().getRemoteAtCommandResponse(remoteAtResponse);
            
            //ryan's test
            hubSerial.print("Received I/O Sample from: ");
            
            hubSerial.print(remoteAtResponse.getRemoteAddress64().getMsb(), HEX);
            hubSerial.print("  ");
            hubSerial.println(remoteAtResponse.getRemoteAddress64().getLsb(), HEX);
            //end ryan's test
            
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
                    ioData = (analogHigh<<8) + analogLow;
                    
                    hubSerial.print("analogValue: ");
                    hubSerial.print(ioData);
                    
                    hubSerial.println("");
                }
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
}




/*
 boolean SrSryrsHub::xbeeReceiveIOData(XBeeIOData &ioData) //todo iodata
{
    xbee.readPacket();
    
    int ret = 0; //return status
    
    if (xbee.getResponse().isAvailable()) {
        // got something
        
        if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE) {
            xbee.getResponse().getZBRxIoSampleResponse(ioSample);
            
            hubSerial.print("Received I/O Sample from: ");
            
            hubSerial.print(ioSample.getRemoteAddress64().getMsb(), HEX);
            hubSerial.print(ioSample.getRemoteAddress64().getLsb(), HEX);
            hubSerial.println("");
            
            //todo, don't know if ret should only be true if something was returned
            //it's possible that there's an IO with no pins configured
            ret = true; 
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
                    ioData.analog[i] = ioSample.getAnalog(i);
                }
            }
            
            // check digital inputs
            for (int i = 0; i <= 12; i++) {
                if (ioSample.isDigitalEnabled(i)) {
                    hubSerial.print("Digital (DI");
                    hubSerial.print(i, DEC);
                    hubSerial.print(") is ");
                    hubSerial.println(ioSample.isDigitalOn(i), DEC);
                    ioData.digital[i] = ioSample.isDigitalOn(i);
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
    
    return ret;
}
*/


void SrSryrsHub::xbeeForceSampleRequest(XBeeAddress64 &remoteAddress, int &ioData)//todo iodata
{
    //todo check to see if pass by reference is fine, or if it should be a pointer
    //pass by reference is readable
    remoteAtRequest.setRemoteAddress64(remoteAddress);
    uint8_t isCmd[2] = {'I', 'S'}; //IS command requests IO sample
    xbeeSetCommand(isCmd);
    xbeeSendRemoteAtCommand();
    xbeeReceiveRemoteAtResponse(ioData);
}

void SrSryrsHub::xbeeControlRemotePins(XBeeAddress64 &remoteAddress, int &ioData)
{
    //todo make this more intuitive
    //passing 4s and 5s doens't make sense
    //maybe limit this to 0s and 1s where 0 is off and 1 is on
    //limit pin set 2 for pins 0-3
    remoteAtRequest.setRemoteAddress64(remoteAddress);
    uint8_t dCmd[2] = {'D', '0'}; //IS command requests IO sample
    uint8_t dVal[] = {4};
    if(ioData != -1)
    {
        dVal[0] = ioData; //todo check if it's a valid value
        remoteAtRequest.clearCommandValue();
        xbeeSetCommand((uint8_t *)dCmd, (uint8_t *)dVal, sizeof(dVal));
        xbeeSendRemoteAtCommand();
        xbeeReceiveRemoteAtResponse();
    }
}


////////////////////////////
/////     Ethernet     /////
////////////////////////////

String SrSryrsHub::ethernetConnectAndRead(){
    //connect to the server
    
    //Initialize client
    client.stop();
    client.flush();
    Serial.println("connecting...");
    
    //port 80 is typical of a www page
    if (client.connect(server, 80)) {
        hubSerial.println("connected");
        client.println("GET /testlamp HTTP/1.1");
        client.println("Host: limitless-headland-1164.herokuapp.com");
        //    client.println("GET /arduino HTTP/1.1");
        //    client.println("Host: mrlamroger.bol.ucla.edu");
        client.println("Connection: close");
        client.println();
        
        //Connected - Read the page
        return ethernetReadPage(); //go and read the output
        
        
    }else{
        hubSerial.println("connection failed");
    }
    
}

String SrSryrsHub::ethernetReadPage(){
    //read the page, and capture & return everything between '<' and '>'
    
    stringPos = 0;
    memset( &inString, 0, 32 ); //clear inString memory
    count = 0;
    boolean settingVariable = true;
    
    while(true){
        if (client.available()) {
            hubSerial.println("Client is available");
            while (char c = client.read()) {
                if (c == '>') {
                    client.stop();
                    client.flush();
                    hubSerial.println("disconnecting.");
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
                            //client.stop();
                            //client.flush();
                            //Serial.println("disconnecting.");
                            //return variable[count - 1];
                        } 
                    }
            }
        }
    }
}

void SrSryrsHub::ethernetScrapeWebsite()
{
    String pageValue = ethernetConnectAndRead();
    hubSerial.println(pageValue);
    for (int i = 0; i < count; i++)
    {
        hubSerial.println(variable[i]);
        hubSerial.println(value[i]);
    }
}
////////////////////////
/////     COSM     /////
////////////////////////
#ifdef USING_COSM
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
        hubSerial.println(cosmControlFeed[0].getInt());
//        cosmControlString = cosmControlFeed[0].getSTRING();
        cosmControlVal = cosmControlFeed[0].getInt();
    }
    
    hubSerial.println();
}
#endif //USING_COSM
/////////////////////////////////////
//////     Helper Functions     /////
/////////////////////////////////////
