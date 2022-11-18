/* Loop code
 *  
 *  
 */
 
void loop(){
    // Check incoming CAN buffer
    if(!digitalRead(CAN0_INT)){                 // If CAN0_INT pin is low, read receive buffer
        CAN0.readMsgBuf(&rxId, &len, rxBuf);    // Read data: len = data length, buf = data byte(s)
        //print_can_data();
        if(msgToRead){
            data_translate();
        }
    }

    // Check if CAN transmitt enabled and if so send CAN msg
    if(digitalRead(SEND_EN)){
        if(check_if_time(last_sent_time, SEND_INTERVAL)){
            can_send_data();
            last_sent_time = millis();
        }
    }
    else{
        failSends = 0;
        failFlags = failFlags & ~FAIL_CTX;      //Clear any failflag of CAN TX
        status_update();
    }

    // Check if BT message received 
    if(Serial.available() > 0){
        com_code = Serial.read();
        if(com_code != char(13) && com_code != char(10))
            com_commands();
    }

    // Check time for next receive/log of CAN message
    if(check_if_time(lastLog, logMillis) && msgToRead){
        if(msgToRead == 3){
            msgToRead = 1;
            lastLog = millis();
        } 
        else {
            if(check_if_time(lastLog, logMillis+rxFailTime)){
                failFlags = failFlags | FAIL_CRX;           //Set fail flag for CAN RX
                status_update();
                no_data_log();
            }
        }
    }

    // Check if button pressed
    if (digitalRead(CAN_LOGGER_BTN)){
        if(btnReleased && check_if_time(btnTimer, btnCooldown)){
            if(msgToRead) log_off();
            else log_on();
            btnReleased = false;
            btnTimer = millis();
        }
    }
    else btnReleased = true;
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
