#include "HubVars.h"
#include "KnowPlaceHub.h"

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

void XBeeNodeMessage::resetChangedPin(uint8_t pin)
{
    m_changed = (m_changed & ~(0x1<<pin));
}

void XBeeNodeMessage::resetChangedBitmap()
{
    m_changed = 0;
}

XBeeNodeMessage::XBeeNodeMessage() : m_xba64(XBeeAddress64(0,0))
{
    m_changed = 0;
    for (int i = 0; i < 16; i++) {
        m_type[i] = DATA_INT;
        m_data[i] = 0;
    }
}

void XBeeNodeMessage::setAddressH(uint32_t xbeeAddressH)
{
    m_xba64.setMsb(xbeeAddressH);
}

void XBeeNodeMessage::setAddressL(uint32_t xbeeAddressL)
{
    m_xba64.setLsb(xbeeAddressL);
}

void XBeeNodeMessage::setAddress(uint32_t xbeeAddressH, uint32_t xbeeAddressL)
{
    m_xba64.setMsb(xbeeAddressH);
    m_xba64.setLsb(xbeeAddressL);
}

void XBeeNodeMessage::setData(uint8_t pin, uint8_t type, uint8_t data)
{
    m_changed = m_changed | 1<<pin;
    m_type[pin] = type;
    m_data[pin] = data;
}

boolean XBeeNodeMessage::checkChanged(uint8_t pin)
{
    return (m_changed & (0x1 << pin));
}

uint16_t XBeeNodeMessage::getChangedBitmap()
{
    return m_changed;
}

uint32_t XBeeNodeMessage::getAddressH()
{
    return m_xba64.getMsb();
}

uint32_t XBeeNodeMessage::getAddressL()
{
    return m_xba64.getLsb();
}

XBeeAddress64 XBeeNodeMessage::getAddress()
{
    return m_xba64;
}

int XBeeNodeMessage::getData(uint8_t pin)
{
    if(pin < 0 || pin >= 16)
    {
        return -1;
    }
    else
    {
        return m_data[pin];
    }
}

int XBeeNodeMessage::getDataType(uint8_t pin)
{
    if(pin < 0 || pin >= 16)
    {
        return -1;
    }
    else
    {
        return m_type[pin];
    }
}

KnowPlaceHub::KnowPlaceHub()
		      :hubSerial(Serial)
#ifdef USING_LCD
            ,lcd(LCD_RS, LCD_RW, LCD_Enable, LCD_D4, LCD_D5, LCD_D6, LCD_D7)
#endif //USING_LCD
            ,xbee(XBee())
            ,ssXbee(SoftwareSerial(XBEE_ADD_RX, XBEE_ADD_TX))
            ,xbeeSS(XBee(ssXbee))
            ,atRequest(NULL, NULL, sizeof(NULL))
            ,atResponse(AtCommandResponse())
            ,remoteAtRequest(RemoteAtCommandRequest(xba64, NULL, NULL, sizeof(NULL)))
            ,remoteAtResponse(RemoteAtCommandResponse())
//            ,zbTx(xba64, NULL, sizeof(NULL))
//            ,txStatus(ZBTxStatusResponse())
            ,ioSample(ZBRxIoSampleResponse())
			,ip(IP0, IP1, IP2, IP3)
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
    hubApiKey = HUB_API_KEY;
    ////////////////////////
    //////     LCD     /////
    ////////////////////////


    ////////////////////////
    /////     XBee     /////
    ////////////////////////
    xbeeAddPin = XBEE_ADD_PIN;
    
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
    strcpy(server,"limitless-headland-1164.herokuapp.com" );
    location = "/testlamp HTTP/1.0";
    
    
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

void KnowPlaceHub::init()
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
#ifdef USING_LCD
    lcd.begin(LCD_ROWS, LCD_COLS);
