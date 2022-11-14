/**
#############################		Sensoren		###################################

			Gas		Arduino Raspberry kompatible Linear Hall Magnetic Sensor Module KY-024 	(1,35 EUR)
					https://arduinomodules.info/ky-024-linear-magnetic-hall-module/						operating voltage 2.7V to 6.5V
					Angeschlossen an 3,3V 
					kein Magnet in der Nähe: 			AnalogValue=512
					starker Magnet ganz dicht dran:		AnalogValue=267
					
			Water	Obstacle Avoidance TCRT5000 Infrared Track Sensor Module For Arduino	(1,00 EUR) 	operating voltage 3,3V to 5V

#############################		Versionen		###################################
 
20161012 Verison 1.2		IsAlive hinzugefügt
20161110 Version 1.3		Entprellen optimiert
20170221 Version 2.0		Umstellung auf Analog
20170310 Version 2.1		Optimierung der Min/Max Werte
20180102 Version 2.2		Codeoptimierung, MinSend alle 10 Minuten funktioniert immer noch nicht
20180205 Version 2.3.1		Umstellung auf Lib 2.2.0, RepeaterFeature deaktiviert
							HEARTBEAT alle 5 Minuten
20181112 Version 2.3.2		Umstellung auf Lib 2.3.0
							bei receive() #defines für CHILD_ID und CHILD_ID_DEBUG verwendet (vorher 1 und 3 fest eingebaut)
20181113 Version 2.4.0		Gas und Water Meter Sketche zusammengeführt. Mit #define WATER wird Wasserzähler aktiviert. Durch Auskommentieren der Gaszähler
							unsigned int sensorValue; (vorher nur int)
							debuglevel bei EEPROM Wert von 255 (also bisher nicht beschrieben) auf 0 setzten und ins EEPROM schreiben
20181116 Version 2.4.1		neben sendHeartbeat wird der Wert von millis() zum DebugChild als V_VAR4 gesendet (damit sollen sporadische Reboots erkannt werden)
20221107 Version 3.0-001	Neuentwicklung, um mehr Speicher bei globalen Variablen zu sparen. Obwohl keine Fehler MY_DEBUG angezeigt wurde, war die Kommunikation mit dem Gateway bei 2.4.1 sehr schlecht.
							Der Node konnte sich nicht registrieren. Ein Gegenversuch, mit einem Minimal-Node auf der selben Hardware lief ohne Probleme.
							Schlussfolgerung: Es sollten genug Bytes für lokale Variablen zur Verfügung stehen.
							Anpassugnen auch bei CommonFunctions.h. Es wurden zu viele ungenutzt MyMessage Objekte angelegt.
20221110 Version 3.02		Optimierungen im Code in Bezug auf FHEM


#############################		Settings		###################################

Settings in Controller FHEM
set MYSENSOR_102 value52 338900 				//set a now gas/water meter value




*************************************************/


#define SKETCH_VER						"3.02"				// Sketch version
#define MY_RADIO_RF24


//	folgendes muss vor MySensors.h stehen
//	###################   Debugging   #####################
#define MY_DEBUG											//Output kann im LogParser analysiert werden https://www.mysensors.org/build/parser
#define SER_DEBUG											// aus CommonFunctions.h für eigenes DEBUG_PRINT
#define MY_SPECIAL_DEBUG									// für Extended Debug in FHEM
// #define MY_DEBUG_VERBOSE_RF24								//Testen, welche zusätzlichen Infos angezeigt werden
// #define MY_SPLASH_SCREEN_DISABLED
// #define MY_SIGNAL_REPORT_ENABLED

//	###################   Features   #####################
// #define MY_REPEATER_FEATURE
// #define MY_GATEWAY_SERIAL
// #define MY_INCLUSION_MODE_FEATURE
// #define MY_INCLUSION_BUTTON_FEATURE
// #define MY_INCLUSION_MODE_BUTTON_PIN 3

//	###################   LEDs   #####################
// #define MY_WITH_LEDS_BLINKING_INVERSE
// #define MY_DEFAULT_TX_LED_PIN 				(8)
// #define MY_DEFAULT_LED_BLINK_PERIOD 		10

