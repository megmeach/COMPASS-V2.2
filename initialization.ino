////////////////////////////////////////////////////
////////// Initialize Data String //////////////////
////////////////////////////////////////////////////
void initData(){
  DATA_SERIAL1.begin(DATA_BAUD);                                         //Data Transfer
  DATA_SERIAL2.begin(DATA_BAUD);
  

  String FHeader = "Flight Time, Minutes ,ATemp (C),BTemp (C),Pressure (PSI),Pressure (ATM),";
  FHeader += "Sensor Heater Status,baro Temp (C), baro Alt";
  FHeader += (",N3A,0dot35,0dot46,0dot66,1dot0,1dot3,1dot7,2dot3,3dot0,4dot0,5dot2,6dot5,8dot0,10dot0,12dot0,bin15,bin16,bin17,bin18,bin19,bin20,bin21,bin22,bin23,40dot0,n3temp,n3humidity,sampling_period,flow_rate,");

  pinMode(chipSelect, OUTPUT);                                         //initialize SD card
  
  if (!SD.begin(chipSelect)) {                                      //power LED will blink if no card is inserted
    Serial.println("No SD");
    SDcard = false;
  }
  SDcard = true;

  for (int i = 0; i < 100; i++) {                                      //Flight Log Name Cration
    Fname = String("COMP" + String(i / 10) + String(i % 10) + ".csv");
    if (!SD.exists(Fname.c_str())) {
      openFlightlog();
      break;
    }
  }
  
  Serial.println("Flight log created: " + Fname);
  
  Flog.println(FHeader);                                               //Set up Flight log format
  Serial.println("Flight log header added");                            

  closeFlightlog();
}

/////////////////////////////////////////////////
////////// Initialize Pressure //////////////////
/////////////////////////////////////////////////
void initPressure() {
  if(pressureSensor){
    if(!baro.begin())
    {
      Serial.println(F("Could not find a valid MS5611 sensor, check wiring!"));
    }
  
    baroReferencePressure = baro.readPressure();                    // Get a reference pressure for relative altitude
    Serial.println(F("MS5611 barometer setup successful..."));
  }else{
    baroReferencePressure = 999;
    Serial.println(F("Intentionally not using a MS5611 sensor"));
  }
  

  updatePressure();
}

///////////////////////////////////////////////
////////// Initialize Relays //////////////////
///////////////////////////////////////////////
void initRelays(){
  sensorHeatRelay.init(false);                                          //Initialize relays
  
  sensorHeat_Status = "OFF";
}


////////////////////////////////////////////
////////// Initialize GPS //////////////////
////////////////////////////////////////////
void initOPCs() {                                                       //Sets up serial and initializes the OPCs
  n3A.initialize();
  delay(1000);
  n3A.setFanDigitalPotShutdownState(true);
  
  Serial.println("N3A Initialized");
}
