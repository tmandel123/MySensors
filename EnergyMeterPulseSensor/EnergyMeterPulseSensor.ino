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
 

//	20181112 Version 1.32		millis() von "uint32_t now" nach "unsigned long currentTime" und wieder zurück
//	20181112 Version 1.33		Sleep_mode ausgebaut und DEBUG_PRINT eingeführt
//	20181114 Version 1.34		boolean informGW = false; - > bool informGW = false;
//	20181114 Version 1.4-003	MY_REPEATER_FEATURE deaktiviert und sendSketchInfo(SKETCH_NAME, SKETCH_VER " " __TIME__ " " __DATE__);
//



// Enable debug prints
// #define MY_DEBUG

#define SER_DEBUG



// Enable and select radio type attached
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 96
#define MY_NODE_ID 100
#define MY_PARENT_NODE_ID 50
// #define MY_REPEATER_FEATURE									//	2019-01-23	MY_REPEATER_FEATURE deaktiviert weil dieser Sensor als einziger instabil läuft. 
																//	Es gibt immer wieder Aussetzer von mehreren Stunden. Danach läuft alles wieder normal. Zählerwerte gehen jedoch nicht verloren. Laufzeit akutell 36 Tage.

#define MY_TRANSPORT_WAIT_READY_MS (5000ul)

#include <MySensors.h>

#define SKETCH_NAME						"EnergyMeter"			// Optional child sensor name
#define SKETCH_VER						"1.4-003"				// Sketch version



#define DIGITAL_INPUT_SENSOR			3					// The digital input you attached your light sensor.  (Only 2 and 3 generates interrupt!)
#define PULSE_FACTOR					1000				// Nummber of blinks per KWH of your meeter
#define MAX_WATT						12000				// Max watt value to report. This filetrs outliers.
#define CHILD_ID						1					// Id of the sensor child
#define CHILD_NAME_1					"PowerMeter Child"
#define CHILD_ID_ANALOG					2					//ID für Threshold Werte und setzten der EEPROM MeterValue
#define CHILD_ID_DEBUG					3					// Debug setzen und lesen


#define EEPROM_DEBUGLEVEL				0					//8  Bit
#define EEPROM_METER_VALUE				1					//32 Bit

// Input and output definitions
// #define ANALOG_INPUT_SENSOR				A0					// The analog input you attached your sensor. 
#define UPLINK_LED						5
#define PULSE_LED						6
#define SEND_WAIT						50
#define REQUEST_ACK						true

// Sonstige Werte
#define HEARTBEAT_INTERVAL				300000				//später alle 5 Minuten, zum Test alle 30 Sekunden
#define INTERNALS_UPDATE_INTERVAL		3600000				//jede Stunde Update senden (Debug, Threshold usw)
#define FLOW_TO_ZERO_TIME				120000				//120000

#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif

// bool pcReceived = false;
bool informGW = false;
bool TransportUplink = true;
bool firstLoop = true;

uint8_t debugLevel = 0;										// sets the debug level, 0 = basic info. 1 = streaming level info. 2 = sent level streaming to gateway.

uint32_t SEND_FREQUENCY = 20000; // Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
uint32_t oldPulseCount = 0;
uint32_t oldWatt = 0;
uint32_t lastSend;
uint32_t lastHeartBeat = 0;
uint32_t lastInternalsUpdate = 0; 
// uint32_t lastPulseTime = 0;


volatile uint32_t pulseCount = 0;
volatile uint32_t lastPulseTime = 0;
volatile uint32_t watt = 0;

// double ppwh = ((double)PULSE_FACTOR)/1000; // Pulses per watt hour
double oldKwh;



MyMessage wattMsg(CHILD_ID,V_WATT);
MyMessage kwhMsg(CHILD_ID,V_KWH);
MyMessage lastCounterMsg(CHILD_ID,V_VAR1);

// MyMessage analogValue	(CHILD_ID_ANALOG, V_VAR1);
// MyMessage MsgMinValue	(CHILD_ID_ANALOG, V_VAR2);
// MyMessage MsgMaxValue	(CHILD_ID_ANALOG, V_VAR3);
MyMessage MeterValue	(CHILD_ID_ANALOG, V_VAR5);

