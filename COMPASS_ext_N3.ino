//============================================================================================================================================
//MURI SPS Blackbox 
//Compact Optical Measurement of Particles via an Autonomous SPS System (COMPASS)
//Written by UMN MURI 2020
//============================================================================================================================================
//
//Version Description: N3 standalone configuration. Takes in >5V and outputs a serial string as noted in the COMPASS System Information Documentation.
//
//=============================================================================================================================================
//=============================================================================================================================================
//           ____                                    
//          / ___|___  _ __ ___  _ __   __ _ ___ ___ 
//         | |   / _ \| '_ ` _ \| '_ \ / _` / __/ __|
//         | |__| (_) | | | | | | |_) | (_| \__ \__ \
//          \____\___/|_| |_| |_| .__/ \__,_|___/___/
//                              |_|                                                                                                                                                                                                                                                                                            
//=============================================================================================================================================
//=============================================================================================================================================

//System Preferences
#define SYSTEM_ID 0x01

/*  Teensy 3.5/3.6 pin connections:
     ------------------------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used         
                                  | Communication: (Pins used) (RX/TX on Teensy for UART)
     UBlox Neo m8n                | UART 1 (0,1)          
     SPS30 A                      | UART 2 (9,10)
     SPS30 B                      | UART 3 (7,8)
     Data Stream                  | UART 5 (34,33)
     Thermistor A                 | Analog (22/A8)
     Thermistor B                 | Analog (23/A9)
     SD A                         | SPI 0 (11,12,13,20)
     Pressure sensor              | Analog (21/A7)
     OLED                         | I2C 0 (18,19)
     OPC Heater                   | (35,36)
     LED                          | (5)
     
     -------------------------------------------------------------------------------------------------------------------------------------------
*/
/////////////////////////////
//////////Libraries//////////
/////////////////////////////
#include <SPI.h>                                                        //SPI Library for R1
#include <SD.h>                                                         //SD Library for logging
#include <Arduino.h>                                                    //Arduino kit
#include <OPCN3.h>
#include <LatchRelay.h>                                                 //Heater relay
#include <Wire.h>                                                       //I2C library if the I2C mode in SPS is disabled
#include <MS5611.h>

////////////////////////////////////
//////////Pin Definitions///////////
////////////////////////////////////
#define SENSOR_HEATER_ON 35                                             //Latching Relay pins for heaters
#define SENSOR_HEATER_OFF 36
#define HONEYWELL_FLOW A3                                          //Analog Honeywell Pressure Sensor
#define THERMISTOR_A A8                                                 //Analog pins for the thermistors
#define THERMISTOR_B A9                                       
#define DATA_SERIAL1 Serial2
#define DATA_SERIAL2 Serial3                                        
#define N3_PIN 20
#define PIN_RESET 17                                                    //The library assumes a reset pin is necessary. The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is not being used

//////////////////////////////
//////////Constants///////////
//////////////////////////////
//Bauds
#define DATA_BAUD 1200                                                //Baud rate for the data string

//Data Transfer
#define BEGIN 0x42                                                      //Start bit for the data transfer
#define STOP  0x53                                                      //Stop bit for the data transfer
#define PACKET 73                                                       //Packet Size

//Values
#define MINUTES_TO_MILLIS 60000                                         //MATH TIME
#define PSI_TO_ATM  0.068046                                            //Pounds per square inch to atmospheres   
#define C2K 273.15                                                      //Degrees Celsius to Kelvin
#define DC_JUMPER 1                                                     //The DC_JUMPER is the I2C Address Select jumper. Set to 1 if the jumper is open (Default), or set to 0 if it's closed.
#define NoFix 0x00                                                      // If the GPS doesn't get a fix, it should return this data bit and that's what NoFix is for
#define Fix 0x01                                                        // Same thing as above, but this is if it gets a fix
#define ADC_MAX 8191.0                                                    // The maximum adc value given to the thermistor, should be 8192 for a teensy and 1024 for an Arduino
#define CONST_A 0.001125308852122                                       // A, B, and C are constants used for a 10k resistor and 10k thermistor for the steinhart-hart equation
#define CONST_B 0.000234711863267                                       // NOTE: These values change when the thermistor and/or resistor change value, so if that happens, more research needs to be done on those constants
#define CONST_C 0.000000085663516                                       
#define CONST_R 10000                                                   // 10k Ω resistor 
#define FEET_PER_METER 3.28084                                          // feet per meter
#define VSUP 3.3
#define PMAX 15.0
#define PMIN 0.0