// ###################   Transport   #####################
/*
RF24_PA_MIN = 	-18dBm 		0	R_TX_Powerlevel_Pct 	25		läuft nicht sauber mit NRF24L01+ Modul (verliert Uplink)
RF24_PA_LOW = 	-12dBm 		1	R_TX_Powerlevel_Pct				optimal für NRF24L01+
RF24_PA_HIGH = 	-6dBm 		2	R_TX_Powerlevel_Pct
RF24_PA_MAX = 	 0dBm		3	R_TX_Powerlevel_Pct
*/
#define MY_RF24_PA_LEVEL 					RF24_PA_LOW  //EchoNote hatte Max -> Reichweite bis Gartenhaus (-29) und noch Empfangen (-149) bis hinter Steins Haus
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
#define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)
// #define MY_TRANSPORT_SANITY_CHECK			//enable regular transport sanity checks -> wirkt nur bei Gateway oder Repeater, ist dort per default aktiviert


// #define MY_PARENT_NODE_ID 					0
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


// #define WATER
#define GAS


#ifdef WATER
	#define MY_NODE_ID					101					// Water Node ID 101
	#define SKETCH_NAME					"Water Meter"		// Optional child sensor name
	#define PULSE_FACTOR				1000				// Number of blinks per m3 of your meter (One rotation/liter)
	#define MAX_FLOW					90					// Max flow (l/min) value to report. This filters outliers.
	#define CHILD_ANALOG_METER_TEXT		(F("Watermeter"))	// Optional child sensor name
	#define	DEFAULT_METER_VALUE			263191				// last seen Value from Meter to set to the EEPROM
	#define	DEFAULT_HI_THRESHOLD		150					// higher threshold for analog readings
	#define	DEFAULT_LO_THRESHOLD		80					// lower threshold for analog readings
	#define	VOLUME_DIGITS				3					// Anzahl der Nachkommastellen bei V_VOLUME
#elif defined GAS
	#define MY_NODE_ID					102					// Gas Node ID 102
	#define SKETCH_NAME					"Gas Meter"			// Optional child sensor name
	#define PULSE_FACTOR				100					// Number of blinks per m3 of your meter (One rotation/liter)
	#define MAX_FLOW					76					// Max flow (l/min) value to report. This filters outliers.
	#define CHILD_ANALOG_METER_TEXT		(F("Gasmeter"))		// Optional child sensor name
	#define	DEFAULT_METER_VALUE			615585				// last seen Value from Meter to set to the EEPROM
	#define	DEFAULT_HI_THRESHOLD		500					// higher threshold for analog readings
	#define	DEFAULT_LO_THRESHOLD		494					// lower threshold for analog readings
	#define	VOLUME_DIGITS				2					// Anzahl der Nachkommastellen bei V_VOLUME
#else //TEST
	#define MY_NODE_ID					109					// Test Node ID 109
	#define SKETCH_NAME					"TEST Meter Node"	// Optional child sensor name
	#define PULSE_FACTOR				1000				// Number of blinks per m3 of your meter (One rotation/liter)
	#define MAX_FLOW					90					// Max flow (l/min) value to report. This filters outliers.
	#define CHILD_ANALOG_METER_TEXT		(F("TestChild"))			// Optional child sensor name
	#define	DEFAULT_METER_VALUE			381749				// last seen Value from Meter to set to the EEPROM
	#define	DEFAULT_HI_THRESHOLD		500					// higher threshold for analog readings
	#define	DEFAULT_LO_THRESHOLD		494					// lower threshold for analog readings
	#define	VOLUME_DIGITS				3					// Anzahl der Nachkommastellen bei V_VOLUME	
#endif



// ###################   Allgemeine MySensors Funktionen aus CommonFunctions.h (erhöht den Speicherverbrauch   #####################

// #define	WITH_BATTERY						//DS18B20 Sensoren funktionieren nicht mit weniger als 2,5V, eigentlich müsste OneWireMaster mit 5V und Netzteil betriebern werden
#define WITH_HWTIME
#define WITH_RF24_INFO						// übermittel RSSI, PA_Level, RF_Channel
#define WITH_NODE_INFO						// übermittel NodeID, ParentNodeID




