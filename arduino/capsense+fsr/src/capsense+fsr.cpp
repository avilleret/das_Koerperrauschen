#include <Arduino.h>
#include <CapacitiveSensor.h>
#include <OSCBundle.h>
#include <EEPROM.h>

//Teensy and Leonardo variants have special USB serial
#if defined(CORE_TEENSY)|| defined(__AVR_ATmega32U4__)
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial(Serial);
#else
// any hardware serial port
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif

/*
 * Antoine Villeret - EnsadLab 2013
 * project : das Körperrauschen
 *
 * Scan capacitive and FSR sensor and send values 
 * through USB in OSC SLIP-encoded messages.
 * Uses a high value resistor e.g. 40M between send pin and receive pin.
 * Resistor effects sensitivity, experiment with values, 50K - 50M. 
 * Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of 
 * foil/metal on this pin.
 * 
 */

#define COMMON_PIN 13
#define MAX_STRING_LENGTH 32

// prototypes
void FSRcontrol(OSCMessage &msg, int addrOffset);
void CScontrol(OSCMessage &msg, int addrOffset);
void SystemControl(OSCMessage &msg, int addrOffset);
void setup();
void receiveOSC();
void loop();
void saveParam();
void loadParam();


// global variables

char path[MAX_STRING_LENGTH];
unsigned char offset=0;

long start_scan=0, end_scan=0;

// 40M resistor between COMMON_PIN & sensor pin, for each sensor
CapacitiveSensor   cs[] = { CapacitiveSensor(COMMON_PIN,0), /* CS0 */ \ 
                            CapacitiveSensor(COMMON_PIN,1), /* CS1 */ \
                            CapacitiveSensor(COMMON_PIN,2), /* CS2 */ \
                            CapacitiveSensor(COMMON_PIN,3), /* CS3 */ \
                            CapacitiveSensor(COMMON_PIN,4), /* CS4 */ \
                            CapacitiveSensor(COMMON_PIN,5), /* CS5 */ \
                            CapacitiveSensor(COMMON_PIN,7), /* CS6 */ \
                            CapacitiveSensor(COMMON_PIN,8), /* CS7 */ \
                            CapacitiveSensor(COMMON_PIN,11)}; /* CS8 */ 
#define CS_COUNT 9 // number of CapacitiveSensor occurence above...
static const int fsr_pin[] = {A0,A1,A2,A3,A4,A5,A11,A10,A9}; // FSR pin
#define FSR_COUNT 9

/* global parameters */
// since this struct is saved 'as is' in EEPROM, please add new element at the end
typedef struct {
  uint8_t address;
  bool fsr_enable[15];
  bool fsr_on;
  bool cs_enable[15];
  bool cs_on;

  uint16_t cs_sensibility;
  uint16_t cs_timeout;
  int16_t cs_autocal;
  uint16_t loopDelay;
} BoardParam;
/* end of global parameters */

BoardParam boardParam;

int i=0, cs_id;
long rawcount=0;
float normcount=0.;
OSCBundle bundleOUT;

void FSRcontrol(OSCMessage &msg, int addrOffset)
{
  if (msg.fullMatch("/on",addrOffset)) {
    if (msg.size() == 0){
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/fsr/on");
      bundleOUT.add(path).add(boardParam.fsr_on);
    } else if (msg.isInt(0)) {
      boardParam.fsr_on = msg.getInt(0) > 0;
    }
  } else if ( msg.fullMatch("/enable",addrOffset) ){
    if ( msg.size() == 0 ){
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/fsr/enable");
      OSCMessage msgOUT(path);
      for (i=0;i<CS_COUNT;i++){
        msgOUT.add(boardParam.fsr_enable[i]);
      }
      msgOUT.send(SLIPSerial);
      SLIPSerial.endPacket();
      msgOUT.empty();
    } else {
      for (i=0;i<msg.size() && i<CS_COUNT;i++){
        boardParam.fsr_enable[i] = msg.getInt(i) > 0;
      }
    }
  }
}

