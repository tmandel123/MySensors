/**
Sensors:	Gas		Arduino Raspberry kompatible Linear Hall Magnetic Sensor Module KY-024 	(1,35 EUR)
					https://arduinomodules.info/ky-024-linear-magnetic-hall-module/						operating voltage 2.7V to 6.5V
					Angeschlossen an 3,3V 
					kein Magnet in der Nähe: 			AnalogValue=512
					starker Magnet ganz dicht dran:		AnalogValue=267
					
			Water	Obstacle Avoidance TCRT5000 Infrared Track Sensor Module For Arduino	(1,00 EUR) 	operating voltage 3,3V to 5V

*/
 
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
// 20181116 Version 2.4.1	neben sendHeartbeat wird der Wert von millis() zum DebugChild als V_VAR4 gesendet (damit sollen sporadische Reboots erkannt werden)

//ToDo: 	DEBUG_SERIAL(x) statt debugMessage

// ToDo		Testmodus: Wenn dieser aktiviert ist, wird die Funktion newPulse alle 10 Sekunden weitergezählt und der eigentliche Zählerwert um eins Erhöht.	
//			Dafür neuen Debugmodus (z.B. debug=9) einführen. Das wird auch der Standardwert für neue Nodes, wenn im EEPROM nocht 0xFF steht
//
//			LED Blink bei Funktion NewPulse 



/* Config from FHEM

set MYSENSOR_102 value52 338900 				//set a now gas/water meter value


*/

#define SKETCH_VER						"2.4.1-018"				// Sketch version
#define MY_RADIO_RF24

#define MY_DEBUG //muss vor MySensors.h stehen
#define SER_DEBUG


// #define MY_REPEATER_FEATURE
#define MY_RF24_CHANNEL 96									// Für Testphase deaktivieren, damit Kanal 76 aktiv wird (Prod=96 Test=76)
#define MY_TRANSPORT_WAIT_READY_MS (5000ul)
#define MY_RF24_SANITY_CHECK
// #define MY_PARENT_NODE_ID 50
// #define MY_PARENT_NODE_ID 0
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_SPLASH_SCREEN_DISABLED
#define MY_RF24_SANITY_CHECK

#define WATER
// #define GAS

#ifdef WATER
	#define MY_NODE_ID 101									// Water Node ID
	#define SKETCH_NAME					"Water Meter"		// Optional child sensor name
	#define PULSE_FACTOR				1000				// Number of blinks per m3 of your meter (One rotation/liter)
	#define MAX_FLOW					40					// Max flow (l/min) value to report. This filters outliers.
	#define CHILD_NAME					"Watermeter"		// Optional child sensor name
	uint16_t highThreshold = 			150;					// higher threshold for analog readings
	uint16_t lowThreshold = 			80;					// lower threshold for analog readings
	uint16_t maxValue = 				210;
	uint16_t minValue = 				40;
#elif defined GAS
	#define MY_NODE_ID 102									// Gas Node ID
	#define SKETCH_NAME					"Gas Meter"			// Optional child sensor name
	#define PULSE_FACTOR				100					// Number of blinks per m3 of your meter (One rotation/liter)
	#define MAX_FLOW					40					// Max flow (l/min) value to report. This filters outliers.
	#define CHILD_NAME					"Gasmeter"			// Optional child sensor name
	uint16_t highThreshold =			500;				// higher threshold for analog readings
	uint16_t lowThreshold =				494;				// lower threshold for analog readings
	uint16_t maxValue = 				516;
	uint16_t minValue = 				486;
#else //TEST
	#define MY_NODE_ID 223									// Test Node ID
	#define SKETCH_NAME					"-TEST- Meter Node"	// Optional child sensor name
	#define PULSE_FACTOR				100					// Number of blinks per m3 of your meter (One rotation/liter)
	#define MAX_FLOW					40					// Max flow (l/min) value to report. This filters outliers.
	#define CHILD_NAME					"-Test- Child"			// Optional child sensor name
	uint16_t highThreshold =			500;				// higher threshold for analog readings
	uint16_t lowThreshold =				494;				// lower threshold for analog readings
	uint16_t maxValue = 				516;
	uint16_t minValue = 				486;
#endif

#include <SPI.h>
#include <MySensors.h>  


// Node and sketch information
#define CHILD_ID						1					// Id of the sensor child
#define CHILD_ID_ANALOG					2					//ID für Threshold Werte
#define CHILD_ID_DEBUG					3					// Debug setzen und lesen

