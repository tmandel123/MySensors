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
 
 
// 20161012 Verison 1.2		IsAlive hinzugefügt
// 20161110 Version 1.3		Entprellen optimiert
// 20170221 Version 2.0		Umstellung auf Analog
// 20170310 Version 2.1		Optimierung der Min/Max Werte
// 20180102 Version 2.2		Codeoptimierung, MinSend alle 10 Minuten funktioniert immer noch nicht
// 20180205 Version 2.3.1	Umstellung auf Lib 2.2.0, RepeaterFeature deaktiviert
//							HEARTBEAT alle 5 Minuten
// 20181112 Version 2.3.2	Umstellung auf Lib 2.3.0
//							bei receive() #defines für CHILD_ID und CHILD_ID_DEBUG verwendet (vorher 1 und 3 fest eingebaut)
// 20181113 Version 2.4.0	Gas und Water Meter Sketche zusammengeführt. Mit #define WATER wird Wasserzähler aktiviert. Durch Auskommentieren der Gaszähler
//							unsigned int sensorValue; (vorher nur int)
//							debuglevel bei EEPROM Wert von 255 (also bisher nicht beschrieben) auf 0 setzten und ins EEPROM schreiben

//ToDo: 	DEBUG_SERIAL(x) statt debugMessage

//ToDo
// von boolean nach bool ändern
// alle 60 Minuten den leztten PulseCount im EEPROM speichern, dazu einen eigenen ChildNode verwenden (oder V_VAR5) 
// diesen PulseCound nicht im Setup abfragen, sondern einfach weiterzählen. ggf aber diesen Pulscound von FHEM aus setzten lassen
// alle 10 Minuten ein IsAlive schicken(z.B. aktuellen PulseCount)
// Statt delay besser Mysensors wait Kommando benutzen
// int high = readEeprom(0);//16 Bit --> besser unsigned int. Die Funktion int readEeprom(int pos)  müsste dann auch auf unsigned int geändert werden
// regelmäßig AnalogValue und Debugvalue senden (alle 60 Minuten)
// testen, ob der Sketch weiterzählt, auch wenn kein Gateway verfügbar ist


#define SKETCH_VER						"2.4.1"				// Sketch version

#define MY_RADIO_NRF24
#define MY_DEBUG //muss vor MySensors.h stehen
#define MY_REPEATER_FEATURE
#define MY_NODE_ID 102
#define MY_RF24_CHANNEL 1									// Nach Testphase deaktivieren, damit Kanal 76 aktiv wird


#include <SPI.h>
#include <MySensors.h>  

// #define WATER
#define GAS

#ifdef WATER
	#define SKETCH_NAME					"Water Meter"		// Optional child sensor name
	#define PULSE_FACTOR				1000				// Number of blinks per m3 of your meter (One rotation/liter)
	#define MAX_FLOW					40					// Max flow (l/min) value to report. This filters outliers.
	#define CHILD_NAME					"Watermeter"		// Optional child sensor name
	volatile uint16_t minValue = 		40;
	volatile uint16_t maxValue = 		250;
	volatile uint16_t highThreshold = 	50;					// higher threshold for analog readings
	volatile uint16_t lowThreshold = 	33;					// lower threshold for analog readings
#else
	#define SKETCH_NAME					"Gas Meter"			// Optional child sensor name
	#define PULSE_FACTOR				1000				// Number of blinks per m3 of your meter (One rotation/liter)
	#define MAX_FLOW					40					// Max flow (l/min) value to report. This filters outliers.
	#define CHILD_NAME					"Gasmeter"			// Optional child sensor name
	volatile uint16_t minValue = 		267;
	volatile uint16_t maxValue = 		513;
	volatile uint16_t highThreshold =	430;					// higher threshold for analog readings
	volatile uint16_t lowThreshold =	350;					// lower threshold for analog readings
#endif


// Node and sketch information

#define CHILD_ID						1					// Id of the sensor child
#define CHILD_ID_ANALOG					2					//ID für Threshold Werte
#define CHILD_ID_DEBUG					3					// Debug setzen und lesen


// Input and output definitions
#define ANALOG_INPUT_SENSOR				A0					// The analog input you attached your sensor. 

