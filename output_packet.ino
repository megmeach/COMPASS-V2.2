//Function to generate the output packet
void sendDataPacket(){
  uint8_t outputBytes[PACKET];                              //Define output byte arrays

  outputData.checksum = 0;                           //Reset the checksum
  
  outputData.packetNum++;                            //Did someone say - "update the system with data points equivalent to what was just logged locally?" YES! I said that just now 6/12/1:52:27
  outputData.relTime = compassData.flightTime;
  outputData.t1 = compassData.T2*1000;                   // Setting the output packet data structure. 
  outputData.pressureMS = compassData.PressurePSI;
  memcpy(&outputData.bins[0],&compassData.n3Adata,56);  
  memcpy(&outputBytes, &outputData, 10);               //Pass the packet to the output array as bytes
  memcpy(&outputBytes[10], &outputData.pressureMS, PACKET-10); 

  
  
  for (unsigned short i = 0; i < (PACKET-3); i++){        //Calculate the checksum this literally adds up the numbers contained within the bytes and checks if they are the same
    outputData.checksum += outputBytes[i];
  }

  memcpy(&outputBytes[(PACKET-3)], &outputData.checksum, 3);    //Pass the checksum bytes to a staging array
      
  Serial2.write(outputBytes,PACKET);                     //Send the data to the hardline serial output
  Serial3.write(outputBytes,PACKET); 

//DEBUG
//  for (unsigned short i = 0; i < PACKET; i++){
//    Serial.print((String(i+1)+": ")); // NOTE STARTING FROM NUMBER 1
//    Serial.print(outputBytes[i],HEX);                     //Send the data to the monitor
//    Serial.println();
//  }
//
//  Serial.print("Pressure data: ");
//  byte test[4] = {0};
//  memcpy(&test, &compassData.PressurePSI, 4);
//  for (unsigned short i = 0; i < 4; i++){
//    Serial.print(test[i],HEX);
//    Serial.print(", ");
//  }
//  Serial.println();
}
