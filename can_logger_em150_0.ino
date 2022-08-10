/* Functions page
*
*
*
*/

void status_update(){
    if (failFlags & 0x0F){
        status_led(RED);
        Serial.print(F("Fatal err: 0x0"));
        Serial.println(failFlags, HEX);
        while(1){
            delay(250);
            status_led(RED);
            delay(250);
            status_led(LED_OFF);
        }
    }
    else if(failFlags & 0xF0){
        status_led(RED);
        if (lastFailFlag != failFlags){
            Serial.print(flagStr);
            Serial.println(failFlags, HEX);
        }
    }
    else if (msgToRead) status_led(GREEN);
    else {
        status_led(YELLOW);
    }
    lastFailFlag = failFlags;
}

void status_led(byte led_status){
    digitalWrite(R_LED, led_status & RED);
    digitalWrite(G_LED, led_status & GREEN);
    digitalWrite(B_LED, led_status & BLUE);
}

void log_on(){
    msgToRead = 1;
    create_new_file();
    lastLog = millis();
    status_update();
    Serial.println(F("Log On"));
}
void log_off(){
    failFlags = failFlags & ~FAIL_CRX; //Clear failflag of CAN RX
    msgToRead = 0;
    status_update();
    Serial.println(F("Log Off"));
    delay(1000);
}