#include <MySensors.h>  
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen



// ###################   Node Spezifisch   #####################

#define CHILD_ANALOG_METER					1					// Id of the sensor child --> V_FLOW, V_VOLUME
#define CHILD_ANALOG_DEBUG					2					// ID für Threshold Werte
#define CHILD_ANALOG_DEBUG_TEXT				(F("AnalogDbg"))		// ID für Threshold Werte

// Input and output definitions
#define ANALOG_INPUT_SENSOR					A0					// The analog input you attached your sensor. 
#define SEND_WAIT							50

// Sonstige Werte
#define SEND_FREQUENCY						30000				//default: 30000	Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
#define MAX_HEARTBEAT_CYCLE					10					//default: 10		X * SEND_FREQUENCY
#define MAX_INTERNALS_UPDATE_CYCLE			120					//default: 120		X * SEND_FREQUENCY, jede Stunde Update senden (Debug, Threshold usw) (120*30Sekunden)
#define MAX_FLOW_TO_ZERO_CYLCE				4					//default: 4		X * SEND_FREQUENCY, nach 2 Minuten ohne Durchfluss Werte auf 0 setzen


#define MAX_CYCLES_LOW_ENERGY				3					//default: 3		nach so vielen Durchläufen 0 Watt anzeigen

#define EEPROM_LO_THRESHOLD					0 					//16 Bit
#define EEPROM_HI_THRESHOLD					2					//16 Bit
#define EEPROM_DEVICE_DEBUG_LEVEL			4					//8  Bit
#define EEPROM_METER_VALUE					5					//32 Bit
#define MAX_DEBUG_LEVEL         			9

MyMessage msgAnalogMeter(CHILD_ANALOG_METER, V_VOLUME);			// S_GAS und S_WATER können V_FLOW, V_VOLUME --> muss später beim senden jeweils gesetzt werden
MyMessage msgAnalogDebug(CHILD_ANALOG_DEBUG, V_VAR1);			// V_VAR1, V_VAR2, V_VAR3, V_VAR4, V_VAR5

// MyMessage thValueMin		(CHILD_ID_ANALOG, V_VAR1);
// MyMessage thValueMax		(CHILD_ID_ANALOG, V_VAR2);
// MyMessage MsgMinValue	(CHILD_ID_ANALOG, V_VAR3);
// MyMessage MsgMaxValue	(CHILD_ID_ANALOG, V_VAR4);
// MyMessage SensorValue	(CHILD_ID_ANALOG, V_VAR5);

volatile uint32_t pulseCount = 0;
volatile bool sensorState = false;
bool GotNewPulse = false;
bool informGW = true;

uint8_t debugLevel = 0;										// sets the debug level, 0 = basic info. 1 = streaming level info. 2 = sent level streaming to gateway.
uint8_t heartBeatCycle = MAX_HEARTBEAT_CYCLE;				// MAX_HEARTBEAT_CYCLE
uint8_t flowToZeroCycle = 0;								// MAX_FLOW_TO_ZERO_CYLCE
volatile uint8_t internalsUpdateCycle = 0;					// MAX_INTERNALS_UPDATE_CYCLE

uint16_t maxValue = (uint16_t) 0;
uint16_t minValue = (uint16_t) 1024;

uint32_t lastSend = 0;
uint32_t lastPulseTime = 0;

float flow = 0;

//für Debug Ausgaben in checkThreshold()
#ifdef SER_DEBUG
	uint16_t oldSensorValue=0;
#endif	
		

void preHwInit() //kein serieller Output 
{
	//Serielles Interface nur setzten, falls nicht schon von MySensors Framework erledigt
	#if !defined MY_DEBUG
		DEBUG_SERIAL(MY_BAUD_RATE);	// MY_BAUD_RATE from MyConfig.h 115200ul, gehört nach preHwInit. Falls in before(), friert der Arduino ein
	#endif
}

void before() 
{
	DEBUG_PRINTLN(F("before"));
}


