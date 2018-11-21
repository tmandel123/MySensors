/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************/
 

//	20181112 Version 1.32	millis() von "uint32_t now" nach "unsigned long currentTime" und wieder zurück
//	20181112 Version 1.33	Sleep_mode ausgebaut und DEBUG_PRINT eingeführt
//	20181114 Version 1.34	boolean informGW = false; - > bool informGW = false;
//										



// Enable debug prints
#define MY_DEBUG

// #define SER_DEBUG
#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x) DEBUG_PRINTLN(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif


// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_RF24_CHANNEL 96
#define MY_NODE_ID 100
#define MY_PARENT_NODE_ID 50
#define MY_REPEATER_FEATURE

#define MY_TRANSPORT_WAIT_READY_MS (5000ul)

#include <MySensors.h>

#define SKETCH_NAME					"EnergyMeter"		// Optional child sensor name
#define SKETCH_VER					"1.33"				// Sketch version



#define DIGITAL_INPUT_SENSOR 3  	// The digital input you attached your light sensor.  (Only 2 and 3 generates interrupt!)
#define PULSE_FACTOR 1000       	// Nummber of blinks per KWH of your meeter
#define MAX_WATT 12000          	// Max watt value to report. This filetrs outliers.
#define CHILD_ID 1              	// Id of the sensor child
#define CHILD_NAME_1				"PowerMeter Child"


#define HEARTBEAT_INTERVAL			300000				//später alle 5 Minuten, zum Test alle 30 Sekunden

uint32_t SEND_FREQUENCY = 20000; // Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
double ppwh = ((double)PULSE_FACTOR)/1000; // Pulses per watt hour
bool pcReceived = false;
bool informGW = false;
volatile uint32_t pulseCount = 0;
volatile uint32_t lastBlink = 0;
volatile uint32_t watt = 0;
uint32_t oldPulseCount = 0;
uint32_t oldWatt = 0;
double oldKwh;
uint32_t lastSend;
uint32_t lastHeartBeat = 0;



MyMessage wattMsg(CHILD_ID,V_WATT);
MyMessage kwhMsg(CHILD_ID,V_KWH);
MyMessage pcMsg(CHILD_ID,V_VAR1);


void setup()
{
	// Fetch last known pulse count value from gw
	request(CHILD_ID, V_VAR1);

	// Use the internal pullup to be able to hook up this sketch directly to an energy meter with S0 output
	// If no pullup is used, the reported usage will be too high because of the floating pin
	pinMode(DIGITAL_INPUT_SENSOR,INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(DIGITAL_INPUT_SENSOR), onPulse, RISING);
	lastSend=millis();
}

void presentation()
{

	sendSketchInfo(SKETCH_NAME, SKETCH_VER); 
	// Register this device as power sensor
	present(CHILD_ID, S_POWER, CHILD_NAME_1);
}

void loop()
{
	uint32_t currentTime = millis();
	if (currentTime - lastHeartBeat > (uint32_t)HEARTBEAT_INTERVAL)
	{
		sendHeartbeat();	
		lastHeartBeat = currentTime;

	}
	
	if (informGW)
	{
		informGW = false;
		send(pcMsg.set(pulseCount));
	}
	// Only send values at a maximum frequency or woken up from sleep
	bool sendTime = currentTime - lastSend > SEND_FREQUENCY;
	if (pcReceived && sendTime) 
	{
		// New watt value has been calculated
		if (watt != oldWatt) 
		{
			// Check that we dont get unresonable large watt value.
			// could hapen when long wraps or false interrupt triggered
			if (watt<((uint32_t)MAX_WATT)) {
				send(wattMsg.set(watt));  // Send watt value to gw
			}
			DEBUG_PRINT("Watt:");
			DEBUG_PRINTLN(watt);
			oldWatt = watt;
		}

		// Pulse cout has changed
		if (pulseCount != oldPulseCount) 
		{
			send(pcMsg.set(pulseCount));  // Send pulse count value to gw
			double kwh = ((double)pulseCount/((double)PULSE_FACTOR));
			oldPulseCount = pulseCount;
			if (kwh != oldKwh) {
				send(kwhMsg.set(kwh, 4));  // Send kwh value to gw
				oldKwh = kwh;
			}
		}
		lastSend = currentTime;
	} 
	else if (sendTime && !pcReceived) 
	{
		// No count received. Try requesting it again
		request(CHILD_ID, V_VAR1);
		lastSend=currentTime;
	}

}

void receive(const MyMessage &message)
{
	if (message.sensor == CHILD_ID)
	{
		if (message.type==V_VAR1) {
			pulseCount = oldPulseCount = message.getLong();
			DEBUG_PRINT("Received last pulse count from gw:");
			DEBUG_PRINTLN(pulseCount);
			pcReceived = true;
			informGW = true;
		}
	}
}

void onPulse()
{
	uint32_t newBlink = micros();
	uint32_t interval = newBlink-lastBlink;
	if (interval<10000L) { // Sometimes we get interrupt on RISING
		return;
	}
	watt = (3600000000.0 /interval) / ppwh;
	lastBlink = newBlink;
	pulseCount++;
}
