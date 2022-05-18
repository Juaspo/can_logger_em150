/* Loop code
 *  
 *  
 */
void loop()
{
  if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    }
    else {
      //for(byte i = 0; i<len; i++){
      //  sprintf(msgString, " 0x%.2X", rxBuf[i]);
      //  Serial.print(msgString);
      //}
      data_translate();
    }
  }
    if((millis() - last_sent_time) >= SEND_INTERVAL){
      byte sndStat1 = CAN0.sendMsgBuf(CAN_SEND_ID, 1, 8, canSend_data);
      print_status(sndStat1, 0x5A);
      last_sent_time = millis();
  }

  if((millis() - logLast) >= logInterval){
    msgToRead = 1;
  }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
