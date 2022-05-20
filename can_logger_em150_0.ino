/*//Functions page
*/

void print_status(byte result, byte msg){
    //Prints status of message
    if(result == CAN_OK){
        Serial.print("Msg Sent: ");
        failSends = 0;
    } else {
        Serial.print("Err Send Msg: ");
        failSends++;
    }
    Serial.println(msg);
}

void create_new_file(){
    Serial.println("CFile");
    DateTime now = RTC.now();                                     // read the time from the RTC
    utc = (now.unixtime());
    Serial.println("F0");
    sprintf(filename, "%04d-%02d-%02d_%02d_%02d.log", now.year(), now.month(), now.day(), now.minute(), now.second());
    Serial.println("F1");
    File dataFile = SD.open(filename, FILE_WRITE);
    Serial.println("F2");
    dataFile.print("Log int,");dataFile.print(logMillis);dataFile.println(",ms");
    Serial.println("F3");
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
    dataFile.println("EC2");
    
    dataFile.flush();                                        // wait for serial data to complete transmission
    dataFile.close();
    loggingData = true;
    Serial.println("File X");
}

void can_send_data(){
    byte canSend_data[8] = CAN_SEND_DATA;
    byte sndStat = CAN0.sendMsgBuf(CAN_SEND_ID, 1, 8, canSend_data);
    if(sndStat == CAN_OK){
        failSends = 0;
        failFlags = failFlags & ~FAIL_CRX; //Clear failflag of CAN RX
    } else {
        failSends++;
        if(failSends >= failAttempts){
            Serial.println("Send Error: ");
            failFlags = failFlags | FAIL_CRX;   //Set fail flag for CAN RX
            status_led(RED);
            loggingData = false;
            Serial.println(failFlags, HEX);
        }
    }
}

void status_led(byte led_status){
    digitalWrite(R_LED, led_status & RED);
    digitalWrite(G_LED, led_status & GREEN);
    digitalWrite(B_LED, led_status & BLUE);
}

bool check_if_time(long savedTime, long timeCheck){
    if ((millis() - savedTime) >= timeCheck) return true;
    else return false;
}

void com_commands(){
    switch(com_code){
        case '0': //Turn on logger
            Serial.println(F("Logger on"));
            loggingData = true;
            create_new_file();
            status_led(GREEN);
        break;
        case '1': //Turn off logger
            Serial.println("Logger off");
            loggingData = false;
            delay(1000);
            status_led(YELLOW);
        break;
        case '3': //Increase interval
            if(logMillis < 1000) logMillis += 100;
            else logMillis += 1000;
            Serial.print("intrval: ");
            Serial.println(logMillis);
        break;
        case '4': //decrease interval
            if(logMillis <= 1000) logMillis -= 100;
            else if (logMillis <= 500) Serial.println("limit 500");
            else logMillis -= 1000;
            Serial.print("intrval: ");
            Serial.println(logMillis);
        break;
        default:
            Serial.print(F("unknown cmd: "));
            Serial.print(com_code);
    }
    com_code = 0;
}

