/*
 CAN-logger for EM150
 Nano pin config:
 4 - Red LED
 5 - Green LED
 6 - Blue LED
 7 - CAN RX enable switch
 8 - CAN logger button
 9 - MCP CS pin
 10 - SD CS pin
 11 - SPI MOSI pin
 12 - SPI MISO pin
 13 - SPI CLK pin


//Memory use
//1560

* Libraries
* Global variables
*
*/

#include <SdFat.h>                              // https://github.com/greiman/SdFat/
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>                             // library from https://github.com/adafruit/RTClib    
#include <mcp_can.h>
#include <SPI.h>
#include <string.h>


#define RX_FAILS        1000                    // How long to wait (ms) for CAN msg before failing
#define TX_FAILS        15                      // How many failed attempts of CAN msg sends before failing
#define R_LED           4                       // Red status LED
#define G_LED           5                       // Green status LED
#define B_LED           6                       // Blue status LED
#define YELLOW          0b00000011              // Yellow status color
#define RED             0b00000001              // Red status color
#define GREEN           0b00000010              // Green status color
#define BLUE            0b00000100              // Blue status color
#define LED_OFF         0x00                    // Turn off LED
#define DEF_LOG_TIME    100                     // ms between log samples

// Fail flags
#define FAIL_MCP        0x01                    // Failflag for MCP
#define FAIL_SD         0x02                    // Failflag for SD
#define FAIL_RTC        0x04                    // Failflag for RTC
#define FAIL_CRX        0x10                    // Failflag for CAN RX
#define FAIL_CTX        0x20                    // Failflag for CAN TX

// CAN paramters
#define CAN_CS_PIN      9                       // Set CS to pin 10
#define CAN0_INT        2                       // Set INT to pin 2
#define CAN_SEND_ID     0x1026105A              // id for display CAN send data
#define CAN_SEND_DATA   {0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00}
#define CAN_LOGGER_BTN  8                       // Toggle switch for can logger
#define SEND_EN         7                       // Pin nr to enable LCD CAN emulator
#define SEND_INTERVAL   250                     // interval to send CAN data to controller

// uSDcard parameters
#define MOSIpin         11                       // For SD card
#define MISOpin         12                       // For SD card
#define SD_CS_PIN       10                       // CS pin for the SD card

// RTC paramters
#define DS1307_I2C_ADDRESS 0x68

// Create objects
MCP_CAN CAN0(CAN_CS_PIN);                       // CAN object with CS pin
SdFat SD;                                       // The SdFat object is "SD"
RTC_DS1307 RTC;                                 // The real time clock object is "RTC"

// CAN related variables
byte failSends = 0;
byte txFails = TX_FAILS;
int rxFailTime = RX_FAILS;
unsigned long last_sent_time = 0;
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

// RTC variables
char dateString[] = "YYYY-MM-DD";               // a template for date string
char timeString[] = "hh:mm:ss";                 // a template for time string
char filename[] = "YYYYMMDD_hhmmss.log";        // template for filename
long utc;

// General variables
byte msgToRead = 0;             // 0=logging off, 1=read first msg, 2=log second message, 3=wait for next cycle
unsigned long lastLog = 0;
unsigned long logMillis = DEF_LOG_TIME;
char delimiter = ',';
byte com_code = 'X';
byte failFlags = 0;             // fail flags: 8:- 7:- [6:CAN-TX][5:CAN-RX] 4:-[3:RTC][2:SD][1:MCP]
byte lastFailFlag = 0;
byte ledStatus = 0;
bool btnReleased = true;
int btnCooldown = 1000;
unsigned long btnTimer = 0;

char dateStr[] = "date: ";
char timeStr[] = "time: ";
char logtStr[] = "LogT: ";
char flagStr[] = "EFlags: ";
char canTxStr[] = "CAN TX: ";
char canRxStr[] = "CAN RX: ";
char failStr[] = " fail";
