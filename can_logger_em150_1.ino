/* Setup code
 *
 *
 *
 */
 
void setup(){
    Serial.begin(9600);

    pinMode(R_LED, OUTPUT);
    pinMode(G_LED, OUTPUT);
    pinMode(B_LED, OUTPUT);
    
    // Initialize MCP2515 running at 8MHz with a baudrate of 250kb/s
    if(CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK)
        Serial.println(F("MCP OK"));
    else{
        Serial.print(F("MCP"));
        Serial.println(failStr);
        failFlags = failFlags | FAIL_MCP;
    }

    pinMode(CAN0_INT, INPUT);                     // Configuring pin for /INT input
    CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
    //CAN0.setMode(MCP_LISTENONLY);

    //CAN Mask and filters
    /*
    CAN0.init_Mask(0,1,0xFFFFFFF0);                // Init first mask...
    CAN0.init_Filt(0,1,0x10261022);                // Init first filter...
    CAN0.init_Filt(1,1,0x10261023);                // Init second filter...
    */
    Wire.begin();
    if (! RTC.begin()) {
        Serial.print(F("RTC"));
        Serial.println(failStr);
        Serial.flush();
        failFlags = failFlags | FAIL_RTC;
    }else Serial.println(F("RTC OK"));
    while (!Serial) {;}                            // wait for serial port to connect. Needed for native USB port only

    //SD card init
    if (!SD.begin(SD_CS_PIN)) {
        Serial.print(F("SD"));
        Serial.println(failStr);
        failFlags = failFlags | FAIL_SD;
    }else Serial.println(F("SD OK"));

    Serial.print(logtStr);
    Serial.println(logMillis);

    File dataFile = SD.open("delete.me", FILE_WRITE);
    if (dataFile) {                                 // if the file is available, write to it:
        dataFile.flush();
        dataFile.close();
    }
    else {
        Serial.print(F("File"));                       // if the file is not open, display an error:
        Serial.println(failStr);
        failFlags = failFlags | FAIL_SDRW;
    }
    status_update();                                // update error flags and status LED
    get_date_time();
    Serial.print(dateStr);
    Serial.println(dateString);
    Serial.print(timeStr);
    Serial.println(timeString);
    Serial.print(F("Ready "));
    Serial.println(failFlags);
}