//Control
#define HIGH_TEMP 5                                                    //Thermal control, this is the high temperature in celcius
#define LOW_TEMP -5                                                     // Low temperature in celcius
#define LOG_RATE 1700                                                   // The rate at which we send data over.This is 1.7 seconds, or 1,700 milliseconds

////////////////////////
//////////Data//////////
////////////////////////
struct n3Data_abv{                                                     // Data structure for the SPS data we receive. It contains the number of hits and a number count of 5
  uint16_t bins[24];
  uint16_t temp;
  uint16_t humid;
  uint16_t sampPer;
  uint16_t sampFlow;
};

struct systemData{                                                      // Data structure fot the entire system. This includes a data structure for the SPS and the GPS, so they are nested structures
  unsigned long flightTime;                                             // This is done to make the code more efficient and easier to read. ALL of our data lies under the compassData data structure
  float T1 = -127.00, T2 = -127.00;
  float PressureATM = 0, PressurePSI = 0;
  bool sensorHeatStatus;
  n3Data_abv n3Adata;
}compassData;

struct outputPacket{                                                    // Data structure for the output packet of the system. This is the raw data being sent to the SD card. Everything from compassData
  uint8_t strt = BEGIN, sysID = SYSTEM_ID;                              // is copied over and transferred to output packet in a systematic way
  uint16_t packetNum = 0;
  uint32_t relTime = millis();
  uint16_t t1 = 0;
  float pressureMS = 0;
  uint16_t bins[24] = {0};
  uint16_t temp = 0, humid = 0, sampPer = 0, sampFlow = 0;
  uint16_t checksum = 0;
  uint8_t stp = STOP;
}outputData;



///////////////////////////////////
//////////Data Management//////////
///////////////////////////////////
//Data Log
unsigned long logCounter = 0;
String data;

// SDClass SD;                                            //SD ERROR?????? MAKE SURE THIS LINE IS COMMENTED OUT UNLESS YOU ARE NATHAN PHARIS
File Flog;                                                                 //Variables needed to establish the flight log
static String dataLine;
String Fname = "";
boolean SDcard = true;
static boolean FlightlogOpen = false;                                      //SD for Flight Computer
const int chipSelect = BUILTIN_SDCARD; 

////////////////////////////////////////////////
//////////Environment Sensor Variables//////////
////////////////////////////////////////////////

// active heating variables
float sensTemp;
bool coldSensor = false;
LatchRelay sensorHeatRelay(SENSOR_HEATER_ON,SENSOR_HEATER_OFF);            //Declare latching relay objects and related logging variables
String sensorHeat_Status = "";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MS5611 Pressure Sensor Variables
bool pressureSensor = false;                                                   // Declare if pressure sensor is in use
MS5611 baro;
float seaLevelPressure;                                                    // in Pascals
float baroReferencePressure;                                               // some fun pressure/temperature readings below 
float baroTemp;                                                            // non-"raw" readings are in Pa for pressure, C for temp, meters for altitude
unsigned int pressurePa;
float pressureAltitude;
float pressureRelativeAltitude;
boolean baroCalibrated = false;                                            // inidicates if the barometer has been calibrated or not

////////////////////////
//////////OPCs//////////
////////////////////////
OPCN3 n3A(N3_PIN);   
String OPCdata = "";

void setup() {
  analogReadResolution(13);                                            // Since this is a Teensy we are using, the read bit resolution can be at a max of 13. So, we want the best resoliution possible. This is why the thermistors have a higher mac adc value than the Arduino
  SPI.begin();                                                         // Beginning the SPI and wire libraries and the serial monitor
  Wire.begin();
  Serial.begin(9600);
  Serial5.begin(DATA_BAUD);

  Serial.println("Beginning Initialization cycle!");
  
  initData();                                                          //Initialize SD
  Serial.println("Data init!");

  initPressure();
  Serial.println("Pressure init!");

  initRelays();                                                        //Initialize Relays
  Serial.println("Actuator init!");

  initOPCs();                                                          //Initialize OPCs
  Serial.println("OPC init!");
  delay(1000);
  
  Serial.println("Setup Complete");
}

void loop() {
  if (millis() - logCounter >= LOG_RATE) {
      logCounter = millis();
      
      updateSensors();                                                  //Updates and logs all sensor data
      sendDataPacket();                                                 //Output the data packet
      actHeat();                                                        //Controls active heating
  }   
}
