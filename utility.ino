String flightTimeStr() {                                                //Returns the flight time as a usable string for print statements  
  unsigned long t = compassData.flightTime / 1000;
  String fTime = "";
  fTime += (String(t / 3600) + ":");
  t %= 3600;
  fTime += String(t / 600);
  t %= 600;
  fTime += (String(t / 60) + ":");
  t %= 60;
  fTime += (String(t / 10) + String(t % 10));
  return fTime;
}

float flightMinutes() {                                                 //Return time in minutes
  float minutes = compassData.flightTime / 1000;
  minutes = minutes / 60;
  return minutes;
}

void openFlightlog() {                                                  //Open flight log
  if (!FlightlogOpen&&SDcard) {
    //add .c_str() next to Fname
    Flog = SD.open(Fname.c_str(), FILE_WRITE);
    FlightlogOpen = true;
  }
}
void closeFlightlog() {                                                 //Close flight log
  if (FlightlogOpen&&SDcard) {
    Flog.close();
    FlightlogOpen = false;
  }
}

void printData(){                                                       // Printing the data in a neat and orderly fashion. This is for debugging purposes. All of these will be logged onto the SD card
  Serial.println();
  Serial.println("Measurement Update");
  Serial.println("========================================================================================");
  Serial.println("             Time");
  Serial.print("Flight Time String: ");
  Serial.println(flightTimeStr());
  Serial.print("Flight Minutes: ");
  Serial.println(String(flightMinutes()));
  Serial.println("------------------------------");
  Serial.println("          Temperature");
  Serial.println("   t1       t2"); 
  Serial.println((String(compassData.T1,4) + ", " +String(compassData.T2,4)));
  Serial.println("------------------------------");
  Serial.println("           Pressure I2C SENSOR");
  Serial.print("Pressure(PSI): ");
  Serial.println(String(compassData.PressurePSI,6));
  Serial.print("Pressure(ATM): ");
  Serial.println(String(compassData.PressureATM,6));
  Serial.println("------------------------------");
  Serial.println("       System Statuses");
  Serial.print("Sensor Heater Relay: ");
  Serial.println(compassData.sensorHeatStatus);
  Serial.println("------------------------------");
  Serial.println("             OPCs");
  Serial.println("OPC data Packet:");          
  Serial.println(OPCdata);
  Serial.println("========================================================================================");  
}