//EM150 can data translation
void data_translate(){
    //EM variables
    String errCode[] = {"", "M",  "H", "C", "B", "L"};
    String errCode2[] = {"OC", "OV",  "UV", "CT", "MT"};
    char markStatus[4] = {'L', 'B', 'C', 'S'};
    char bikeStatus[4] = {'P', 'R', 'N', 'D'};
    char gearStatus[4] = {'1', '2', '3', 'S'};

    Serial.println();
    char errorString[4];

    unsigned char dataByte=0;
    if ((rxId & 0x1FFFFFFF) == 0x10261022 && msgToRead == 1){

        if(newFile){
            create_new_file();
            newFile = false;
        }

        File dataFile = SD.open(filename, FILE_WRITE);
        dataFile.print(dateString);dataFile.print(delimiter);
        dataFile.print(timeString);dataFile.print(delimiter);

        //Create divisor --------------------------------------------------
        for (byte i=0; i<25; i++){
            Serial.print('--');
        }Serial.println();

        int i=0;
        dataByte = rxBuf[0];

        //Error codes
        if(dataByte == 0){
            Serial.print(errCode[0]);
            dataFile.print(errCode[0]);
        }
        else{
            unsigned char filter = 1;
            Serial.print("Err1: ");
            for(int i=1; i<6; i++){
                if(dataByte & filter){
                    Serial.print(errCode[i]);
                    Serial.print(" ");
                    dataFile.print(errCode[i]);
                }
                filter = filter << 1;
            }
        }
        dataFile.print(delimiter);
        Serial.println();

        //Mode and gear
        dataByte = rxBuf[1];
        Serial.print("Status: ");
        Serial.println(bikeStatus[(dataByte & 0x30)>>4]);
        dataFile.print(bikeStatus[(dataByte & 0x30)>>4]);dataFile.print(delimiter);
        Serial.print("Gear: ");
        Serial.println(gearStatus[(dataByte & 0xC0)>>6]);
        dataFile.print(gearStatus[(dataByte & 0xC0)>>6]);dataFile.print(delimiter);

        //RPM
        int rpm = rxBuf[3];
        rpm = rpm << 8;
        rpm += rxBuf[2];
        Serial.print("RPM: ");
        Serial.println(rpm);
        dataFile.print(rpm);dataFile.print(delimiter);

        //Battery voltage
        int battLevel = rxBuf[5];
        battLevel = battLevel << 8;
        battLevel += rxBuf[4];
        Serial.print("B-volt: ");
        Serial.println(float(battLevel*0.1));
        dataFile.print(float(battLevel*0.1));dataFile.print(delimiter);

        //Battery current
        int battCurrent = rxBuf[7];
        battCurrent = battCurrent << 8;
        battCurrent += rxBuf[6];
        Serial.print("B-curr: ");
        Serial.println(float(battCurrent*0.1));
        dataFile.print(float(battCurrent*0.1));dataFile.print(delimiter);

        dataFile.flush();                                        // wait for serial data to complete transmission
        dataFile.close();
        msgToRead++;
    }
    else if ((rxId & 0x1FFFFFFF) == 0x10261023 && msgToRead == 2){
        File dataFile = SD.open(filename, FILE_WRITE);

        //Controller temp
        unsigned char temp = rxBuf[0];
        Serial.print("Ctemp: ");
        Serial.println(temp);
        dataFile.print(temp);dataFile.print(delimiter);

        //Motor temp
        temp = rxBuf[1];
        Serial.print("Mtemp: ");
        Serial.println(temp);
        dataFile.print(temp);dataFile.print(delimiter);

        //Hall sensor
        dataFile.print("");dataFile.print(delimiter);

        //Throttle %
        unsigned char throttle = rxBuf[4];
        Serial.print("Throt %: ");
        Serial.println(throttle);
        dataFile.print(throttle);dataFile.print(delimiter);

        //Error codes2
        dataByte = rxBuf[6];
        if(dataByte != 0){
            char filter = 1;
            Serial.print("Err2: ");
            for(int i=1; i<5; i++){
                if(dataByte & filter){
                    //sprintf(errorString, "% ", errCode[i].c_str());
                    Serial.print(errCode2[i]);
                    Serial.print(" ");
                    dataFile.print(errCode2[i]);
                    dataFile.print(" ");
                }
                filter = filter << 1;
            }
        }
        else dataFile.print("");
        dataFile.println(delimiter);

        dataFile.flush();                                        // wait for serial data to complete transmission
        dataFile.close();

        msgToRead = 0;
    }
    /*else{
        if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
            sprintf(msgString, "ExtID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
        else
            sprintf(msgString, "StrdID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
        Serial.print(msgString);
    }*/
}