void setup()
{
	DEBUG_PRINTLN(F("setup"));

	// +++++++++++++++++		pulseCount		++++++++++++++++++++
	uint32_t MeterValue = readEeprom32(EEPROM_METER_VALUE);
	if (MeterValue == 0xFFFFFFFF)
	{
		writeEeprom32(EEPROM_METER_VALUE, DEFAULT_METER_VALUE);	
		DEBUG_PRINT(F("EEPROM_METER_VALUE to default"));
		MeterValue=DEFAULT_METER_VALUE;
	}
	DEBUG_PRINT(F("readEeprom32: EEPROM_METER_VALUE "));
	DEBUG_PRINTLN(MeterValue);
	pulseCount = MeterValue;
	
	
	// +++++++++++++++++		debugLevel		++++++++++++++++++++
	debugLevel = loadState(EEPROM_DEVICE_DEBUG_LEVEL); 	//8 Bit
	if (debugLevel>MAX_DEBUG_LEVEL)
	{
		debugLevel=0;
		DEBUG_PRINTLN(F("Save: debugLevel to 0"));
		saveState(EEPROM_DEVICE_DEBUG_LEVEL, debugLevel);//8 Bit
	}
	DEBUG_PRINT(F("debugLevel level fetched from EEPROM "));
	DEBUG_PRINTLN(debugLevel);
	
	
	// +++++++++++++++++		Threshold hi/lo		++++++++++++++++++++
	uint16_t tempVal = readEeprom16(EEPROM_HI_THRESHOLD);//16 Bit , weil 1024 nicht in 8 Bit reinpasst
	if (tempVal == 0xFFFF) //frisches EEPROM mit neuen Standardwerten beschreiben und highThreshold belassen wie im Programmkopf gesetzt
	{
		writeEeprom16(EEPROM_HI_THRESHOLD, DEFAULT_HI_THRESHOLD);
	}
	tempVal = readEeprom16(EEPROM_LO_THRESHOLD); //16 Bit
	if (tempVal == 0xFFFF) //frisches EEPROM mit neuen Standardwerten beschreiben und lowThreshold belassen wie im Programmkopf gesetzt
	{
		writeEeprom16(EEPROM_LO_THRESHOLD, DEFAULT_LO_THRESHOLD);
	}



	#ifdef SER_DEBUG
		showEEpromHex();
	#endif

}


void presentation()  {
	sendSketchInfo(SKETCH_NAME, SKETCH_VER );

	#ifdef WATER
		DEBUG_PRINTLN(F("Present Water"));
		present(CHILD_ANALOG_METER, S_WATER, CHILD_ANALOG_METER_TEXT);       
	#else
		DEBUG_PRINTLN(F("Present Gas"));
		present(CHILD_ANALOG_METER, S_GAS, CHILD_ANALOG_METER_TEXT); 
	#endif
	
	present(CHILD_ANALOG_DEBUG, S_CUSTOM, CHILD_ANALOG_DEBUG_TEXT);
	
	myPresentation();	
}
	