// Sonstige Werte
#define HEARTBEAT_INTERVAL				300000				//später alle 5 Minuten, zum Test alle 30 Sekunden
#define INTERNALS_UPDATE_INTERVAL		3600000				//jede Stunde Update senden (Debug, Threshold usw)
#define SEND_FREQUENCY					30000



#define EEPROM_HI_THRESHOLD				0 					//16 Bit
#define EEPROM_LO_THRESHOLD				2					//16 Bit
#define EEPROM_DEBUGLEVEL				4					//8Bit


MyMessage flowMsg(CHILD_ID, V_FLOW);
MyMessage volumeMsg(CHILD_ID, V_VOLUME);
MyMessage lastCounterMsg(CHILD_ID, V_VAR1); 				//dieser Wert wird vom Controller abgefragt und stellt den Zählerstand dar

MyMessage analogValue(CHILD_ID_ANALOG, V_VAR1);
MyMessage MsgMinValue(CHILD_ID_ANALOG, V_VAR2);
MyMessage MsgMaxValue(CHILD_ID_ANALOG, V_VAR3);

MyMessage debugValue (CHILD_ID_DEBUG, V_VAR1);
MyMessage thValueMin (CHILD_ID_DEBUG, V_VAR2);
MyMessage thValueMax (CHILD_ID_DEBUG, V_VAR3);

// Global vars
// unsigned long sendFrequency = 30000;							// Minimum time between send (in milliseconds). We don't want to spam the gateway.


volatile uint16_t midValue = (lowThreshold+highThreshold)/2;	// Mittelwert von Threshold Max und Min (Soll minValue und maxValue begrenzen)
volatile uint16_t debugLevel = 0;								// sets the debug level, 0 = basic info. 1 = streaming level info. 2 = sent level streaming to gateway.
volatile uint32_t pulseCount = 0;
volatile uint32_t lastBlink = 0;
volatile double flow = 0;
boolean pcReceived = false;
boolean informGW = false;
boolean sensorState;

uint32_t oldPulseCount = 0;
uint32_t newBlink = 0;
uint32_t lastSend = 0;
uint32_t lastPulse = 0;
uint32_t lastHeartBeat = 0;
uint32_t lastInternalsUpdate = INTERNALS_UPDATE_INTERVAL-15000; //15 Sec nach Start Werte aktualisieren
uint16_t sensorValue;

double ppl = ((double)PULSE_FACTOR) / 1000;					// Pulses per liter
double oldflow = 0;
double volume = 0;
double oldvolume = 0;



void setup()
{

	debugMessage("Setup: ","Start");
	pulseCount = oldPulseCount = 0;

	lastHeartBeat = lastSend = lastPulse = millis();

	// Fetch debug level from EEPROM
	debugLevel = loadState(EEPROM_DEBUGLEVEL); //8 Bit
	debugMessage("Debug level fetched from EEPROM, value: ", String(debugLevel));
	
	if (debugLevel == 255)//vermutlich wurde das EEPROM an dieser Stelle noch nicht beschrieben
	{
		debugLevel=0;
		saveState(EEPROM_DEBUGLEVEL,debugLevel);
		debugMessage("Debug level defaults stored to  EEPROM, value: ", String(debugLevel));
	}

	// Fetch the last set thresholds from EEPROM
	uint16_t high = readEeprom(EEPROM_HI_THRESHOLD);//16 Bit , weil 1024 nicht in 8 Bit reinpasst
	uint16_t low = readEeprom(EEPROM_LO_THRESHOLD); //16 Bit
	if (high == 0 || high == 65535 || low == 0 || low == 65535) 
	{
		
		debugMessage("High threshold set to standard value: ", String(highThreshold));
		debugMessage("Low threshold set to standard value: ", String(lowThreshold));
	}
	else 
	{
		highThreshold = high;
		lowThreshold = low;
		debugMessage("High threshold fetched from EEPROM, value: ", String(highThreshold));
		debugMessage("Low threshold fetched from EEPROM, value: ", String(lowThreshold));
	}
	midValue=uint16_t((lowThreshold+highThreshold)/2);
	debugMessage("midValue: ", String(midValue));
	debugMessage("Setup: ","End");
}