// Input and output definitions
#define ANALOG_INPUT_SENSOR				A0					// The analog input you attached your sensor. 
#define UPLINK_LED						5
#define PULSE_LED						6
#define SEND_WAIT						50
#define REQUEST_ACK						true

// Sonstige Werte
#define HEARTBEAT_INTERVAL				300000				//später alle 5 Minuten, zum Test alle 30 Sekunden
#define INTERNALS_UPDATE_INTERVAL		3600000				//jede Stunde Update senden (Debug, Threshold usw)
#define FLOW_TO_ZERO_TIME				120000				//120000

#define EEPROM_HI_THRESHOLD				0 					//16 Bit
#define EEPROM_LO_THRESHOLD				2					//16 Bit
#define EEPROM_DEBUGLEVEL				4					//8  Bit
#define EEPROM_METER_VALUE				5					//32 Bit


#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif


MyMessage flowMsg(CHILD_ID, V_FLOW);
MyMessage volumeMsg(CHILD_ID, V_VOLUME);
MyMessage lastCounterMsg(CHILD_ID, V_VAR1); 				//dieser Wert wird vom Controller abgefragt und stellt den Zählerstand dar

MyMessage analogValue	(CHILD_ID_ANALOG, V_VAR1);
MyMessage MsgMinValue	(CHILD_ID_ANALOG, V_VAR2);
MyMessage MsgMaxValue	(CHILD_ID_ANALOG, V_VAR3);
MyMessage MeterValue	(CHILD_ID_ANALOG, V_VAR5);

MyMessage debugValue	(CHILD_ID_DEBUG, V_VAR1);
MyMessage thValueMin	(CHILD_ID_DEBUG, V_VAR2);
MyMessage thValueMax	(CHILD_ID_DEBUG, V_VAR3);
MyMessage hwTime		(CHILD_ID_DEBUG, V_VAR4);
MyMessage Uplink		(CHILD_ID_DEBUG, V_VAR5);


volatile uint32_t pulseCount = 0;

bool informGW = false;
bool sensorState = true;
bool TransportUplink = true;
bool firstLoop = true;

uint8_t debugLevel = 0;										// sets the debug level, 0 = basic info. 1 = streaming level info. 2 = sent level streaming to gateway.

uint16_t midValue = 0;
uint16_t sensorValue = 0;

uint32_t oldPulseCount = 0;
uint32_t lastSend = 0;
uint32_t lastHeartBeat = 0;
uint32_t lastInternalsUpdate = 0; 
uint32_t lastPulseTime = 0;

float ppl = ((float)PULSE_FACTOR) / 1000;					// Pulses per liter
float oldflow = 0;
float volume = 0;
float flow = 0;

void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit: ");

	// showEEprom();
	// hwPinMode(UPLINK_LED, OUTPUT);
	// hwPinMode(PULSE_LED, OUTPUT);
}

void before() 
{
	DEBUG_PRINTLN("before: ");
	DEBUG_PRINT(SKETCH_NAME);
	DEBUG_PRINT(" ");
	DEBUG_PRINTLN(SKETCH_VER);

	// DEBUG_PRINTLN("writeEeprom32: ");
	// writeEeprom32(EEPROM_METER_VALUE, 338849);
	// showEEprom();
	// DEBUG_PRINT("readEeprom32: ");
	// uint32_t MeterValue = readEeprom32(EEPROM_METER_VALUE);
	// DEBUG_PRINTLN(MeterValue);
}


