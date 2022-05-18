/*//Functions page
*/

void print_status(byte result, byte msg){
  //Prints status of message
  if(result == CAN_OK){
    Serial.print("Message Sent Successfully: ");
    failSends = 0;
  } else {
    Serial.print("Error Sending Message: ");
    failSends++;
  }
  Serial.println(msg);
}

void data_translate(){
    Serial.println();
    char errorString[4];

    unsigned char dataByte=0;
    if ((rxId & 0x1FFFFFFF) == 0x10261022 && msgToRead == 1){
        Serial.println("############# controller ###############");
        int i=0;
        dataByte = rxBuf[0];
        //sprintf(errorString, " REMOTE REQUEST FRAME");

        //Error Code
        if(dataByte == 0){
            Serial.print("No Error");
        }
        else{
            unsigned char filter = 1;
            Serial.print("Errors: ");
            for(int i=1; i<6; i++){
                //Serial.println("data byte and filter");
                //Serial.println(dataByte);
                //Serial.println(filter);
                if(dataByte & filter){
                    //sprintf(errorString, "% ", errCode[i].c_str());
                    Serial.print(errCode[i]);
                    Serial.print(" ");
                }
                filter = filter << 1;
            }
        }
        Serial.println();

        //Mode and gear
        dataByte = rxBuf[1];
        Serial.print("Status: ");
        Serial.println(bikeStatus[(dataByte & 0x30)>>4]);
        Serial.print("Gear: ");
        Serial.println(gearStatus[(dataByte & 0xC0)>>6]);

        //RPM
        int rpm = rxBuf[3];
        rpm = rpm << 8;
        rpm += rxBuf[2];
        Serial.print("RPM: ");
        Serial.println(rpm);

        //Battery voltage
        int battLevel = rxBuf[5];
        battLevel = battLevel << 8;
        battLevel += rxBuf[4];
        Serial.print("Battery voltage: ");
        Serial.println(float(battLevel*0.1));

        //Battery current
        int battCurrent = rxBuf[7];
        battCurrent = battCurrent << 8;
        battCurrent += rxBuf[6];
        Serial.print("Battery current: ");
        Serial.println(float(battCurrent*0.1));
        msgToRead++;
    }
    else if ((rxId & 0x1FFFFFFF) == 0x10261023 && msgToRead == 2){
        unsigned char temp = rxBuf[0];
        Serial.print("Controller temp: ");
        Serial.println(temp);
        temp = rxBuf[1];
        Serial.print("Motor temp: ");
        Serial.println(temp);
        unsigned char throttle = rxBuf[4];
        Serial.print("Throttle precentage: ");
        Serial.println(throttle);

        dataByte = rxBuf[6];
        if(dataByte == 0) Serial.println("No errors");
        else{
            char filter = 1;
            Serial.print("Errors detected: ");
            for(int i=1; i<5; i++){
                if(dataByte & filter){
                    //sprintf(errorString, "% ", errCode[i].c_str());
                    Serial.print(errCode2[i]);
                    Serial.print(" ");
                }
                filter = filter << 1;
            }
        }
        msgToRead = 0;
    }
    else{
        if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
            sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
        else
            sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
        Serial.print(msgString);
    }
}
