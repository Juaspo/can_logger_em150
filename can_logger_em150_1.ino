/* Loop code
 *  
 *  
 */
void loop(){
    if (firstRun){
        create_new_file();
        firstRun = false;
    }

    if(!digitalRead(CAN0_INT)){                   // If CAN0_INT pin is low, read receive buffer
        CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)

        if(msgToRead){
            data_translate();
        }
    }

    if(digitalRead(SEND_EN)){
        if(check_if_time(last_sent_time, SEND_INTERVAL)){
            can_send_data();
            last_sent_time = millis();
        }
    }

    if(Serial.available() > 0){
        com_code = Serial.read();
        if(com_code != char(13) && com_code != char(10))
            com_commands();
    }

    if(check_if_time(lastLog, logMillis) && loggingData){
        msgToRead = 1;
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