void CScontrol(OSCMessage &msg, int addrOffset)
{
  if (msg.fullMatch("/on",addrOffset)) {
    if (msg.size() == 0){
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/on");
      bundleOUT.add(path).add(boardParam.cs_on);
    } else if (msg.isInt(0)){
      boardParam.cs_on = msg.getInt(0) > 0;
    }
  } else if ( msg.fullMatch("/enable", addrOffset) ) {
    if ( msg.size() == 0 ){ // if no data, send back current settings
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/enable");
      OSCMessage msgOUT(path);
      for (i=0;i<CS_COUNT;i++){
        msgOUT.add(boardParam.cs_enable[i]);
      }
      msgOUT.send(SLIPSerial);
      SLIPSerial.endPacket();
      msgOUT.empty();
    } else {
      for (i=0;i<msg.size() && i<CS_COUNT;i++){
        boardParam.cs_enable[i] = msg.getInt(i) > 0;
      }
    }
  } else if ( msg.fullMatch("/sensibility", addrOffset) ) {
    if ( msg.size() == 0 ){
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/sensibility");
      bundleOUT.add(path).add( (int) boardParam.cs_sensibility);
    } else if (msg.isInt(0)) {
      boardParam.cs_sensibility = msg.getInt(0);
    }
  } else if ( msg.fullMatch("/reset", addrOffset) ) {
    for ( i=0; i<CS_COUNT ; i++)
    {
      cs[i].reset_CS_AutoCal(); // reset all sensors
    }
  } else if ( msg.fullMatch("/autocal", addrOffset) ) {
    if (msg.size() == 0){
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/autocal");
      bundleOUT.add(path).add( (int) boardParam.cs_autocal);
    } else if (msg.isInt(0))
    {
      boardParam.cs_autocal=msg.getInt(0);
      for ( i=0; i<CS_COUNT ; i++)
      {
        cs[i].set_CS_AutocaL_Millis(boardParam.cs_autocal);
      }
    }
  } else if ( msg.fullMatch("/timeout", addrOffset) ) {
    if (msg.size() == 0){
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/timeout");
      bundleOUT.add(path).add((int)boardParam.cs_timeout);
    } else if (msg.isInt(0)) {
      boardParam.cs_timeout=msg.getInt(0);
      for ( i=0; i<CS_COUNT ; i++)
      {
        cs[i].set_CS_Timeout_Millis(boardParam.cs_timeout);
      }
    }
  }
}

void SystemControl(OSCMessage &msg, int addrOffset){
  if (msg.fullMatch("/del",addrOffset)) { // set the delay on each loop
    if (msg.size() == 0){
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/s/del");
      bundleOUT.add(path).add((int) boardParam.loopDelay);
    } else if (msg.isInt(0))
    {
      boardParam.loopDelay = msg.getInt(0)>0?msg.getInt(0):0;
    }
  } else if (msg.fullMatch("/save",addrOffset)){
    saveParam();
  } else if (msg.fullMatch("/load",addrOffset)){
    loadParam();
  } else {
    snprintf(path+offset,MAX_STRING_LENGTH-offset,"/s/date");
    bundleOUT.add(path).add(__DATE__);
    snprintf(path+offset,MAX_STRING_LENGTH-offset,"/s/time");
    bundleOUT.add(path).add(__TIME__);
  }
}
 
void getAllParams(){
  OSCMessage get_on("/on");
  FSRcontrol(get_on,0);
  CScontrol(get_on,0);
  
  OSCMessage get_enable("/enable");
  FSRcontrol(get_enable,0);
  CScontrol(get_enable,0);
  
  OSCMessage get_sensibility("/sensibility");
  CScontrol(get_sensibility,0);
  
  OSCMessage get_timeout("/timeout");
  CScontrol(get_timeout,0);
  
  OSCMessage get_autocal("/autocal");
  CScontrol(get_autocal,0);
  
  OSCMessage get_loopDelay("/del");
  SystemControl(get_loopDelay,0);
  
  // some garde fou
  if ( boardParam.loopDelay > 1000 ) boardParam.loopDelay=0;
  if ( boardParam.cs_autocal > 30000 ) boardParam.cs_autocal=2000;
  if ( boardParam.cs_timeout > 2000 ) boardParam.cs_timeout=100;
  if ( boardParam.cs_sensibility > 2000 ) boardParam.cs_sensibility=30;
}
 