#endif //USING_LCD
    
    //XBee
    ssXbee.begin(XBEE_BAUD); //don't need to begin the xbeeSS because of 
    xbee.begin(XBEE_BAUD);
    
    pinMode(xbeeAddPin,INPUT); //as an interrupt, it doesn't need pinModed-ed
//    attachInterrupt(xbeeAddPin, addNodeToWeb, FALLING); //doesn't like member function
    
    //xbee.setSerial(); //here just so you know it's an option
    
//Ethernet

//    if (Ethernet.begin(mac) == 0) {
//        hubSerial.println("Failed to configure Ethernet using DHCP");
//        // DHCP failed, so use a fixed IP address:
//        Ethernet.begin(mac, ip);
//    }

}
////////////////////////
//////     LCD     /////
////////////////////////
#ifdef USING_LCD
//prints leading zeros of four digit analog values
//should only be used for 4 digits (assumed max of 1023)
//could be made more general
//maybe there's already an lcd print function that prints leading zeros
void KnowPlaceHub::lcdPrintAnalog(int analog)
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
#endif //USING_LCD
////////////////////////
/////     XBee     /////
////////////////////////


void KnowPlaceHub::xbeeSetAtCommand(uint8_t *cmd)
{
    atRequest.clearCommandValue();
    atRequest.setCommand(cmd);
}

void KnowPlaceHub::xbeeSetAtCommand(uint8_t *cmd, uint8_t *value, uint8_t valueLength)
{
    atRequest.clearCommandValue();
    atRequest.setCommand(cmd);
    atRequest.setCommandValue(value);
    atRequest.setCommandValueLength(valueLength);
}

void KnowPlaceHub::xbeeSendAtCommand(XBee &xbee)
{
//    hubSerial.println("Sending command to the XBee");
    
    // send the command
    xbee.send(atRequest);
}