void setup()
{

	DEBUG_PRINTLN("setup: ");
	hwPinMode(UPLINK_LED, OUTPUT);
	hwPinMode(PULSE_LED, OUTPUT);
	
	uint32_t MeterValue = readEeprom32(EEPROM_METER_VALUE);
	// DEBUG_PRINT("readEeprom32: EEPROM_METER_VALUE ");
	// DEBUG_PRINTLN(MeterValue);
	pulseCount = MeterValue;
	oldPulseCount = MeterValue;
	volume = (float)pulseCount / ((float)PULSE_FACTOR);

	lastHeartBeat = millis();
	lastSend = lastHeartBeat;
	lastPulseTime = lastHeartBeat;
	lastInternalsUpdate = lastHeartBeat;

	// Fetch debug level from EEPROM
	debugLevel = loadState(EEPROM_DEBUGLEVEL); //8 Bit
	// debugMessage("Debug level fetched from EEPROM, value: ", String(debugLevel));
	
	if (debugLevel == 255)//vermutlich wurde das EEPROM an dieser Stelle noch nicht beschrieben
	{
		debugLevel=0;
		saveState(EEPROM_DEBUGLEVEL,debugLevel);
		// debugMessage("Debug level defaults stored to  EEPROM, value: ", String(debugLevel));
	}

	
	uint16_t high = readEeprom16(EEPROM_HI_THRESHOLD);//16 Bit , weil 1024 nicht in 8 Bit reinpasst
	uint16_t low = readEeprom16(EEPROM_LO_THRESHOLD); //16 Bit
	if (high == 0 || high == 65535) //frisches EEPROM mit neuen Standardwerten beschreiben und highThreshold belassen wie im Programmkopf gesetzt
	{
		writeEeprom16(EEPROM_HI_THRESHOLD, highThreshold);
		// debugMessage("highThreshold to EEPROM: ", String(highThreshold));
	}
	else 
	{
		highThreshold = high;
		// debugMessage("highThreshold from EEPROM: ", String(highThreshold));
	}
	
	if (low == 0 || low == 65535) //frisches EEPROM mit neuen Standardwerten beschreiben und lowThreshold belassen wie im Programmkopf gesetzt
	{
		writeEeprom16(EEPROM_LO_THRESHOLD, lowThreshold);
		// debugMessage("lowThreshold to EEPROM: ", String(lowThreshold));
	}
	else 
	{
		lowThreshold = low;
		// debugMessage("lowThreshold from EEPROM: ", String(lowThreshold));
	}
	midValue=uint16_t((lowThreshold+highThreshold)/2);	// Mittelwert von Threshold Max und Min (Soll minValue und maxValue begrenzen)
	// debugMessage("midValue: ", String(midValue));
	// debugMessage("Setup: ","End");
}


void presentation()  {
	
	sendSketchInfo(SKETCH_NAME, SKETCH_VER " " __TIME__ " " __DATE__);
#ifdef WATER	
	present(CHILD_ID, S_WATER, CHILD_NAME, "Counter Child");       
#else
	present(CHILD_ID, S_GAS, CHILD_NAME, "Counter Child"); 
#endif
	present(CHILD_ID_ANALOG, S_CUSTOM, "Analog Set/Get Child");
	present(CHILD_ID_DEBUG, S_CUSTOM, "Debug Set/Get Child");
}