void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_VER);     
  present(CHILD_ID, S_WATER, CHILD_NAME);       
  present(CHILD_ID_ANALOG, S_CUSTOM, "Analog Get Child");
  present(CHILD_ID_DEBUG, S_CUSTOM, "Debug Set/Get Child");
}

void loop()
{
	uint32_t currentTime = millis();
	if (currentTime - lastHeartBeat > (uint32_t)HEARTBEAT_INTERVAL)
	{
		sendHeartbeat();	
		lastHeartBeat = currentTime;
		if (informGW)
		{
			informGW = false;
			send(lastCounterMsg.set(pulseCount));
		}
	}
	//Serviceroutine, welche alle 60 Minuten läuft um Werte für FHEM Grafik aktuell zu halten
	if (currentTime - lastInternalsUpdate > (uint32_t)INTERNALS_UPDATE_INTERVAL)
	{
		//Min und Max etwas Näher an den Durchschnitt heranziehen, gibt es auch bei der Flowberechnung
		if (minValue < (midValue - 4 ))
		{
				  minValue++;
		}
		if (maxValue > (midValue + 4 ))
		{
		  maxValue--;
		}
		
		send(debugValue.set(debugLevel));
		send(thValueMin.set(lowThreshold));
		send(thValueMax.set(highThreshold));
		send(analogValue.set(sensorValue));
		send(MsgMinValue.set(minValue));
		send(MsgMaxValue.set(maxValue));
		send(flowMsg.set(flow, 2));
		send(lastCounterMsg.set(pulseCount));
		send(volumeMsg.set(volume, 3));
		


		lastInternalsUpdate = currentTime;
	}
	// Check the analog sensor values an change state when thresholds are passed
	checkThreshold();

	// Only send values at a maximum frequency
	if (currentTime - lastSend > (uint32_t)SEND_FREQUENCY) 
	{
		lastSend = currentTime;

		if (!pcReceived) {
			//Last Pulsecount not yet received from controller, request it again
			debugMessage("Request: ","Start");
			request(CHILD_ID, V_VAR1);
			return;
		}
    
		if (flow != oldflow) {
			oldflow = flow;
			debugMessage("l/min:", String(flow));

			// Check that we dont get unresonable large flow value. 
			// could hapen when long wraps or false interrupt triggered
			if (flow < ((uint32_t)MAX_FLOW)) {
				// Send flow value to gw
				send(flowMsg.set(flow, 2));
				send(MsgMinValue.set(minValue));
				send(MsgMaxValue.set(maxValue));

				//Min und Max etwas Näher an den Durchschnitt heranziehen
				if (minValue < (midValue - 4 ))
				{
						  minValue++;
				}
				if (maxValue > (midValue + 4 ))
				{
				  maxValue--;
				}
				debugMessage("MinNeu:", String(minValue));
				debugMessage("MaxNeu:", String(maxValue));
				
			}
		}

		// No Pulse count received in 2min 
		if (currentTime - lastPulse > 120000) {
			flow = 0;
		}

		// Pulse count has changed
		if (pulseCount != oldPulseCount) 
		{
			oldPulseCount = pulseCount;
			// if (debugLevel < 2) {
				debugMessage("pulsecount: ", String(pulseCount));
				// Send  pulsecount value to gw in VAR1
				send(lastCounterMsg.set(pulseCount));
			// }
			double volume = ((double)pulseCount / ((double)PULSE_FACTOR));
			if (volume != oldvolume)
			{
				oldvolume = volume;
				// if (debugLevel == 2) {
					debugMessage("volume: ", String(volume, 3));
					// Send volume value to gw
					send(volumeMsg.set(volume, 3));
				// }
			}
		}
	}

}

void debugMessage(String header, String content)
{
	// DEBUG code ------
	Serial.print(header);
	Serial.println(content);
	// DEBUG code ------   
}

