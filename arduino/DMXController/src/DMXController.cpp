#include <Arduino.h>
#include <DMXSerial.h>
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

// - - - - -
// DmxSerial - A hardware supported interface to DMX.
// DMXSerialFlow.ino: Sample DMX application for sending 60 DMX values.
// Copyright (c) 2014 by Matthias Hertel, http://www.mathertel.de
// This work is licensed under a BSD style license. See http://www.mathertel.de/License.aspx
// 
// Documentation and samples are available at http://www.mathertel.de/Arduino
// 25.07.2011 creation of the DmxSerial library.
// 01.07.2013 published version of the example applikation
//
// This is an example of how to send a more complex RGB based pattern of colors
// over a DMX line.
// The values for one of the channels can be watched at the PWM outputs of the Arduino Board.
// - - - - -

byte channel, value;
OSCBundle bundleOUT;

void setup(void)
{
  Serial.begin(115200);

  DMXSerial.init(DMXController);
  
  for (int i=1; i<513;i++){
    DMXSerial.write(i,0);
  }
  
  // Set the number of channels the controller will send
  // this call is not needed, because the DMXController extends the DMX packet length automatically when data is added.
  DMXSerial.maxChannel(24);
  
  while(!Serial);
  bundleOUT.add("/dmx");
  bundleOUT.send(SLIPSerial);
  SLIPSerial.endPacket();

} // setup

void loop(void)
{
  while ( Serial.available() / 2 )
  { 
    channel=Serial.read();
    value=Serial.read();
    DMXSerial.write(channel+1,value);
  }
}

// End