void loop()
{
	checkThreshold();
	
	uint32_t currentTime = millis();
	uint32_t TimeSinceHeartBeat = currentTime - lastHeartBeat;
	uint32_t TimeSinceLastPulse = currentTime - lastPulseTime;
	
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
		if (informGW) //Aktualisierung nachdem bei receive neue Werte empfangen wurden
		{
			DEBUG_PRINTLN("informGW");
			informGW = false;
			send(lastCounterMsg.set(pulseCount),REQUEST_ACK);
			wait(SEND_WAIT);
			send(thValueMin.set(lowThreshold),REQUEST_ACK);
			wait(SEND_WAIT);
			send(thValueMax.set(highThreshold),REQUEST_ACK);
			wait(SEND_WAIT);
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
		
		// DEBUG_PRINTLN("IT: update");
		
		//Min und Max etwas Näher an den Durchschnitt heranziehen, gibt es auch bei der Flowberechnung
		if (minValue < (midValue - 4 ))
		{
			minValue++;
		}
		if (maxValue > (midValue + 4 ))
		{
			maxValue--;
		}
		

		
		send(debugValue.set(debugLevel),REQUEST_ACK);
		wait(SEND_WAIT);
		send(thValueMin.set(lowThreshold),REQUEST_ACK);
		wait(SEND_WAIT);
		send(thValueMax.set(highThreshold),REQUEST_ACK);
		wait(SEND_WAIT);
		send(analogValue.set(sensorValue),REQUEST_ACK);
		wait(SEND_WAIT);
		send(MsgMinValue.set(minValue),REQUEST_ACK);
		wait(SEND_WAIT);
		send(MsgMaxValue.set(maxValue),REQUEST_ACK);
		wait(SEND_WAIT);
		send(flowMsg.set(flow, 2),REQUEST_ACK);
		wait(SEND_WAIT);
		send(lastCounterMsg.set(pulseCount),REQUEST_ACK);
		wait(SEND_WAIT);
		send(volumeMsg.set(volume, 3),REQUEST_ACK);
		wait(SEND_WAIT);
		send(MeterValue.set(pulseCount),REQUEST_ACK);
		wait(SEND_WAIT);
		
		lastInternalsUpdate = currentTime;
	}


	if (abs(flow - oldflow) > 0.1)//float lässt sich schwer mit != vergleichen
	{
		oldflow = flow;
		// debugMessage("l/min:", String(flow));

		if (flow < ((float)MAX_FLOW)) 
		{
			// Send flow value to gw
			send(flowMsg.set(flow, 2));
			wait(SEND_WAIT);
			send(MsgMinValue.set(minValue));
			wait(SEND_WAIT);
			send(MsgMaxValue.set(maxValue));
			wait(SEND_WAIT);
			//Min und Max etwas Näher an den Durchschnitt heranziehen
			if (minValue < (midValue - 4 ))
			{
				minValue++;
			}
			if (maxValue > (midValue + 4 ))
			{
				maxValue--;
			}
			// debugMessage("MinNeu:", String(minValue));
			// debugMessage("MaxNeu:", String(maxValue));
		}
		else
		{
			DEBUG_PRINT("Flow to high: ");
			DEBUG_PRINTLN(flow);	
		}
	}

	// No Pulse count received in 2min 
	if (TimeSinceLastPulse > (uint32_t)FLOW_TO_ZERO_TIME && flow != 0) 
	{
		flow = 0;
		// DEBUG_PRINTLN("set flow 0: TSLP");
		// DEBUG_PRINTLN(TimeSinceLastPulse);
		// DEBUG_PRINT("oldflow ");
		// DEBUG_PRINT(oldflow);
		// DEBUG_PRINT("flow ");
		// DEBUG_PRINTLN(flow);
	}

	// Pulse count has changed
	if (pulseCount != oldPulseCount) 
	{
		digitalWrite(PULSE_LED,HIGH);
		oldPulseCount = pulseCount;
		// debugMessage("pulsecount: ", String(pulseCount));
		send(lastCounterMsg.set(pulseCount));
		volume = (float)pulseCount / ((float)PULSE_FACTOR);
		// debugMessage("volume: ", String(volume, 3));
		send(volumeMsg.set(volume, 3));
	}
	else
	{
		digitalWrite(PULSE_LED,LOW);
	}
	// 
	// DEBUG_PRINT(".");

}

// void debugMessage(String header, String content)
// {
	// DEBUG code ------
	// Serial.print(header);
	// Serial.println(content);
	// DEBUG code ------   
// }

void receive(const MyMessage &message)
{
	// debugMessage("Receiver: ", String(message.sensor));
	//myreadS: 0 Last: 50 dest: 223 Sen: 3 Cmd: 1 type: 27 PLT: 5 Len: 4 got Ack from: 3

	// DEBUG_PRINT("myread");
	// DEBUG_PRINT("S: ");
	// DEBUG_PRINT(message.sender);
	// DEBUG_PRINT(" Last: ");
	// DEBUG_PRINT(message.last);
	// DEBUG_PRINT(" dest: ");
	// DEBUG_PRINT(message.destination);
	// DEBUG_PRINT(" Sen: ");
	// DEBUG_PRINT(message.sensor);
	// DEBUG_PRINT(" Cmd: ");
	// DEBUG_PRINT(mGetCommand(message));
	// DEBUG_PRINT(" type: ");
	// DEBUG_PRINT(message.type);	
	// DEBUG_PRINT(" PLT: ");
	// DEBUG_PRINT(mGetPayloadType(message));
	// DEBUG_PRINT(" Len: ");
	// DEBUG_PRINTLN(mGetLength(message));

	if ((message.sensor == CHILD_ID_ANALOG) && !mGetAck(message))
	{
		switch (message.type) 
		{
			case V_VAR5: 
			{
				// unsigned long gwPulseCount = message.getULong();
				// pulseCount = gwPulseCount;
				pulseCount = message.getULong();
				DEBUG_PRINTLN("new pulseCount: set flow 0");
				flow = oldflow = 0;
				// debugMessage("Received last pulse count from gw: ", String(pulseCount));
				// pcReceived = true;
				informGW = true;
				writeEeprom32(EEPROM_METER_VALUE, pulseCount);
			}
		}
	}
	else if ((message.sensor == CHILD_ID_DEBUG) && !mGetAck(message))
	{	
		switch (message.type) 
		{
			case V_VAR1: 
			{
				debugLevel = message.getULong();
				saveState(EEPROM_DEBUGLEVEL, debugLevel);//8 Bit
				// debugMessage("Received new debug state from gw: ", String(debugLevel));
			}
			break;
			case V_VAR2: 
			{
				lowThreshold = message.getULong();
				writeEeprom16(EEPROM_LO_THRESHOLD, lowThreshold); //16 Bit
				// debugMessage("Received new low threshold from gw: ", String(lowThreshold));
				informGW = true;
				midValue=uint16_t((lowThreshold+highThreshold)/2);
			}
			break;
			case V_VAR3: 
			{
				highThreshold = message.getULong();
				writeEeprom16(EEPROM_HI_THRESHOLD, highThreshold); // 16 Bit
				// debugMessage("Received new high threshold from gw: ", String(highThreshold));
				informGW = true;
				midValue=uint16_t((lowThreshold+highThreshold)/2);
			}
			break;
		}
  	}
	else
	{
		if (mGetAck(message))
		{
			// DEBUG_PRINT("got Ack from: ");
			// DEBUG_PRINTLN(message.sensor);
		}
		// debugMessage("Received invalid message from gw! ", "");
	}
    

}


