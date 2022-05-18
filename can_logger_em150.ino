// CAN-logger for EM150
//

#include <mcp_can.h>
#include <SPI.h>
#include <string.h>

#define SEND_INTERVAL   250
#define LOGINTERVAL     1000        //default interval for logging
unsigned long logLast = 0;
int logInterval = LOGINTERVAL;

#define CAN_SEND_ID     0x1026105A
byte canSend_data[8] = {0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00};

byte failSends = 0;
byte failAttempts = 25;

unsigned long last_sent_time = 0;

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string
String errCode[] = {"NOER", "MOTO",  "HALL", "CTRL", "BRKE", "LAME"};
String errCode2[] = {"OCUR", "OVOL",  "UVOL", "COTE", "MOTE"};
char markStatus[4] = {'L', 'B', 'C', 'S'};
char bikeStatus[4] = {'P', 'R', 'N', 'D'};
char gearStatus[4] = {'1', '2', '3', 'S'};

byte msgToRead = 0;
byte loggerStatus = 0;      //0=idle, 1=run, 2=wait, 

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

void setup()
{
  Serial.begin(115200);
  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
  //CAN0.setMode(MCP_LISTENONLY);

  pinMode(CAN0_INT, INPUT);                      // Configuring pin for /INT input
  //CAN Mask and filters
  CAN0.init_Mask(0,1,0xFFFFFFF0);                // Init first mask...
  CAN0.init_Filt(0,1,0x10261022);                // Init first filter...
  CAN0.init_Filt(1,1,0x10261023);                // Init second filter...
    
  Serial.println("MCP2515 Library Receive Example...");
}