void saveParam(){
  eeprom_write_block(&boardParam, 0, sizeof(BoardParam));   
}

void loadParam(){
  eeprom_read_block(&boardParam, 0, sizeof(BoardParam)); 
  getAllParams();
}

void setup()                    
{
  Serial.begin(115200);
  while(!Serial); // wait until a serial connection is opened
  
  loadParam();
  offset = sprintf(path,"/b%02d",boardParam.address);

  for ( i=0; i<FSR_COUNT; i++){
    boardParam.fsr_enable[i]=true;
  }
  for ( i=0; i<CS_COUNT ; i++)
  {
    boardParam.cs_enable[i]=true;
    //~cs[i].set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off autocalibration
    //~cs[i].set_CS_AutocaL_Millis(boardParam.cs_autocal);
    //~cs[i].set_CS_Timeout_Millis(boardParam.cs_timeout);
    cs[i].set_CS_AutocaL_Millis(2000);
    cs[i].set_CS_Timeout_Millis(30);
  }
  boardParam.cs_on=true;
  boardParam.fsr_on=false;
}

void receiveOSC(){
  OSCBundle bundleIN;
  if ( SLIPSerial.available() > 0 ){
    while(!SLIPSerial.endofPacket()){
      int size=0;
      if( (size = SLIPSerial.available()) > 0)
      {
         while(size--)
            bundleIN.fill(SLIPSerial.read());
      }
    }
    
    if(!bundleIN.hasError()) {
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/fsr");
      bundleIN.route(path, FSRcontrol);
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs");
      bundleIN.route(path, CScontrol);
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/s");
      bundleIN.route(path, SystemControl);
    }
  }
    
}

void loop()
{
  if ( !Serial ){
    setup(); // if board is disconnected, "restart the sketch"
  }
  
  receiveOSC();

  end_scan = millis();
  // compute and send scan time over OSC
  snprintf(path+offset,MAX_STRING_LENGTH-offset,"/s/t");
  bundleOUT.add(path).add(end_scan - start_scan);
  start_scan = millis();
  if ( boardParam.cs_on ){
    int j = CS_COUNT;
    while ( boardParam.cs_enable[cs_id]==0 && j-- ) {
      cs_id++;
      cs_id%=CS_COUNT;      
    }
    if ( boardParam.cs_enable[cs_id] ){
        rawcount = cs[cs_id].capacitiveSensor(boardParam.cs_sensibility);
        normcount = rawcount / boardParam.cs_sensibility;
        snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/%d",cs_id);
        bundleOUT.add(path).add(normcount);
    }
    cs_id++;
    cs_id%=CS_COUNT;
  }

  if ( boardParam.fsr_on ){
    //~ put COMMON_PIN to LOW to use it as a reference for the FSR
    digitalWrite(COMMON_PIN,LOW);  
    for ( i=0;i<FSR_COUNT;i++){
      if ( boardParam.fsr_enable[i] ){
        // set an internal pull-up resistor on A0 pin
        digitalWrite(fsr_pin[i],HIGH);
        rawcount = analogRead(fsr_pin[i]);
        normcount = (1023 - rawcount) / 1023.;
        snprintf(path+offset,MAX_STRING_LENGTH-offset,"/fsr/%d",i);
        bundleOUT.add(path).add(normcount);
        // disable internal pull-up
        digitalWrite(fsr_pin[i],LOW);
      }
    }
  }
  
  bundleOUT.send(SLIPSerial);
  SLIPSerial.endPacket();
  bundleOUT.empty();
  
  delay(boardParam.loopDelay);
}