void receive(const MyMessage &message)
{
	debugMessage("Receiver: ", String(message.sensor));
	if (message.sensor == CHILD_ID)
	{
		switch (message.type) 
		{
			case V_VAR1: 
			{
				// unsigned long gwPulseCount = message.getULong();
				// pulseCount = gwPulseCount;
				pulseCount = message.getULong();
				flow = oldflow = 0;
				debugMessage("Received last pulse count from gw: ", String(pulseCount));
				pcReceived = true;
				informGW = true;
			}
		}
	}
	else if (message.sensor == CHILD_ID_DEBUG)
	{	
		switch (message.type) 
		{
			case V_VAR1: 
			{
				debugLevel = message.getULong();
				saveState(EEPROM_DEBUGLEVEL, debugLevel);//8 Bit
				debugMessage("Received new debug state from gw: ", String(debugLevel));
			}
			break;
			case V_VAR2: 
			{
				lowThreshold = message.getULong();
				storeEeprom(EEPROM_LO_THRESHOLD, lowThreshold); //16 Bit
				debugMessage("Received new low threshold from gw: ", String(lowThreshold));
			}
			break;
			case V_VAR3: 
			{
				highThreshold = message.getULong();
				storeEeprom(EEPROM_HI_THRESHOLD, highThreshold); // 16 Bit
				debugMessage("Received new high threshold from gw: ", String(highThreshold));
			}
			break;
		}
  	}
	else
	{
		debugMessage("Received invalid message from gw! ", "");
	}
    midValue=uint16_t((lowThreshold+highThreshold)/2);

}


void newPulse()
{
	uint32_t newBlink = millis();
	uint32_t interval = newBlink - lastBlink;
	if (debugLevel > 0)
	{
		debugMessage("new Pulse Interval since last ", String(interval));
	}

	if (interval != 0) 
	{
		lastPulse = millis();
		flow = (60000.0 / interval) / ppl;
	}
	lastBlink = newBlink;

	pulseCount++;
}

void checkThreshold() {
  
  sensorValue = getAverage();
  if (minValue>=sensorValue)
  {
    minValue=sensorValue;
  }
  if (maxValue<=sensorValue)
  {
    maxValue=sensorValue;
  }
 
	if (debugLevel > 0) 
	{
		debugMessage("val = ", String(sensorValue, DEC));
		debugMessage("min = ", String(minValue, DEC));
		debugMessage("max = ", String(maxValue, DEC));
		send(debugValue.set(debugLevel));
		send(thValueMin.set(lowThreshold));
		send(thValueMax.set(highThreshold));
    
		if (debugLevel == 1) 
		{
			send(analogValue.set(sensorValue));
			wait(800);
		}
		
		if (debugLevel == 2) 
		{
			send(analogValue.set(sensorValue));
			send(MsgMinValue.set(minValue));
			send(MsgMaxValue.set(maxValue));
			wait(800);
		}

		if (debugLevel == 3) 
		{
		  debugMessage("flow dummy: ", "123.123");
		  debugMessage("volume dummy: ", "456.45");
		  send(volumeMsg.set(123.123, 3));
		  send(flowMsg.set(456.45, 2));
		  wait(800);
		}
	}
	if ((sensorState == true) && (sensorValue < lowThreshold))
	{
		newPulse();
		sensorState = !sensorState;
		debugMessage("pulsCount + 1", "");
		debugMessage("Sensor state: ", String(sensorState));
	}
	if ((sensorState == false) && (sensorValue > highThreshold)) 
	{
		sensorState = !sensorState;
		debugMessage("Sensor state: ", String(sensorState));
	}
}

int getAverage()
{
	// hier wird 100 Mal eingelesen. 10 Mal hintereinander analogRead ohne Pause. Dann 50ms Pause. Dies wird dann 10 Mal wiederholt
	uint8_t cycles = 0;
	uint16_t val = 0;     // variable to store current values from the input
	uint16_t newVal = 0;      // variable to store new values from the input
	uint16_t average = 0;    // variable to store the peak value
	uint8_t count = 0;       // variable for loop

						 // take 10 samples over half a second 
						 // for each sample the input is taken 10 times
	for (cycles = 0; cycles < 10; cycles++) {
		// read input 10 times and get the sum
		// "ANALOG_INPUT_SENSOR" was defined in main program as shown below:
		// "#define ANALOG_INPUT_SENSOR A0"
		for (count = 0; count < 10; count++) {
			val = val + analogRead(ANALOG_INPUT_SENSOR);
		}
		// get average of readings
		val = (val / 10);

		// add reading to newVal
		newVal += val;
		val = 0;
		// measure samples over half a second or "newVal" will
		// almost always be the same resulting in wrong average
		wait(10); // 10 cycles of 50ms gives  10 samples in 500ms
	}
	// set average to average of newValue1
	average = newVal / 10;

	// return the value of average to main program
	return average;
}