boolean KnowPlaceHub::xbeeReceiveAtResponse(XBee &xbee)
{
    boolean ret = false;
    
    // wait up to 5 seconds for the status response
    if (xbee.readPacket(1000)) {
        // got a response!
        
        // should be an AT command response
        if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
            xbee.getResponse().getAtCommandResponse(atResponse);
            
            if (atResponse.isOk()) {
//                 hubSerial.print("Command [");
//                 hubSerial.print(atResponse.getCommand()[0]);
//                 hubSerial.print(atResponse.getCommand()[1]);
//                 hubSerial.println("] was successful!");
                
                if (atResponse.getValueLength() > 0) {
//                     hubSerial.print("Command value length is ");
//                     hubSerial.println(atResponse.getValueLength(), DEC);
                    
//                     hubSerial.print("Command value: ");
                    
                    for (int i = 0; i < atResponse.getValueLength(); i++) {
//                         hubSerial.print(atResponse.getValue()[i], HEX);
//                         hubSerial.print(" ");
                    }
                    
//                     hubSerial.println("");
                }
                ret = true;
            }
            else {
//                 hubSerial.print("Command return error code: ");
                 //hubSerial.println(atResponse.getStatus(), HEX);
            }
        } else {
//             hubSerial.print("Expected AT response but got ");
             //hubSerial.print(xbee.getResponse().getApiId(), HEX);
        }
    } else {
        // at command failed
        if (xbee.getResponse().isError()) {
//             hubSerial.print("Error reading packet.  Error code: ");
             //hubSerial.println(xbee.getResponse().getErrorCode());
        }
        else {
//             hubSerial.println("No response from radio");
        }
    }
    return ret;
}
/*
boolean KnowPlaceHub::xbeeGetCoordinatorPanID(uint8_t *idVal, uint8_t valSize)
{
    //the panID will be stored in the atResponse variable
    boolean ret = false;
    uint8_t idCmd[2] = {'I', 'D'}; //get panID command
    int dataOffset = 6;
    if(valSize < 4)
    {
//        hubSerial.println("panID must be at least 4 characters");
    }
    xbeeSetAtCommand(idCmd);
    xbeeSendAtCommand(xbee);
    
    if (xbeeReceiveAtResponse(xbee))
    {
        hubSerial.println("Coordinator panID: ");
        for (int i = dataOffset; i < atResponse.getValueLength() &&
                                 i < valSize + dataOffset; i++) {
            idVal[i] = atResponse.getValue()[i];
            hubSerial.print(atResponse.getValue()[i],HEX);
        }
        hubSerial.println();
        ret = true;
    }
    return ret;
}

boolean KnowPlaceHub::xbeeSetNodePanID(uint8_t *idVal, uint8_t valSize)
{
    //todo make this more intuitive
    //passing 4s and 5s doens't make sense
    //maybe limit this to 0s and 1s where 0 is off and 1 is on
    //limit pin set 2 for pins 0-3
    
    boolean ret = false;
    uint8_t idCmd[2] = {'I', 'D'}; //IS command requests IO sample
//    hubSerial.println("Attempting to set new node's panID");
    
    if(valSize >= 2)
    {
        xbeeSetAtCommand(idCmd,idVal, valSize);
        xbeeSendAtCommand(xbeeSS);
        if (xbeeReceiveAtResponse(xbeeSS))
        {
            //just checking
            hubSerial.println("Verifying panID:");
            xbeeSetAtCommand(idCmd);
            xbeeSendAtCommand(xbeeSS);
            xbeeReceiveAtResponse(xbeeSS);
            ret = true;
        }
        else
        {
            hubSerial.println("Failed to set PanID on new node");
        }
    }
    else
    {
        hubSerial.println("panVal must have at least 4 characters");
    }
    return ret;
}
*/
void KnowPlaceHub::storeXBeeAddress64(uint8_t addrH[4], uint8_t addrL[4])
{
    uint32_t halfAddr = addrH[0];
    for (int i = 1; i < 4; i++) {
        halfAddr =  (halfAddr<<8) | addrH[i];
    }
    xba64.setMsb(halfAddr);
//    hubSerial.print("Stored addr64Msb:");
//    hubSerial.print(xba64.getMsb()>>16 & 0xFFFF,HEX);
//    hubSerial.print(xba64.getMsb() & 0xFFFF,HEX);
//    hubSerial.println();
    
    halfAddr = addrL[0];
    for (int i = 1; i < 4; i++) {
        halfAddr = (halfAddr<<8) | addrL[i];
    }
    xba64.setLsb(halfAddr);
//    hubSerial.print("Stored addr64Lsb:");
//    hubSerial.print(xba64.getLsb()>>16 & 0xFFFF,HEX);
//    hubSerial.print(xba64.getLsb() & 0xFFFF,HEX);
//    hubSerial.println();
}