void newPulse()
{
	uint32_t newPulseTime = millis();
	uint32_t interval = newPulseTime - lastPulseTime;
	if (debugLevel > 0)
	{
		// debugMessage("new Pulse Interval since last ", String(interval));
	}
	
	if (interval<(uint32_t)10000) { // Sometimes we get interrupt on RISING
		return;
	}

	flow = (60000.0 / interval) / ppl;
	
	lastPulseTime = newPulseTime;

	pulseCount++;
	DEBUG_PRINT("newPulse: ");
	DEBUG_PRINT(pulseCount);
	DEBUG_PRINT(" flow: ");
	DEBUG_PRINTLN(flow);
	
}

void checkThreshold() {
  
	// DEBUG_PRINTLN("checkThreshold");
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
		// debugMessage("val = ", String(sensorValue, DEC));
		// debugMessage("min = ", String(minValue, DEC));
		// debugMessage("max = ", String(maxValue, DEC));
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
		  // debugMessage("flow dummy: ", "123.123");
		  // debugMessage("volume dummy: ", "456.45");
		  send(volumeMsg.set(123.123, 3));
		  send(flowMsg.set(456.45, 2));
		  wait(800);
		}
	}
	if ((sensorState == true) && (sensorValue < lowThreshold))
	{
		newPulse();
		sensorState = !sensorState;
		// debugMessage("pulsCount + 1", "");
		// debugMessage("Sensor state: ", String(sensorState));
	}
	if ((sensorState == false) && (sensorValue > highThreshold)) 
	{
		sensorState = !sensorState;
		// debugMessage("Sensor state: ", String(sensorState));
	}
}

int getAverage()
{
	// hier wird 100 Mal eingelesen. 10 Mal hintereinander analogRead ohne Pause. Dann 50ms Pause. Dies wird dann 10 Mal wiederholt
	uint8_t cycles = 0;
	uint8_t maxCycles = 4;
	uint8_t count = 0;       // variable for loop
	uint16_t val = 0;     // variable to store current values from the input
	uint16_t newVal = 0;      // variable to store new values from the input
	uint16_t average = 0;    // variable to store the peak value

	for (cycles = 0; cycles < maxCycles; cycles++) {
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
		wait(5); // 10 cycles of 50ms gives  10 samples in 500ms
	}
	// set average to average of newValue1
	average = newVal / maxCycles;

	// return the value of average to main program
	return average;
}



void writeEeprom16(uint8_t pos, uint16_t value) 
{
  saveState(pos, ((uint16_t)value >> 8));
  pos++;
  saveState(pos, (value & 0xff));
}

uint16_t readEeprom16(uint8_t pos) 
{
  uint16_t hiByte;
  uint16_t loByte;
  hiByte = loadState(pos) << 8;
  pos++;
  loByte = loadState(pos);
  return (hiByte | loByte);
}


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
readEeprom16: Pos 1Value 255
readEeprom16: Pos 3Value 255
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