void storeEeprom(uint8_t pos, uint16_t value) 
{
  // function for saving the values to the internal EEPROM
  // value = the value to be stored (as int)
  // pos = the first byte position to store the value in
  // only two bytes can be stored with this function (max 32.767)
  saveState(pos, ((uint16_t)value >> 8));
  pos++;
  saveState(pos, (value & 0xff));
  Serial.print("storeEeprom: Pos ");
  Serial.print(pos);
  Serial.print("Value ");
  Serial.println(value);
}

uint16_t readEeprom(uint8_t pos) 
{
  // function for reading the values from the internal EEPROM
  // pos = the first byte position to read the value from 

  uint16_t hiByte;
  uint16_t loByte;

  hiByte = loadState(pos) << 8;
  pos++;
  loByte = loadState(pos);
  
  Serial.print("readEeprom: Pos ");
  Serial.print(pos);
  Serial.print(" Value ");
  Serial.println(hiByte | loByte);
  
  return (hiByte | loByte);
}

/* Dies muss noch auf die MySensors Variante (loadState, saveState) umgeschrieben werden, damit im EEPROM nicht die Werte für NodeID und Co überschrieben werden
//This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to address + 3.
void EEPROMWritelong(int address, long value)
{
	//Decomposition from a long to 4 bytes by using bitshift.
	//One = Most significant -> Four = Least significant byte
	byte four = (value & 0xFF);
	byte three = ((value >> 8) & 0xFF);
	byte two = ((value >> 16) & 0xFF);
	byte one = ((value >> 24) & 0xFF);

	//Write the 4 bytes into the eeprom memory.
	EEPROM.write(address, four);
	EEPROM.write(address + 1, three);
	EEPROM.write(address + 2, two);
	EEPROM.write(address + 3, one);
}

long EEPROMReadlong(long address)
{
	//Read the 4 bytes from the eeprom memory.
	long four = EEPROM.read(address);
	long three = EEPROM.read(address + 1);
	long two = EEPROM.read(address + 2);
	long one = EEPROM.read(address + 3);

	//Return the recomposed long by using bitshift.
	return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}  
*/