void KnowPlaceHub::addNodeToWeb()
{
    digitalWrite(internalLed, HIGH);
    delay(300);
    digitalWrite(internalLed, LOW);
    if(digitalRead(xbeeAddPin) == LOW)
    {
        uint8_t idCmd[2] = {'I', 'D'}; //get panID command
        uint8_t panID[] = {0,0,0,0,0,0,0,0};
        int idSize = sizeof(panID);
        //PANID set section
        
        //get coordinator panID section
        xbeeSetAtCommand(idCmd);
        xbeeSendAtCommand(xbee);
        boolean gotCoordID = xbeeReceiveAtResponse(xbee);
        
        if(gotCoordID && atResponse.getValueLength() == idSize)
        {
            for (int i = 0; i < atResponse.getValueLength(); i++) {
                panID[i] = atResponse.getValue()[i];
            }
            //set new node panID section
            delay(5000); //wait for radio to initialize
            xbeeSetAtCommand(idCmd,panID, idSize);
            xbeeSendAtCommand(xbeeSS);
            if (xbeeReceiveAtResponse(xbeeSS))
            {
                //just checking
                //                hubSerial.println("Verifying panID:");
                //                xbeeSetAtCommand(idCmd);
                //                xbeeSendAtCommand(xbeeSS);
                //                xbeeReceiveAtResponse(xbeeSS);
                hubSerial.println("PanID set on new node!");
                while(digitalRead(xbeeAddPin) == LOW);
            }
        }
        
        //64bit new node address seciton
        uint8_t shCmd[2] = {'S','H'};
        uint8_t slCmd[2] = {'S','L'};
        uint8_t nodeSH[] = {0,0,0,0};
        uint8_t nodeSL[] = {0,0,0,0};
        xbeeSetAtCommand(shCmd);
        xbeeSendAtCommand(xbeeSS);
        xbeeReceiveAtResponse(xbeeSS);
        for (int i = 0; i < atResponse.getValueLength() && i < 4; i++) {
            nodeSH[i] = atResponse.getValue()[i];
        }
        
        xbeeSetAtCommand(slCmd);
        xbeeSendAtCommand(xbeeSS);
        xbeeReceiveAtResponse(xbeeSS);
        for (int i = 0; i < atResponse.getValueLength() && i < 4; i++) {
            nodeSL[i] = atResponse.getValue()[i];
        }
        
        //verification or submit to website
        hubSerial.println("New Node PanID");
        for(int i = 0; i < 8; i++)
        {
            if(panID[i] < 16)
            {
                hubSerial.print("0");
                if(panID[i] == 0)
                    hubSerial.print("0");
            }
            else
            {
                hubSerial.print(panID[i],HEX);
            }
        }
        hubSerial.println();
        hubSerial.println("New Node Address:");
        for(int i = 0; i < 4; i++)
        {
            if(nodeSH[i] < 16)
            {
                hubSerial.print("0");
                if(nodeSH[i] == 0)
                    hubSerial.print("0");
            }
            else
            {
                hubSerial.print(nodeSH[i],HEX);
            }
        }
        hubSerial.print(" ");
        for(int i = 0; i < 4; i++)
        {
            if(nodeSL[i] < 16)
            {
                hubSerial.print("0");
                if(nodeSL[i] == 0)
                    hubSerial.print("0");
            }
            else
            {
                hubSerial.print(nodeSL[i],HEX);
            }
        }
        hubSerial.println();
        
        //post to KnowPlace
        storeXBeeAddress64(nodeSH, nodeSL);
        ethernetPostNewNodeAddress(xba64);
    }
    else
    {
        hubSerial.println("Button Not Pressed");
    }
    
}


//REMOTE_AT_COMMAND

void KnowPlaceHub::xbeeSetRemoteAtCommand(uint8_t *cmd)
{
    remoteAtRequest.clearCommandValue();
    remoteAtRequest.setCommand(cmd);
}

void KnowPlaceHub::xbeeSetRemoteAtCommand(uint8_t *cmd, uint8_t *value, uint8_t valueLength)
{
    remoteAtRequest.clearCommandValue();
    remoteAtRequest.setCommand(cmd);
    remoteAtRequest.setCommandValue(value);
    remoteAtRequest.setCommandValueLength(valueLength);
}

void KnowPlaceHub::xbeeSendRemoteAtCommand() {
    hubSerial.println("Sending command to the XBee");

    // send the command
    xbee.send(remoteAtRequest);
    //todo check for command sent
}


//for the cases where the response is just a confirmation, no data included
void KnowPlaceHub::xbeeReceiveRemoteAtResponse() {
    int tempVar = 0;
    xbeeReceiveRemoteAtResponse(tempVar);
}