void create_new_file(){
    DateTime now = RTC.now();                                     // read the time from the RTC
    utc = (now.unixtime());
    sprintf(filename, "%04d%02d%02d_%02d%02d%02d.log", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    File dataFile = SD.open(filename, FILE_WRITE);
    dataFile.print("LogTime,");dataFile.print(logMillis);dataFile.println(",ms");
    dataFile.print("date");dataFile.print(delimiter);
    dataFile.print("time");dataFile.print(delimiter);
    dataFile.print("EC1");dataFile.print(delimiter);
    dataFile.print("mark");dataFile.print(delimiter);
    dataFile.print("PRND");dataFile.print(delimiter);
    dataFile.print("gear");dataFile.print(delimiter);
    dataFile.print("RPM");dataFile.print(delimiter);
    dataFile.print("BVo");dataFile.print(delimiter);
    dataFile.print("BCu");dataFile.print(delimiter);
    dataFile.print("CTmp");dataFile.print(delimiter);
    dataFile.print("MTmp");dataFile.print(delimiter);
    dataFile.print("HALL");dataFile.print(delimiter);
    dataFile.print("T%");dataFile.print(delimiter);
    dataFile.print("EC2");dataFile.print(delimiter);
    dataFile.println("FF");

    dataFile.flush();                                        // wait for serial data to complete transmission
    dataFile.close();
}

void can_send_data(){
    byte canSend_data[8] = CAN_SEND_DATA;
    byte sndStat = CAN0.sendMsgBuf(CAN_SEND_ID, 1, 8, canSend_data);
    if(sndStat == CAN_OK){
        failSends = 0;
        failFlags = failFlags & ~FAIL_CTX;                  //Clear failflag of CAN TX
    } else {
        failSends++;
        if(failSends >= txFails){
            failFlags = failFlags | FAIL_CTX;               //Set fail flag for CAN TX
        }
    }
    status_update();
}

bool check_if_time(long savedTime, long timeCheck){
    if ((millis() - savedTime) >= timeCheck) return true;
    else return false;
}

void com_commands(){
    switch(com_code){
        case '0': //Turn off logger
            log_off();
        break;
        case '1': //Turn on logger
            log_on();
        break;
        case '2': //Increase interval
            if(logMillis < 100) logMillis += 10;
            else if(logMillis < 1000) logMillis += 100;
            else logMillis += 1000;
            Serial.print(logtStr);
            Serial.println(logMillis);
        break;
        case '3': //decrease interval
            if(logMillis <= 0);
            else if (logMillis <= 100) logMillis -= 10;
            else if (logMillis <= 1000) logMillis -= 100;
            else logMillis -= 1000;
            Serial.print(logtStr);
            Serial.println(logMillis);
        break;
        case '4': //show current config
            get_date_time();
            Serial.print(dateStr);
            Serial.println(dateString);
            Serial.print(timeStr);
            Serial.println(timeString);
            Serial.print(flagStr);
            Serial.println(failFlags, HEX);
            Serial.print(logtStr);
            Serial.println(logMillis);
            Serial.print(canTxStr);
            Serial.println(digitalRead(SEND_EN));
            Serial.print(canRxStr);
            Serial.println(msgToRead);
        break;
        default:
            Serial.print(F("bad cmd: "));
            Serial.print(com_code);
    }
    com_code = 'X';
}

void get_date_time(){
        DateTime now = RTC.now();                                   // read the time from the RTC
        utc = (now.unixtime());
        sprintf(dateString, "%04d-%02d-%02d", now.year(), now.month(), now.day());
        sprintf(timeString, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
}

void no_data_log(){
    get_date_time();
    File dataFile = SD.open(filename, FILE_WRITE);
    dataFile.print(dateString);dataFile.print(delimiter);
    dataFile.print(timeString);

    if(logMillis <= 100){
        dataFile.print(msAppend);
    }

    dataFile.print(delimiter);
    for(byte n=0; n<12; n++)   dataFile.print(delimiter);           // skip all missing data points
    dataFile.print(failFlags, HEX); dataFile.println(delimiter);    // add failflags to log
    dataFile.flush();                                               // wait for serial data to complete transmission
    dataFile.close();
}

//EM150 can data translation
void data_translate(){
    //EM variables
    String errCode[] = {"-", "M",  "H", "C", "B", "L"};
    String errCode2[] = {"OC", "OV",  "UV", "CT", "MT"};
    char markStatus[4] = {'L', 'B', 'C', 'S'};
    char bikeStatus[4] = {'P', 'R', 'N', 'D'};
    char gearStatus[4] = {'1', '2', '3', 'S'};
    char hallStatus[3] = {'A', 'B', 'C'};
    unsigned char filter = 1;

    Serial.println();
    char errorString[4];

    unsigned char dataByte=0;
    if ((rxId & 0x1FFFFFFF) == 0x10261022 && msgToRead == 1){
        failFlags = failFlags & ~FAIL_CRX;                          //Clear failflag of CAN RX

        get_date_time();

        File dataFile = SD.open(filename, FILE_WRITE);
        dataFile.print(dateString);dataFile.print(delimiter);
        dataFile.print(timeString);dataFile.print(delimiter);

        //Create divisor --------------------------------------------------
        Serial.print(F("----------------------------------------------------"));

        //int i=0;
        dataByte = rxBuf[0];

        //Error codes
        Serial.print(F("Err1: "));
        if(dataByte == 0){
            Serial.print(errCode[0]);
            dataFile.print(errCode[0]);
        }
        else{
            for(byte i=1; i<6; i++){
                if(dataByte & filter){
                    Serial.print(errCode[i]);
                    Serial.print(F(" "));
                    dataFile.print(errCode[i]);
                }
                filter = filter << 1;
            }
        }
        dataFile.print(delimiter);
        Serial.println();

        //Lock status
        filter = 1;
        dataByte = rxBuf[1];
        Serial.print(F("Lock: "));
        for(byte n=0; n<4; n++){
            if(dataByte & filter){
                Serial.print(markStatus[n]);
                dataFile.print(markStatus[n]);
            }
            filter = filter << 1;
        }
        Serial.println();
        dataFile.print(delimiter);

        //Mode and gear
        Serial.print(F("Status: "));
        Serial.println(bikeStatus[(dataByte & 0x30)>>4]);
        dataFile.print(bikeStatus[(dataByte & 0x30)>>4]);dataFile.print(delimiter);
        Serial.print(F("Gear: "));
        Serial.println(gearStatus[(dataByte & 0xC0)>>6]);
        dataFile.print(gearStatus[(dataByte & 0xC0)>>6]);dataFile.print(delimiter);

        //RPM
        int rpm = rxBuf[3];
        rpm = rpm << 8;
        rpm += rxBuf[2];
        Serial.print(F("RPM: "));
        Serial.println(rpm);
        dataFile.print(rpm);dataFile.print(delimiter);

        //Battery voltage
        int battLevel = rxBuf[5];
        battLevel = battLevel << 8;
        battLevel += rxBuf[4];
        Serial.print(F("B-V: "));
        Serial.println(float(battLevel*0.1));
        dataFile.print(float(battLevel*0.1));dataFile.print(delimiter);

        //Battery current
        int battCurrent = rxBuf[7];
        battCurrent = battCurrent << 8;
        battCurrent += rxBuf[6];
        Serial.print(F("B-I: "));
        Serial.println(float(battCurrent*0.1));
        dataFile.print(float(battCurrent*0.1));dataFile.print(delimiter);

        dataFile.flush();                                        // wait for serial data to complete transmission
        dataFile.close();
        msgToRead = 2;
    }
    else if ((rxId & 0x1FFFFFFF) == 0x10261023 && msgToRead == 2){
        File dataFile = SD.open(filename, FILE_WRITE);

        //Controller temp
        unsigned char temp = rxBuf[0];
        Serial.print(F("Ctmp: "));
        Serial.println(temp);
        dataFile.print(temp);dataFile.print(delimiter);

        //Motor temp
        temp = rxBuf[1];
        Serial.print(F("Mtmp: "));
        Serial.println(temp);
        dataFile.print(temp);dataFile.print(delimiter);

        //Hall sensor
        dataByte = rxBuf[3];
        filter = 1;
        Serial.print(F("Hall: "));
        for (byte n=0; n<3; n++){
            if(dataByte & filter){
                Serial.print(hallStatus[n]);
                dataFile.print(hallStatus[n]);
            }
            filter = filter << 1;
        }
        dataFile.print(delimiter);
        Serial.println();

        //Throttle %
        unsigned char throttle = rxBuf[4];
        Serial.print(F("Throt %: "));
        Serial.println(throttle);
        dataFile.print(throttle);dataFile.print(delimiter);

        //Error codes2
        dataByte = rxBuf[6];
        if(dataByte != 0){
            char filter = 1;
            Serial.print(F("Err2: "));
            for(int i=1; i<5; i++){
                if(dataByte & filter){
                    Serial.print(errCode2[i]);
                    Serial.print(F(" "));
                    dataFile.print(errCode2[i]);
                    dataFile.print(" ");
                }
                filter = filter << 1;
            }
        }
        else dataFile.print("");
        dataFile.print(delimiter);
        dataFile.print(failFlags, HEX); dataFile.println(delimiter);
        dataFile.flush();                                        // wait for serial data to complete transmission
        dataFile.close();

        msgToRead = 3;
    }
}
