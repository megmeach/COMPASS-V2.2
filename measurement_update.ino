//function to handle both retrieval of data from sensors, as well as recording it on the SD card
void updateSensors() {
  
  updatePressure();
  
  if(sensorHeatRelay.getState()){
    sensorHeat_Status = "ON";
  }
  else if(!sensorHeatRelay.getState()){
    sensorHeat_Status = "OFF";
  }


  //////////////////////////////////////////////////////
  //////////// Raw calculation of variables ////////////
  //////////////////////////////////////////////////////
  // Temperature, Pressure, and OPc
  // First, temperature
  compassData.T1 = analogRead(THERMISTOR_A);                                   // All of these calculations have to do with the Steinhart-Hart equations and setting them up properly
  compassData.T2 = analogRead(THERMISTOR_B);
  compassData.T1 = log(((ADC_MAX/compassData.T1)-1)*CONST_R);
  compassData.T2 = log(((ADC_MAX/compassData.T2)-1)*CONST_R);
  compassData.T1 = CONST_A+CONST_B*compassData.T1+CONST_C*compassData.T1*compassData.T1*compassData.T1;
  compassData.T2 = CONST_A+CONST_B*compassData.T2+CONST_C*compassData.T2*compassData.T2*compassData.T2;
  compassData.T1 = 1/compassData.T1-C2K;                                                  // The final temperatures for both transistors in Celsius
  compassData.T2 = 1/compassData.T2-C2K;


  // Finally, OPC data
  HistogramData hist = n3A.readHistogramData();

  OPCdata = "";
  memcpy(&compassData.n3Adata.bins[0],&hist.binCounts[0],48);
  compassData.n3Adata.temp = hist.temperature;
  compassData.n3Adata.humid = hist.humidity;
  compassData.n3Adata.sampPer = hist.samplingPeriod;
  compassData.n3Adata.sampFlow = hist.sampleFlowRate; 
  
  for (unsigned short i = 0; i < 24; i++){
       OPCdata += String(hist.binCounts[i]);
       OPCdata += ",";
  }
   OPCdata += String(hist.getTempC());
   OPCdata += ",";
   OPCdata += String(hist.getHumidity());
   OPCdata += ",";
   OPCdata += String(hist.samplingPeriod);
   OPCdata += ",";
   OPCdata += String(hist.sampleFlowRate);

 

  //////////////////////////////////////////////////////////////////////////////////
  ///////////// UPDATING THE SYSTEM DATA STRUCT (called "compassData") /////////////
  //////////////////////////////////////////////////////////////////////////////////
  //Using a struct means that update functions are only called once, so the data logged locally and sent externally is universally consistent.

  compassData.flightTime = millis();

  compassData.sensorHeatStatus = sensorHeat_Status;
  if(pressureSensor){
    compassData.PressureATM = baro.readPressure()*0.00000986923;
    compassData.PressurePSI = compassData.PressureATM*1/PSI_TO_ATM;
  }else{
    compassData.PressureATM = 999;
    compassData.PressurePSI = 999;
  }
  
  
  
  //////////////////////////////////////////////////////////////////
  ////////// UPDATING THE DATA STRING (called "data") //////////////
  //////////////////////////////////////////////////////////////////
  
  dataLine = "";
  dataLine = flightTimeStr()+ "," + String(flightMinutes()) + ",";
  
  dataLine += (String(compassData.T1,4) + "," +String(compassData.T2,4) + ",");     //Data string population
  dataLine += (String(compassData.PressurePSI,6) + ",");
  dataLine += (compassData.sensorHeatStatus + "," + String(baroTemp,4) + "," + String(pressureRelativeAltitude,6));
  dataLine += (",=," + OPCdata);

  
  /////////////////////////////////////////////////////////////
  ///////////// Logging the data onto the SD card /////////////
  /////////////////////////////////////////////////////////////
  // All these functions are in the "SD.h" library, so for more information, go there

  openFlightlog();
  delay(100); 
  Flog.println(dataLine);                                                                   // Printing the data from A in the SD card
  closeFlightlog();


  ///////////////////////////////////////////////////////
  //////////// Printing the Data to Serial //////////////
  ///////////////////////////////////////////////////////
  // Go to "utility.ino" for more on this function
  
  printData();

}

void updatePressure(){
  if(pressureSensor){
    // Read true temperature & Pressure
    baroTemp = baro.readTemperature();
    pressurePa = baro.readPressure();
    // Calculate altitude
    pressureRelativeAltitude = baro.getAltitude(pressurePa, baroReferencePressure)*FEET_PER_METER; 
  } else{
    baroTemp = 999;
    pressurePa = 999;
    pressureRelativeAltitude = 999;
  }
}