MyMessage debugValue	(CHILD_ID_DEBUG, V_VAR1);
// MyMessage thValueMin	(CHILD_ID_DEBUG, V_VAR2);
// MyMessage thValueMax	(CHILD_ID_DEBUG, V_VAR3);
MyMessage hwTime		(CHILD_ID_DEBUG, V_VAR4);
MyMessage Uplink		(CHILD_ID_DEBUG, V_VAR5);

void preHwInit() 
{
	DEBUG_SERIAL(115200);
}

void before() 
{
	DEBUG_PRINTLN("before: ");
	DEBUG_PRINT(SKETCH_NAME);
	DEBUG_PRINT(" ");
	DEBUG_PRINTLN(SKETCH_VER);
}

void setup()
{
	DEBUG_PRINTLN("setup: ");
	hwPinMode(UPLINK_LED, OUTPUT);
	hwPinMode(PULSE_LED, OUTPUT);
	// writeEeprom32(EEPROM_METER_VALUE, 6787639);

	uint32_t MeterValue = readEeprom32(EEPROM_METER_VALUE);
	DEBUG_PRINT("readEeprom32: EEPROM_METER_VALUE ");
	DEBUG_PRINTLN(MeterValue);
	pulseCount = MeterValue;
	oldPulseCount = MeterValue;

	
	pinMode(DIGITAL_INPUT_SENSOR,INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(DIGITAL_INPUT_SENSOR), onPulse, RISING);
	lastHeartBeat=millis();
	lastSend=lastHeartBeat;
	lastPulseTime = lastSend;
	lastInternalsUpdate = lastSend;
	
	// Fetch debug level from EEPROM
	debugLevel = loadState(EEPROM_DEBUGLEVEL); //8 Bit
	// debugMessage("Debug level fetched from EEPROM, value: ", String(debugLevel));
	
	if (debugLevel == 255)//vermutlich wurde das EEPROM an dieser Stelle noch nicht beschrieben
	{
		debugLevel=0;
		saveState(EEPROM_DEBUGLEVEL,debugLevel);
		// debugMessage("Debug level defaults stored to  EEPROM, value: ", String(debugLevel));
	}
	
}

void presentation()
{

	// sendSketchInfo(SKETCH_NAME, SKETCH_VER); 
	sendSketchInfo(SKETCH_NAME, SKETCH_VER " " __TIME__ " " __DATE__);
	// Register this device as power sensor
	present(CHILD_ID, S_POWER, CHILD_NAME_1);
	present(CHILD_ID_ANALOG, S_CUSTOM, "Set/Get MeterValue EEPROM Child");
	present(CHILD_ID_DEBUG, S_CUSTOM, "Debug Set/Get Child");
}

void loop()
{
	uint32_t currentTime = millis();
	uint32_t TimeSinceHeartBeat = currentTime - lastHeartBeat;
	// uint32_t TimeSinceLastPulse = currentTime - lastPulseTime;
	
	if (TransportUplink)
	{
		digitalWrite(UPLINK_LED,HIGH);
	}
	else
	{
		digitalWrite(UPLINK_LED,LOW);
	}
	
	if (((TimeSinceHeartBeat > (uint32_t)HEARTBEAT_INTERVAL)) || firstLoop || informGW)
	{
		TransportUplink = send(hwTime.set(currentTime), REQUEST_ACK);
		DEBUG_PRINT("Uplink Check: ");
		DEBUG_PRINTLN(TransportUplink);	
		wait(SEND_WAIT);
		send(Uplink.set(TransportUplink));		
	
		lastHeartBeat = currentTime;

		if (informGW)
		{
			informGW = false;
			send(lastCounterMsg.set(pulseCount));
		}
	}

	//Serviceroutine, welche alle 60 Minuten läuft um Werte für FHEM Grafik aktuell zu halten
	if ((currentTime - lastInternalsUpdate > (uint32_t)INTERNALS_UPDATE_INTERVAL) || firstLoop)
	{
		if (firstLoop)
		{
			// DEBUG_PRINTLN("firstLoop IT");
			firstLoop = false;
		}
		else
		{
			// DEBUG_PRINTLN("IT: Write EEPROM_METER_VALUE");
			writeEeprom32(EEPROM_METER_VALUE, pulseCount);	
		}
		send(debugValue.set(debugLevel),REQUEST_ACK);
		wait(SEND_WAIT);
		send(MeterValue.set(pulseCount),REQUEST_ACK);
		wait(SEND_WAIT);
		
		lastInternalsUpdate = currentTime;
	}	
	
	// Only send values at a maximum frequency or woken up from sleep
	if (currentTime - lastSend > SEND_FREQUENCY) 
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
			digitalWrite(PULSE_LED,HIGH);
			send(lastCounterMsg.set(pulseCount));  // Send pulse count value to gw
			double kwh = ((double)pulseCount/((double)PULSE_FACTOR));
			oldPulseCount = pulseCount;
			if (kwh != oldKwh) {
				send(kwhMsg.set(kwh, 4));  // Send kwh value to gw
				oldKwh = kwh;
			}
		}
		else
		{
			digitalWrite(PULSE_LED,LOW);
		}
		lastSend = currentTime;
	} 
}