/*
 
 __  __       ____
|  \/  |_   _/ ___|  ___ _ __  ___  ___  _ __ ___
| |\/| | | | \___ \ / _ \ `_ \/ __|/ _ \| `__/ __|
| |  | | |_| |___| |  __/ | | \__ \  _  | |  \__ \
|_|  |_|\__, |____/ \___|_| |_|___/\___/|_|  |___/
        |___/                      2.3.0

16 MCO:BGN:INIT REPEATER,CP=RNNRA---,VER=2.3.0
26 TSM:INIT
28 TSF:WUR:MS=5000
36 TSM:INIT:TSP OK
38 TSM:INIT:STATID=102
40 TSF:SID:OK,ID=102
43 TSM:FPAR
79 TSF:MSG:SEND,102-102-255-255,s=255,c=3,t=7,pt=0,l=0,sg=0,ft=0,st=OK:
2091 !TSM:FPAR:NO REPLY
2093 TSM:FPAR
2129 TSF:MSG:SEND,102-102-255-255,s=255,c=3,t=7,pt=0,l=0,sg=0,ft=0,st=OK:
2928 TSF:MSG:READ,0-0-102,s=255,c=3,t=8,pt=1,l=1,sg=0:0
2934 TSF:MSG:FPAR OK,ID=0,D=1
4139 TSM:FPAR:OK
4139 TSM:ID
4141 TSM:ID:OK
4143 TSM:UPL
4147 TSF:MSG:SEND,102-102-0-0,s=255,c=3,t=24,pt=1,l=1,sg=0,ft=0,st=OK:1
4155 TSF:MSG:READ,0-0-102,s=255,c=3,t=25,pt=1,l=1,sg=0:1
4161 TSF:MSG:PONG RECV,HP=1
4163 TSM:UPL:OK
4165 TSM:READY:ID=102,PAR=0,DIS=1
4171 TSF:MSG:SEND,102-102-0-0,s=255,c=3,t=15,pt=6,l=2,sg=0,ft=0,st=OK:0100
4179 TSF:MSG:READ,0-0-102,s=255,c=3,t=15,pt=6,l=2,sg=0:0100
4188 TSF:MSG:SEND,102-102-0-0,s=255,c=0,t=18,pt=0,l=5,sg=0,ft=0,st=OK:2.3.0
4198 TSF:MSG:SEND,102-102-0-0,s=255,c=3,t=6,pt=1,l=1,sg=0,ft=0,st=OK:0
4214 TSF:MSG:READ,0-0-102,s=255,c=3,t=6,pt=0,l=1,sg=0:M
4222 TSF:MSG:SEND,102-102-0-0,s=255,c=3,t=11,pt=0,l=9,sg=0,ft=0,st=OK:Gas Meter
4233 TSF:MSG:SEND,102-102-0-0,s=255,c=3,t=12,pt=0,l=5,sg=0,ft=0,st=OK:2.4.1
4243 TSF:MSG:SEND,102-102-0-0,s=1,c=0,t=21,pt=0,l=8,sg=0,ft=0,st=OK:Gasmeter
4253 TSF:MSG:SEND,102-102-0-0,s=2,c=0,t=23,pt=0,l=16,sg=0,ft=0,st=OK:Analog Get Child
4265 TSF:MSG:SEND,102-102-0-0,s=3,c=0,t=23,pt=0,l=19,sg=0,ft=0,st=OK:Debug Set/Get Child
4276 MCO:REG:REQ
4280 TSF:MSG:SEND,102-102-0-0,s=255,c=3,t=26,pt=1,l=1,sg=0,ft=0,st=OK:2
4288 TSF:MSG:READ,0-0-102,s=255,c=3,t=27,pt=1,l=1,sg=0:1
4294 MCO:PIM:NODE REG=1
4298 MCO:BGN:STP
Setup: Start
Debug level fetched from EEPROM, value: 0
readEeprom: Pos 1Value 255
readEeprom: Pos 3Value 255
High threshold fetched from EEPROM, value: 255
Low threshold fetched from EEPROM, value: 255
midValue: 255
Setup: End
4315 MCO:BGN:INIT OK,TSP=1
4325 TSF:MSG:SEND,102-102-0-0,s=3,c=1,t=24,pt=3,l=2,sg=0,ft=0,st=OK:0
4335 TSF:MSG:SEND,102-102-0-0,s=3,c=1,t=25,pt=3,l=2,sg=0,ft=0,st=OK:255
4345 TSF:MSG:SEND,102-102-0-0,s=3,c=1,t=26,pt=3,l=2,sg=0,ft=0,st=OK:255
4354 TSF:MSG:SEND,102-102-0-0,s=2,c=1,t=24,pt=3,l=2,sg=0,ft=0,st=OK:0
4364 TSF:MSG:SEND,102-102-0-0,s=2,c=1,t=25,pt=3,l=2,sg=0,ft=0,st=OK:41
4374 TSF:MSG:SEND,102-102-0-0,s=2,c=1,t=26,pt=3,l=2,sg=0,ft=0,st=OK:250
4382 TSF:MSG:SEND,102-102-0-0,s=1,c=1,t=34,pt=7,l=5,sg=0,ft=0,st=OK:0.00
4392 TSF:MSG:SEND,102-102-0-0,s=1,c=1,t=24,pt=5,l=4,sg=0,ft=0,st=OK:0
4403 TSF:MSG:SEND,102-102-0-0,s=1,c=1,t=35,pt=7,l=5,sg=0,ft=0,st=OK:0.000
Sensor state: 1
Request: Start
34418 TSF:MSG:SEND,102-102-0-0,s=1,c=2,t=24,pt=0,l=0,sg=0,ft=0,st=OK:
34428 TSF:MSG:READ,0-0-102,s=1,c=1,t=24,pt=0,l=1,sg=0:0
Receiver: 1
Received last pulse count from gw: 0
*/