//for use when issuing and IS remoteAtCommand and data is sent back as a reponse
void KnowPlaceHub::xbeeReceiveRemoteAtResponse(int &ioData) {
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
             //hubSerial.print("Received I/O Sample from: ");
            
             //hubSerial.print(remoteAtResponse.getRemoteAddress64().getMsb(), HEX);
             //hubSerial.print("  ");
             //hubSerial.println(remoteAtResponse.getRemoteAddress64().getLsb(), HEX);
            //end ryan's test
            
            if (remoteAtResponse.isOk()) {
                 //hubSerial.print("Command [");
                 //hubSerial.print(remoteAtResponse.getCommand()[0]);
                 //hubSerial.print(remoteAtResponse.getCommand()[1]);
                 //hubSerial.println("] was successful!");
                
                if (remoteAtResponse.getValueLength() > 0) {
                     //hubSerial.print("Command value length is ");
                     //hubSerial.println(remoteAtResponse.getValueLength(), DEC);
                    
                     //hubSerial.print("Command value: ");
                    
                    for (int i = 0; i < remoteAtResponse.getValueLength(); i++) {
                         //hubSerial.print(remoteAtResponse.getValue()[i], HEX);
                         //hubSerial.print(" ");
                    }
                    
                    int analogHigh = remoteAtResponse.getValue()[4] ;
                    int analogLow = remoteAtResponse.getValue()[5] ;
                    ioData = (analogHigh<<8) + analogLow;
                    
                     //hubSerial.print("analogValue: ");
                     //hubSerial.print(ioData);
                    
                     //hubSerial.println("");
                }
            } else {
                 //hubSerial.print("Command returned error code: ");
                 //hubSerial.println(remoteAtResponse.getStatus(), HEX);
            }
        } else {
             //hubSerial.print("Expected Remote AT response but got ");
             //hubSerial.print(xbee.getResponse().getApiId(), HEX);
        }
    } else {
        // remote at command failed
        if (xbee.getResponse().isError()) {
             //hubSerial.print("Error reading packet.  Error code: ");  
             //hubSerial.println(xbee.getResponse().getErrorCode());
        } else {
             //hubSerial.print("No response from radio");  
        }
    }
}