void receive(const MyMessage &message)
{
	if ((message.sensor == CHILD_ID_ANALOG) && !mGetAck(message))
	{
		switch (message.type) 
		{
			case V_VAR5: 
			{
				noInterrupts(); //für den nächsten Zuweisungen die Interrupts deaktiveren
				pulseCount = message.getULong();
				oldPulseCount = pulseCount;
				DEBUG_PRINTLN("new MeterValue from GW");
				watt = oldWatt = 0;
				informGW = true;
				writeEeprom32(EEPROM_METER_VALUE, pulseCount);
				interrupts();// Interrupts wieder aktiveren
			}
		}
	}
	
	// if (message.sensor == CHILD_ID)
	// {
		// if (message.type==V_VAR1) {
			// pulseCount = oldPulseCount = message.getLong();
			// DEBUG_PRINT("Received last pulse count from gw:");
			// DEBUG_PRINTLN(pulseCount);
			// informGW = true;
		// }
	// }
}

void onPulse()
{
	uint32_t newPulseTime = micros();
	uint32_t interval = newPulseTime-lastPulseTime;
	// if (interval<10000L) { // Sometimes we get interrupt on RISING
	if (interval<1000L) { // Sometimes we get interrupt on RISING
		return;
	}
	// watt = (3600000000.0 /interval) / ppwh;
	watt = (3600000000.0 /interval);
	lastPulseTime = newPulseTime;
	pulseCount++;
}




// void writeEeprom16(uint8_t pos, uint16_t value) 
// {
  // saveState(pos, ((uint16_t)value >> 8));
  // pos++;
  // saveState(pos, (value & 0xff));
// }

// uint16_t readEeprom16(uint8_t pos) 
// {
  // uint16_t hiByte;
  // uint16_t loByte;
  // hiByte = loadState(pos) << 8;
  // pos++;
  // loByte = loadState(pos);
  // return (hiByte | loByte);
// }


//This function will write a 4 byte (32bit) uint32_t to the eeprom at
//the specified pos to pos + 3.
void writeEeprom32(int pos, uint32_t value)
{
	//Decomposition from a uint32_t to 4 bytes by using bitshift.
	//One = Most significant -> Four = Least significant byte
	byte four = (value & 0xFF);
	byte three = ((value >> 8) & 0xFF);
	byte two = ((value >> 16) & 0xFF);
	byte one = ((value >> 24) & 0xFF);

	//Write the 4 bytes into the eeprom memory.
	saveState(pos, four);
	saveState(pos + 1, three);
	saveState(pos + 2, two);
	saveState(pos + 3, one);
}

uint32_t readEeprom32(int pos)
{
	//Read the 4 bytes from the eeprom memory.
	uint32_t four = loadState(pos);
	uint32_t three = loadState(pos + 1);
	uint32_t two = loadState(pos + 2);
	uint32_t one = loadState(pos + 3);

	//Return the recomposed uint32_t by using bitshift.
	return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}  

void showEEprom()
{
	DEBUG_PRINTLN("showEEprom()");
	byte counter=0;
	byte Zeichen;
	for (byte i = 0; i<16; i++)
	{
		for (byte j = 0; j<16; j++)
		{
			Zeichen=loadState(counter);
			if (Zeichen < 16)
			{
				Serial.print("0");
				
			}
			Serial.print(String(Zeichen,HEX));
			if (j < 15)
			{
				Serial.print("-");
			}
			counter++;
		}	
		DEBUG_PRINTLN("");
	}
}