void loop()
{
	checkThreshold();
	uint32_t currentTime = millis();
	if (NodePresented)
	{
		if ((((currentTime - lastSend) > (uint32_t)SEND_FREQUENCY)) || informGW)
		{
			lastSend = currentTime;
			heartBeatCycle++;			//MAX_HEARTBEAT_CYCLE
			internalsUpdateCycle++;		//MAX_INTERNALS_UPDATE_CYCLE
			flowToZeroCycle++;			//MAX_FLOW_TO_ZERO_CYLCE
			DEBUG_PRINT(F("SEND_FREQUENCY HB: "));
			DEBUG_PRINT(heartBeatCycle);
			DEBUG_PRINT(F(" flowToZeroCycle: "));
			DEBUG_PRINTLN(flowToZeroCycle);
			if (GotNewPulse)
			{
				GotNewPulse = false;
				send(msgNewMeterValue.set(pulseCount));
				float volume = (float)pulseCount / ((float)PULSE_FACTOR);
				send(msgAnalogMeter.setType(V_VOLUME).set(volume, (uint8_t)VOLUME_DIGITS));
				
				if (flow < ((float)MAX_FLOW)) 
				{
					send(msgAnalogMeter.setType(V_FLOW).set(flow, 1));
				}
				else
				{
					send(msgDebugReturnString.set(F("Flow to high")));
				}		
			}
			
			if (informGW)
			{
				DEBUG_PRINTLN("informGW");
				informGW = false;
				send(msgNewMeterValue.set(pulseCount));
			}
		}
		
		if (heartBeatCycle >= (uint8_t)MAX_HEARTBEAT_CYCLE)
		{
			heartBeatCycle=0;
			myHeartBeatLoop();
		}
		
		
		if (internalsUpdateCycle >= (uint8_t)MAX_INTERNALS_UPDATE_CYCLE)
		{
			internalsUpdateCycle = 0;
			uint16_t localThreshold;

			DEBUG_PRINTLN(F("IU: Write EEPROM_METER_VALUE"));
			writeEeprom32(EEPROM_METER_VALUE, pulseCount);

			send(msgDebugLevel.set(debugLevel));
			send(msgNewMeterValue.set(pulseCount));
			float volume = (float)pulseCount / ((float)PULSE_FACTOR);
			send(msgAnalogMeter.setType(V_VOLUME).set(volume, (uint8_t)VOLUME_DIGITS));

			send(msgAnalogDebug.setType(V_VAR1).set(readEeprom16(EEPROM_LO_THRESHOLD)));
			send(msgAnalogDebug.setType(V_VAR2).set(readEeprom16(EEPROM_HI_THRESHOLD)));
			
			send(msgAnalogDebug.setType(V_VAR3).set(minValue));
			send(msgAnalogDebug.setType(V_VAR4).set(maxValue));
			
			send(msgDebugReturnString.set(F("---")));
		}
		
		
		if (flowToZeroCycle >= (uint8_t)MAX_FLOW_TO_ZERO_CYLCE)
		{	
			flowToZeroCycle = 0;
			if (not(flow == 0.0))//Gateway nicht zuspammen, deshalb flow nur auf 0 setzen, wenn es nicht schon so ist.
			{
				flow = 0;
				send(msgDebugReturnString.set(F("Flw0")));
				send(msgAnalogMeter.setType(V_FLOW).set(flow, 1));
			}
		}

	}
}

void receive(const MyMessage &message)
{
	if ((message.sensor == CHILD_DEBUG_LEVEL) && !message.isEcho())
	{
		switch (message.type) 
		{
			case V_TEXT: 
			{
				debugLevel = message.getByte();
				if (debugLevel == 0)
				{
					send(msgDebugReturnString.set(F("dbg 0")));
				}	
				if (debugLevel == 1)
				{
					send(msgDebugReturnString.set(F("dbg1 send V_VAR5")));
				}
				if (debugLevel == 2)
				{
					send(msgDebugReturnString.set(F("dbg2 send V_VAR3-5")));
				}
				if (debugLevel == 3)
				{
					send(msgDebugReturnString.set(F("dbg3 send dummy")));
				}
				if (debugLevel == 4)
				{
					#ifdef SER_DEBUG
						send(msgDebugReturnString.set(F("showEEpromHex")));
						showEEpromHex();
					#else
						send(msgDebugReturnString.set(F("dbg4 not possible")));
					#endif
					debugLevel=0;
				}

				if (debugLevel == 5)
				{
					// send(msgDebugReturnString.set(F("dbg5 internal Upd")));//kann deaktiviert bleiben, weil es sowieso durch "---" überschrieben wird 
					internalsUpdateCycle=MAX_INTERNALS_UPDATE_CYCLE;
					debugLevel=0;
				}
				if (debugLevel == 9)
				{
					send(msgDebugReturnString.set(F("hwReboot")));
					hwReboot();
				}
				//	+++++++++++++ nur debugLevel 0,1,2,3 im EEPROM abspeichent. Die anderen Werte sollten nur einmalig eine Aktion auslösen
				if ((debugLevel >= 0) and (debugLevel <=3))
				{
					DEBUG_PRINT(F("Save: debugLevel to "));
					DEBUG_PRINTLN(debugLevel);
					saveState(EEPROM_DEVICE_DEBUG_LEVEL, debugLevel);//8 Bit	
				}
			}
			break;
		}
	}
	else if ((message.sensor == CHILD_NEW_METER_VALUE) && !message.isEcho())
	{
		switch (message.type) 
		{
			case V_TEXT: 
			{
				pulseCount = message.getULong();
				DEBUG_PRINT("new MeterValue from GW");
				DEBUG_PRINTLN(pulseCount);
				informGW = true;
				writeEeprom32(EEPROM_METER_VALUE, pulseCount);
			}
		}
	}
	else if ((message.sensor == CHILD_ANALOG_DEBUG) && !message.isEcho())
	{	
		switch (message.type) 
		{
			case V_VAR1: 
			{
				writeEeprom16(EEPROM_LO_THRESHOLD, message.getULong()); //16 Bit
				internalsUpdateCycle = (uint8_t) MAX_INTERNALS_UPDATE_CYCLE;
			}
			break;
			case V_VAR2: 
			{
				writeEeprom16(EEPROM_HI_THRESHOLD, message.getULong()); // 16 Bit
				internalsUpdateCycle = (uint8_t) MAX_INTERNALS_UPDATE_CYCLE;
			}
			break;
		}
  	}
}