/*
 boolean KnowPlaceHub::xbeeReceiveIOData(XBeeIOData &ioData) //todo iodata
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

void KnowPlaceHub::xbeeForceSampleRequest(XBeeAddress64 &remoteAddress, int &ioData)//todo iodata
{
    //todo check to see if pass by reference is fine, or if it should be a pointer
    //pass by reference is readable
    remoteAtRequest.setRemoteAddress64(remoteAddress);
    uint8_t isCmd[2] = {'I', 'S'}; //IS command requests IO sample
    xbeeSetRemoteAtCommand(isCmd);
    xbeeSendRemoteAtCommand();
    xbeeReceiveRemoteAtResponse(ioData);
}

void KnowPlaceHub::xbeeControlRemotePins(XBeeAddress64 &remoteAddress, int &ioData)
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
        xbeeSetRemoteAtCommand((uint8_t *)dCmd, (uint8_t *)dVal, sizeof(dVal));
        xbeeSendRemoteAtCommand();
        xbeeReceiveRemoteAtResponse();
    }
}


boolean KnowPlaceHub::xbeePwmTxRequest(XBeeAddress64 &remoteAddress, uint16_t pwmVal)
{
    uint8_t payload[] = {0,0};
    
    ZBTxRequest zbTx = ZBTxRequest(remoteAddress, payload, sizeof(payload));
    ZBTxStatusResponse txStatus = ZBTxStatusResponse();
    
    payload[0] = pwmVal >> 8 & 0xff;
    payload[1] = pwmVal & 0xff;
    
    xbee.send(zbTx);
    
    // flash TX indicator
//    flashLed(internalLed, 1, 100);
    
    // after sending a tx request, we expect a status response
    // wait up to half second for the status response
    if (xbee.readPacket(500)) {
        // got a response!
        
        // should be a znet tx status
    	if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
            xbee.getResponse().getZBTxStatusResponse(txStatus);
    		
            // get the delivery status, the fifth byte
            if (txStatus.getDeliveryStatus() == SUCCESS) {
            	// success.  time to celebrate
//             	flashLed(internalLed, 5, 50);
                return true;
            }
//            else
//            {
//            	// the remote XBee did not receive our packet. is it powered on?
//             	flashLed(internalLed, 3, 500);
//            }
        }
    }
//    else
//    {
//        // local XBee did not provide a timely TX Status Response -- should not happen
//        flashLed(internalLed, 2, 50);
//    }
    return false;
    
}
////////////////////////////
/////     Ethernet     /////
////////////////////////////
boolean KnowPlaceHub::ethernetConnect()
{
    //connect to the server
    //Initialize client
    client.stop();
    client.flush();
    Serial.println("connecting...");
    //port 80 is typical of a www page
    return client.connect(server, 80);
}

void KnowPlaceHub::ethernetDisconnect()
{
    client.println("Connection: close");
    client.println();
}

void KnowPlaceHub::ethernetClientPrintAddress64(XBeeAddress64 node_address)
{
    client.print("?node_address_high=");
    client.print(node_address.getMsb()>>16 & 0xFFFF, HEX); //print HEX is base 16, not 32
    client.print(node_address.getMsb() & 0xFFFF,HEX);
    client.print("?node_address_low=");
    client.print(node_address.getLsb()>>16 & 0xFFFF,HEX);
    client.print(node_address.getLsb() & 0xFFFF,HEX);
}

boolean KnowPlaceHub::ethernetGetRequest()
{
#ifdef USING_PROCESSING_AS_INTERNET
    hubSerial.println("<");
    hubSerial.print("GET /mydata?action=");
    hubSerial.print(">");
    //Connected - Read the page
    return ethernetReadPage();
#else
    if (ethernetConnect())
    {
        hubSerial.println("connected");
        client.print("GET /testlamp?node_address=");
        client.print(String(node_address));
        client.println(" HTTP/1.1");
        client.println("Host: limitless-headland-1164.herokuapp.com");
        //    client.println("GET /arduino HTTP/1.1");
        //    client.println("Host: mrlamroger.bol.ucla.edu");
        ethernetDisconnect();
        //Connected - Read the page
        return ethernetReadPage(); //go and read the output
    }else{
        hubSerial.println("connection failed");
    }
#endif
}

boolean KnowPlaceHub::ethernetPostSensorData(/*XBeeAddress64*/int node_address, /*uint32_t*/int data)
{
    boolean ret = false;
#ifdef USING_PROCESSING_AS_INTERNET
    //connect with starting character '<'
    hubSerial.println("<");
    hubSerial.print("POST /testlamp");
    hubSerial.print("?node_address=");
    hubSerial.print(String(node_address));
    hubSerial.print("&data_value=");
    hubSerial.print(String(data));
    //disconnect with ending character '>'
    hubSerial.print(">");
#else
    if (ethernetConnect())
    {
        hubSerial.println("connected");
        client.print("POST /testlamp");
        client.print("?node_address=");
        client.print(String(node_address));
        client.print("&data_value=");
        client.print(String(data));
        client.println(" HTTP/1.1");
        client.println("Host: limitless-headland-1164.herokuapp.com");
        //    client.println("GET /arduino HTTP/1.1");
        //    client.println("Host: mrlamroger.bol.ucla.edu");
        ethernetDisconnect();
        
        //Verify data was successfuly posted.
        ethernetScrapeWebsite(node_address);
        if (value[0].toInt() == data)
        {
            hubSerial.println("Successfully Posted");
            ret= true;
        }
        else{
            hubSerial.println("Post Failed"); //go and read the output
        }
        
    }else{
        hubSerial.println("connection failed");
    }
#endif //USING_PROCESSING_AS_INTERNET
    return ret;
}

