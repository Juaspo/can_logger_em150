// CAN-logger for EM150
//1680

#include <SdFat.h>                              // https://github.com/greiman/SdFat/
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>                             // library from https://github.com/adafruit/RTClib    
#include <mcp_can.h>
#include <SPI.h>
#include <string.h>

#define R_LED           4                       // Red status LED
#define G_LED           5                       // Green status LED
#define B_LED           6                       // Blue status LED
#define YELLOW          0b00000110              // Yellow status color
#define RED             0b00000001              // Red status color
#define GREEN           0b00000010              // Green status color
#define BLUE            0b00000100              // Blue status color

#define FAIL_MCP        0x01                    // Failflag for MCP
#define FAIL_SD         0x02                    // Failflag for SD
#define FAIL_RTC        0x04                    // Failflag for RTC
#define FAIL_CRX        0x10                    // Failflag for CAN RX
#define FAIL_CTX        0x20                    // Failflag for CAN TX

//CAN paramters
#define CAN_CS_PIN      9                      // Set CS to pin 10
#define CAN0_INT        2                       // Set INT to pin 2
#define CAN_SEND_ID     0x1026105A              // id for display CAN send data
#define CAN_SEND_DATA   {0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00}
#define SEND_EN         5                       // Pin nr to enable LCD CAN emulator
#define SEND_INTERVAL   250                     // interval to send CAN data to controller

//SDcard parameters
#define MOSIpin         11                       // For SD card
#define MISOpin         12                       // For SD card
#define SD_CS_PIN       10                        // CS pin for the SD card

//RTC paramters
#define DS1307_I2C_ADDRESS 0x68

//Create objects
MCP_CAN CAN0(CAN_CS_PIN);                       // CAN object with CS pin
SdFat SD;                                       // The SdFat object is "SD"
RTC_DS1307 RTC;                                 // The real time clock object is "RTC"

//CAN variables
byte failSends = 0;
byte failAttempts = 25;
unsigned long last_sent_time = 0;
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                            // Array to store serial string

//RTC variables
char dateString[ ] = "0000-00-00";              // a template for date string
char timeString[ ] = "00:00:00";                // a template for time string
char filename[ ] = "0000-00-00_00_00.log";      // template for filename
long utc;


//General variables
byte msgToRead = 0;

bool loggingData = true;
unsigned long lastLog = 0;
bool newFile = true;
unsigned long logMillis = 1000;
char delimiter = ',';
byte com_code = 0;
byte failFlags = 0;           // fail flags: 8:- 7:- [6:CAN-TX][5:CAN-RX] 4:-[3:RTC][2:SD][1:MCP]
byte firstRun = true;


void fail_flag_check(){
    if (failFlags & 0x0F){
        digitalWrite(R_LED, HIGH);
        Serial.print("Fatal err: ");
        Serial.println(failFlags);
        while(1){
            delay(500);
            digitalWrite(R_LED, LOW);
            delay(500);
            digitalWrite(R_LED, HIGH);
        } 
    }
    else if(failFlags & 0xF0){
        digitalWrite(R_LED, HIGH);
        Serial.print("Err: ");
        Serial.println(failFlags);
    }
}

void setup(){
    Serial.begin(115200);

    pinMode(R_LED, OUTPUT);
    pinMode(G_LED, OUTPUT);
    pinMode(B_LED, OUTPUT);
    
    // Initialize MCP2515 running at 8MHz with a baudrate of 250kb/s
    if(CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK)
        Serial.println("MCP OK");
    else{
        Serial.println("MCP Error");
        failFlags = failFlags | FAIL_MCP;
    }

    CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
    //CAN0.setMode(MCP_LISTENONLY);
    pinMode(CAN0_INT, INPUT);                      // Configuring pin for /INT input
    //CAN Mask and filters
    /*
    CAN0.init_Mask(0,1,0xFFFFFFF0);                // Init first mask...
    CAN0.init_Filt(0,1,0x10261022);                // Init first filter...
    CAN0.init_Filt(1,1,0x10261023);                // Init second filter...
    */
    Wire.begin();
    if (! RTC.begin()) {
        Serial.println("RTC Error");
        Serial.flush();
        failFlags = failFlags | FAIL_RTC;
    }
    while (!Serial) {;}                            // wait for serial port to connect. Needed for native USB port only

    //SD card init
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Card failed");
        failFlags = failFlags | FAIL_SD;
    }else Serial.println(" SD card OK");

    Serial.print("Log interval ");
    Serial.println(logMillis);

    File dataFile = SD.open("test.txt", FILE_WRITE);
    if (dataFile) {                                 // if the file is available, write to it:
        dataFile.flush();
        dataFile.close();
    }
    else {
        Serial.println("file error");               // if the file is not open, display an error:
    }
    digitalWrite(G_LED, HIGH);
    delay(1000);
    //digitalWrite(R_LED, HIGH);
    //delay(1000);
    Serial.println("Ready");
}