void newPulse()
{
	uint32_t newPulseTime = millis();
	uint32_t interval = newPulseTime - lastPulseTime;
	GotNewPulse = true;	
	
	flowToZeroCycle = 0;
	flow = (float)(60000.0 / interval) * (float)( 1000 / PULSE_FACTOR);
	
	lastPulseTime = newPulseTime;

	pulseCount++;
	DEBUG_PRINT("newPulse: ");
	DEBUG_PRINT(pulseCount);
	DEBUG_PRINT(" interval: ");
	DEBUG_PRINT(interval);
	DEBUG_PRINT(" flow: ");
	DEBUG_PRINTLN(flow);
}


void checkThreshold() {
	uint8_t maxCycles = 8;
	uint16_t sensorValue = analogRead(ANALOG_INPUT_SENSOR);	//vorwärmen
	uint16_t lowThreshold = readEeprom16(EEPROM_LO_THRESHOLD);
	uint16_t highThreshold = readEeprom16(EEPROM_HI_THRESHOLD);
	
	for (uint8_t cycles = 1; cycles < maxCycles; cycles++) {
			sensorValue = sensorValue + analogRead(ANALOG_INPUT_SENSOR);
	}
	sensorValue = (sensorValue / maxCycles);	

	if (minValue>=sensorValue)
	{
		minValue=sensorValue;
	}
	if (maxValue<=sensorValue)
	{
		maxValue=sensorValue;
	}
 

	if ((sensorState == true) && (sensorValue < lowThreshold))
	{
		newPulse();
		sensorState = false;
	}
	if ((sensorState == false) && (sensorValue > highThreshold)) 
	{
		sensorState = true;
	}
	
	
	
	#ifdef SER_DEBUG
		if (abs(sensorValue - oldSensorValue) > 1)
		{
			DEBUG_PRINT("checkThreshold: ");
			DEBUG_PRINT(sensorValue);
			DEBUG_PRINT(" oldValue: ");
			DEBUG_PRINT(oldSensorValue);
			DEBUG_PRINT(" sensorState: ");
			DEBUG_PRINT(sensorState);
			DEBUG_PRINT(" hiTH: ");
			DEBUG_PRINT(highThreshold);
			DEBUG_PRINT(" LoTH: ");
			DEBUG_PRINTLN(lowThreshold);
			oldSensorValue=sensorValue;
		}
	#endif
	

	
	
	
	if (debugLevel > 0) 
	{
		if (debugLevel == 1) 
		{
			send(msgAnalogDebug.setType(V_VAR5).set(sensorValue));
			wait(50);
		}
		
		if (debugLevel == 2) 
		{
			send(msgAnalogDebug.setType(V_VAR3).set(minValue));
			send(msgAnalogDebug.setType(V_VAR4).set(maxValue));
			send(msgAnalogDebug.setType(V_VAR5).set(sensorValue));
			wait(50);
		}

		if (debugLevel == 3) 
		{
			newPulse();
			#ifdef WATER
				wait(2000); //erst warten, dann NewPulse, damit die Updates der Werte in Loop zügig gesendet werden
			#else
				wait(8000); 
			#endif

		}
		wait(300);
	}
}