boolean KnowPlaceHub::ethernetPostNewNodeAddress(XBeeAddress64 & node_address)
{
    boolean ret = false;
    if (ethernetConnect())
    {
        hubSerial.println("connected");
        
        client.print("POST /testlamp");
        client.print("hub_api_key=");
        client.print(String(hubApiKey)); //a member variable of the hub class
        ethernetClientPrintAddress64(node_address);
        client.println(" HTTP/1.1");
        client.println("Host: limitless-headland-1164.herokuapp.com");

        ethernetDisconnect();
        
        //todo: figure out specific implementation for adding
        //Verify data was successfuly posted.
//        ethernetScrapeWebsite(node_address);
//        if (value[0].toInt() == data)
//        {
//            hubSerial.println("Successfully Posted");
//            ret = true;
//        }
//        else
//        {
//            hubSerial.println("Post Failed"); //go and read the output
//        }
        ret = true;
    }
    else
    {
        hubSerial.println("connection failed");
    }
    return ret;
}

void ethernetPostDataJson(XBeeNodeMessage &dataPacket) 
{
    boolean ret = false;
#ifdef USING_PROCESSING_AS_INTERNET
    //connect with starting character '<'
    hubSerial.println("<");
    hubSerial.print("POST /testlamp");
    hubSerial.print("?nodeAddressH=");
    hubSerial.print(String(dataPacket.getAddressH()));
    hubSerial.print("&nodeAddressL=");
    hubSerial.print(String(dataPacket.getAddressH()));
    hubSerial.print("&type=");
    hubSerial.print(String(dataPacket.getDataType(0));
    hubSerial.print("&data=");
    hubSerial.print(String(dataPacket.getData(0));

    //disconnect with ending character '>'
    hubSerial.print(">");
#else
    if (ethernetConnect())
    {
        hubSerial.println("connected");
        client.print("POST /testlamp");
        client.print("?nodeAddressH=");
        client.print(String(dataPacket.getAddressH()));
        client.print("&nodeAddressL=");
        client.print(String(dataPacket.getAddressH()));
        client.print("&type=");
        client.print(String(dataPacket.getDataType(0));
        client.print("&data=");
        client.print(String(dataPacket.getData(0));
        client.println(" HTTP/1.1");
        client.println("Host: limitless-headland-1164.herokuapp.com");
        //    client.println("GET /arduino HTTP/1.1");
        //    client.println("Host: mrlamroger.bol.ucla.edu");
        ethernetDisconnect();
        
        //Verify data was successfuly posted.
        ethernetScrapeWebsite(node_address);
        if (value[0].toInt() == data)
        {
            hubSerial.println("Successfully Posted");
            ret= true;
        }
        else{
            hubSerial.println("Post Failed"); //go and read the output
        }
        
    }else{
        hubSerial.println("connection failed");
    }
#endif //USING_PROCESSING_AS_INTERNET
    return ret;

}

boolean KnowPlaceHub::ethernetReadPage(){
    //read the page, and capture & return everything between '[' and ']'
    
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
                    hubSerial.println(inString);
                    hubSerial.println("disconnecting.");
                    return false;
                } else
                    
                    if (c == '[' ) { //'[' is our begining character
                        startRead = true; //Ready to start reading the part
                        memset( &inString, 0, 32 );
                        stringPos = 0;
                    } else if(startRead){
                        if(c != ']'){ //']' is our ending character
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
        } else {
            hubSerial.println("Client is not available");
            delay(5000);
        }
    }
}

boolean KnowPlaceHub::ethernetReadPageJson(XBeeNodeMessage &dataPacket){
    //read the page, and capture & return everything between '[' and ']'
    
    char jsonString[64];
    char* jsonParam;
    token_list_t *tokenList = NULL;
    
    boolean startRead = false;
    uint8_t stringPos = 0;
    
//    uint32_t addressH = 0;
//    uint32_t addressL = 0;
    uint8_t tempPin = 0;
    int tempType = -1;
    uint8_t tempData = 0;

#ifdef USING_PROCESSING_AS_INTERNET
    while(!hubSerial.available()){}
    hubSerial.println("Client is available");
    while (char c = hubSerial.read()) {
        if(startRead)
        {
            if(c != '}')
            {
                jsonString[stringPos++] = c;
                break;
            }
            else
            {
                //Serial.print(c);
                jsonString[stringPos++] = c;
                jsonString[stringPos++] = '\0';
            }
        }
        else if (c == '{' )
        { //'{' is our begining character
            startRead = true; //Ready to start reading the part
            memset( jsonString, 0, 32 );
            stringPos = 0;
            jsonString[stringPos++] = c;
        }
        else if (c == '>') {
            return false;
        }
        else
        {
            continue; //do nothing
        }
    }



#else
    while(true){
        if (client.available()) {
            hubSerial.println("Client is available");
            while (char c = client.read()) {
                if(startRead)
                {
                    if(c != '}')
                    {
                        jsonString[stringPos++] = c;
                        break;
                    }
                    else
                    {
                        //Serial.print(c);
                        jsonString[stringPos++] = c;
                        jsonString[stringPos++] = '\0';
                    }
                }
                else if (c == '{' )
                { //'{' is our begining character
                    startRead = true; //Ready to start reading the part
                    memset( jsonString, 0, 32 );
                    stringPos = 0;
                    jsonString[stringPos++] = c;
                }
                else if (c == '>') {
                    client.stop();
                    client.flush();
                    hubSerial.println(inString);
                    hubSerial.println("disconnecting.");
                    return false;
                }
                else
                {
                    continue; //do nothing
                }
            }
        } else {
            hubSerial.println("Client is not available");
            delay(5000);
        }
    }
#endif //USING_PROCESSING_AS_INTERNET
    
    //process the json object
    tokenList = create_token_list(10);//I don't know what this number is yet
    json_to_token_list(jsonString, tokenList);
    
    jsonParam = json_get_value(tokenList,"nodeAddressH");
    dataPacket.setAddressH(strtoul(jsonParam, NULL, 0)); //store the address
    
    jsonParam = json_get_value(tokenList,"nodeAddressL");
    dataPacket.setAddressL(strtoul(jsonParam, NULL, 0)); //store the address
    
    jsonParam = json_get_value(tokenList,"pinNumber");
    tempPin = atoi(jsonParam);
    
    if (jsonParam == "binary") {
        tempType = DATA_BINARY;
    }
    else if (jsonParam == "integer") {
        tempType = DATA_INT;
    }
    jsonParam = json_get_value(tokenList,"dataValue");
    tempData = atoi(jsonParam);

    dataPacket.setData(tempPin, tempType, tempData); //store the data
    
}

void KnowPlaceHub::ethernetScrapeWebsite()
{
    if(ethernetGetRequest())
    {
        while (ethernetReadPageJson(nodeMessage))
        {
            //send out xbee command
        }
    }
    else
    {
        //thing of something
    }
}

//<<<<<<< HEAD
//=======
//>>>>>>> cf98246ed2ae4876dfaecc6ad4a9f6a5418e0afd
int KnowPlaceHub::getDeviceStatus(/*XBeeAddress64*/int node_address)
{
    int numDevices = count % 3; //Count is the number of entries.
    //Currently there are three variables per device
    //lampStatus, lampStatusTime, lampAddress
//    int i;
//    for(i = 0; i < numDevices; i++ )
//    {
//        if(value[i].toInt() == node_address)
//        {
//            return (value[ 0 + i*numDevices]).toInt();
//        }
//    }
    if (value[2] == node_address)
    {
        return (value[0]).toInt();
    }
    
    return -1; // Did not find device with id
    
}

////////////////////////
/////     COSM     /////
////////////////////////
#ifdef USING_COSM
// this method makes a HTTP connection to the cosmServer:
void KnowPlaceHub::cosmSendData(int thisData)
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

void KnowPlaceHub::cosmRequestData()
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
