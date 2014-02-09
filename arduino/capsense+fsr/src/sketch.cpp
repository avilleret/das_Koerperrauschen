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


// global variables
static const int fsr_pin[] = {A0,A1,A2,A3,A4,A5,A11,A10,A9}; // FSR pin
#define FSR_COUNT 9

char path[MAX_STRING_LENGTH];
unsigned char offset=0;

long start_scan=0, end_scan=0;

bool cs_on = true, fsr_on = true; // disable CS for testing

/* global parameters - all should be saved */
char address=-1;
unsigned short cs_sensibility = 30;
unsigned short timeout = 30;

/* end of global parameters */

int i=0, cs_id;
long rawcount=0;
float normcount=0.;
OSCBundle bundleOUT;
int size=0;
int loopDelay=0;

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
bool fsr_enable[FSR_COUNT], cs_enable[CS_COUNT];

void recall(){ // recall all parameters from EEPROM
  address = EEPROM.read(0);
  //~timeout = EEPROM.read(1);
}
void save(){ // save all parameters to EEPROM
  EEPROM.write(0,address);
}

void FSRcontrol(OSCMessage &msg, int addrOffset)
{
  if (msg.fullMatch("/on",addrOffset)) {
    if (msg.isInt(0))
    {
      fsr_on = msg.getInt(0) > 0;
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/fsr/on");
      bundleOUT.add(path).add(fsr_on);
    }
  } else if (msg.fullMatch("/on",addrOffset)) {
    for ( int i = 0; i<msg.size() && i<FSR_COUNT; i++){
      if (msg.isInt(i))
      {
        fsr_enable[i] = msg.getInt(i) > 0;
      }
    }
  }
}

void CScontrol(OSCMessage &msg, int addrOffset)
{
  if (msg.fullMatch("/on",addrOffset)) {
    if (msg.size() == 1){
      if (msg.isInt(0)){
        cs_on = msg.getInt(0) > 0;
        snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/on");
        bundleOUT.add(path).add(cs_on);
      }
    } else if (msg.size()>1){
      for (i=0;i<msg.size() && i<CS_COUNT;i++){
        cs_enable[i] = msg.getInt(i) > 0;
      }
    }
  } else if ( msg.fullMatch("/sensibility", addrOffset) ) {
    if (msg.isInt(0))
    {
      cs_sensibility = msg.getInt(0);
      for ( i=0; i<CS_COUNT ; i++)
      {
        cs[i].reset_CS_AutoCal(); // reset all sensors
      }
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/sensibility");
      bundleOUT.add(path).add( (int) cs_sensibility);
    }
  }
  
  else if ( msg.fullMatch("/autocal", addrOffset) ) 
  {
    if (msg.isInt(0))
    {
      for ( i=0; i<CS_COUNT ; i++)
      {
        cs[i].set_CS_AutocaL_Millis(msg.getInt(0));
      }
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/autocal");
      bundleOUT.add(path).add(msg.getInt(0));
    } else {
      for ( i=0; i<CS_COUNT ; i++)
      {
        cs[i].reset_CS_AutoCal(); // reset all sensors
      }
    }
  } else if ( msg.fullMatch("/timeout", addrOffset) ) {
    if (msg.isInt(0))
    {
      for ( i=0; i<CS_COUNT ; i++)
      {
        cs[i].set_CS_Timeout_Millis(msg.getInt(0));
      }
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/timeout");
      bundleOUT.add(path).add(msg.getInt(0));
    }
  }
}

void SystemControl(OSCMessage &msg, int addrOffset){
  if (msg.fullMatch("/del",addrOffset)) { // set the delay on each loop
    if (msg.isInt(0))
    {
      loopDelay = msg.getInt(0)>0?msg.getInt(0):0;
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/s/del");
      bundleOUT.add(path).add(loopDelay);
    }
  } else {
    snprintf(path+offset,MAX_STRING_LENGTH-offset,"/s/date");
    bundleOUT.add(path).add(__DATE__);
    snprintf(path+offset,MAX_STRING_LENGTH-offset,"/s/time");
    bundleOUT.add(path).add(__TIME__);
  }
}

void setup()                    
{
  for ( i=0; i<FSR_COUNT; i++){
    fsr_enable[i]=true;
  }
  for ( i=0; i<CS_COUNT ; i++)
  {
    cs_enable[i]=false;
    //~cs[i].set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off autocalibration
    cs[i].set_CS_AutocaL_Millis(30000);
    cs[i].set_CS_Timeout_Millis(200);
  }
  cs_enable[8]=true;

  Serial.begin(115200);
  while(!Serial)
    ;
    
  recall();
  offset = sprintf(path,"/b%02d",address);
}

void receiveOSC(){
  OSCBundle bundleIN;
  if ( SLIPSerial.available() > 0 ){
    while(!SLIPSerial.endofPacket()){
      if( (size =SLIPSerial.available()) > 0)
      {
         while(size--)
            bundleIN.fill(SLIPSerial.read());
      }
    }
    
    
    if(!bundleIN.hasError()) {
      bundleIN.route("/fsr", FSRcontrol);
      bundleIN.route("/cs", CScontrol);
      bundleIN.route("/s", SystemControl);
    }
  }
    
}

void loop()
{
  receiveOSC();

  end_scan = millis();
  // compute and send scan time over OSC
  snprintf(path+offset,MAX_STRING_LENGTH-offset,"/s/t");
  bundleOUT.add(path).add(end_scan - start_scan);
  start_scan = millis();
  if ( cs_on ){
    //~i=cs_id;
    i=8;
    if ( cs_enable[i] ){
      rawcount = cs[i].capacitiveSensor(cs_sensibility);
      normcount = rawcount / cs_sensibility;
      snprintf(path+offset,MAX_STRING_LENGTH-offset,"/cs/%d",i);
      bundleOUT.add(path).add(normcount);
    }
    cs_id++;
    cs_id%=CS_COUNT;
  }

  if ( fsr_on ){
    //~ put COMMON_PIN to LOW to use it as a reference for the FSR
    digitalWrite(COMMON_PIN,LOW);  
    for ( i=0;i<FSR_COUNT;i++){
      if ( fsr_enable[i] ){
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
  
  delay(loopDelay);
}
