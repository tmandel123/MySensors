/**
 
20181112 Version 1.32		millis() von "uint32_t now" nach "unsigned long currentTime" und wieder zurück
20181112 Version 1.33		Sleep_mode ausgebaut und DEBUG_PRINT eingeführt
20181114 Version 1.34		boolean informGW = false; - > bool informGW = false;
20181114 Version 1.4-003	MY_REPEATER_FEATURE deaktiviert und sendSketchInfo(SKETCH_NAME, SKETCH_VER " " __TIME__ " " __DATE__);
							Es gibt immer wieder Aussetzer von mehreren Stunden. Danach läuft alles wieder normal. Zählerwerte gehen jedoch nicht verloren. Laufzeit akutell 36 Tage.


*************************************************/

//	###################   Debugging   #####################
// #define MY_DEBUG
// #define SER_DEBUG
// #define MY_DEBUG_VERBOSE_RF24								//Testen, welche zusätzlichen Infos angezeigt werden
// #define MY_SPLASH_SCREEN_DISABLED
// #define MY_SIGNAL_REPORT_ENABLED

//	###################   Features   #####################
#define MY_REPEATER_FEATURE
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
RF24_PA_MIN = 	-18dBm 		0	R_TX_Powerlevel_Pct 	25
RF24_PA_LOW = 	-12dBm 		1	R_TX_Powerlevel_Pct
RF24_PA_HIGH = 	-6dBm 		2	R_TX_Powerlevel_Pct
RF24_PA_MAX = 	 0dBm		3	R_TX_Powerlevel_Pct
*/
#define MY_RF24_PA_LEVEL 					RF24_PA_LOW  //EchoNote hatte Max -> Reichweite bis Gartenhaus (-29) und noch Empfangen (-149) bis hinter Steins Haus
// #define MY_RF24_PA_LEVEL 					RF24_PA_MAX
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
// #define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)
#define MY_TRANSPORT_SANITY_CHECK

#define MY_NODE_ID 							100
// #define MY_PARENT_NODE_ID 					50
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE





// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.5-004"        		// Sketch version
#define SKETCH_NAME           				"EnergyMeter"   		// Optional child sensor name


#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen


#define PULSE_FACTOR						1000				// Nummber of blinks per KWH of your meeter
#define MAX_WATT							12000				// Max watt value to report. This filetrs outliers.

#define EEPROM_DEVICE_DEBUG_LEVEL			0					//8  Bit
#define EEPROM_METER_VALUE					1					//32 Bit

#define SEND_WAIT							50
// #define REQUEST_ACK							true

// Sonstige Werte
#define HEARTBEAT_INTERVAL					300000				//später alle 5 Minuten, zum Test alle 30 Sekunden
#define INTERNALS_UPDATE_INTERVAL			3600000				//jede Stunde Update senden (Debug, Threshold usw)

#define MAX_DEBUG_LEVEL         			9



bool informGW = false;
bool firstLoop = true;

uint8_t debugLevel = 0;										// sets the debug level, 0 = basic info. 1 = streaming level info. 2 = sent level streaming to gateway.

uint32_t SEND_FREQUENCY = 20000; // Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
uint32_t oldPulseCount = 0;
uint32_t oldWatt = 0;
uint32_t lastSend;
uint32_t lastHeartBeat = 0;
uint32_t lastInternalsUpdate = 0; 

volatile uint32_t pulseCount = 0;
volatile uint32_t lastPulseTime = 0;
volatile uint32_t watt = 0;


// MyMessage wattMsg(CHILD_ID,V_WATT);
// MyMessage kwhMsg(CHILD_ID,V_KWH);
// MyMessage lastCounterMsg(CHILD_ID,V_VAR1);


void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	delay(50); // bei weniger als 18ms kommt soetwas hier: p⸮Y⸮%⸮⸮⸮⸮ 
	DEBUG_PRINTLN("preHwInit");
}

void before() 
{
	DEBUG_PRINTLN("before");
	
	debugLevel = loadState(EEPROM_DEVICE_DEBUG_LEVEL); 	//8 Bit
	if (debugLevel>MAX_DEBUG_LEVEL)
	{
		debugLevel=0;
		DEBUG_PRINTLN(F("Save: debugLevel to 0"));
		saveState(EEPROM_DEVICE_DEBUG_LEVEL, debugLevel);//8 Bit
	}
	DEBUG_PRINT(F("debugLevel level fetched from EEPROM "));
	DEBUG_PRINTLN(debugLevel);
	
	uint32_t MeterValue = readEeprom32(EEPROM_METER_VALUE);
	if (MeterValue == 0xFFFFFFFF)
	{
		DEBUG_PRINT("EEPROM_METER_VALUE to 0");
		MeterValue=0;
	}
	DEBUG_PRINT("readEeprom32: EEPROM_METER_VALUE ");
	DEBUG_PRINTLN(MeterValue);
	pulseCount = MeterValue;
	oldPulseCount = MeterValue;

}

void setup()
{
	DEBUG_PRINTLN("setup: ");

	pinMode(PULSE_LED, OUTPUT);
	
	pinMode(DIGITAL_INPUT_SENSOR,INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(DIGITAL_INPUT_SENSOR), onPulse, RISING);
	lastHeartBeat=millis();
	lastSend=lastHeartBeat;
	lastPulseTime = lastSend;
	lastInternalsUpdate = lastSend;
	
}

void presentation()
{
	mySendSketchInfo();
	myPresentation();
	present(CHILD_POWER_METER, S_POWER, CHILD_POWER_METER_TEXT);
	wait(100);
	myHeartBeatLoop();
}

void loop()
{
	uint32_t currentTime = millis();
	uint32_t TimeSinceHeartBeat = currentTime - lastHeartBeat;
	// uint32_t TimeSinceLastPulse = currentTime - lastPulseTime;
	
	
	if (((TimeSinceHeartBeat > (uint32_t)HEARTBEAT_INTERVAL)) || firstLoop || informGW)
	{
		lastHeartBeat = currentTime;

		if (informGW)
		{
			informGW = false;
			send(msgPowerMeter.setType(V_VAR1).set(pulseCount));
		}
		sendHeartbeat();
		myHeartBeatLoop();
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
			myHeartBeatLoop();
		}
		send(msgDebugLevel.set(debugLevel));
		send(msgNewMeterValue.set(pulseCount));

		lastInternalsUpdate = currentTime;
		
	}	
	
	// Only send values at a maximum frequency or woken up from sleep
	if (currentTime - lastSend > SEND_FREQUENCY) //todo: pulseled bool for faster off of led
	{
		if (watt != oldWatt) 
		{
			// Check that we dont get unresonable large watt value.
			// could hapen when long wraps or false interrupt triggered
			if (watt<((uint32_t)MAX_WATT)) {
				send(msgPowerMeter.setType(V_WATT).set(watt));  // Send watt value to gw
			}
			DEBUG_PRINT("Watt:");
			DEBUG_PRINTLN(watt);
			oldWatt = watt;
		}

		// Pulse cout has changed
		if (pulseCount != oldPulseCount) 
		{
			digitalWrite(PULSE_LED,HIGH);
			send(msgPowerMeter.setType(V_VAR1).set(pulseCount));  
			float kwh = ((float)pulseCount/((float)PULSE_FACTOR));
			oldPulseCount = pulseCount;
			send(msgPowerMeter.setType(V_KWH).set(kwh, 3));  // Send kwh value to gw

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
	if ((message.sensor == CHILD_NEW_METER_VALUE) && !mGetAck(message))
	{
		switch (message.type) 
		{
			case V_TEXT: 
			{
				noInterrupts(); //für die nächsten Zuweisungen die Interrupts deaktiveren
				pulseCount = message.getULong();
				oldPulseCount = pulseCount;
				DEBUG_PRINT("new MeterValue from GW");
				DEBUG_PRINTLN(pulseCount);
				watt = oldWatt = 0;
				informGW = true;
				writeEeprom32(EEPROM_METER_VALUE, pulseCount);
				interrupts();// Interrupts wieder aktiveren
			}
		}
	}
	if (message.sensor == CHILD_DEBUG_LEVEL)
	{ 
		switch (message.type) 
		{
			case V_TEXT: 
			{
				// debugLevel = message.getULong();
				debugLevel = message.getByte();
				if (debugLevel == 3)
				{
					#ifdef SER_DEBUG
						send(msgDebugReturnString.set(F("showEEpromHex")));
						showEEpromHex();
					#else
						send(msgDebugReturnString.set(F("dbg3 not possible")));
					#endif
				}		
				if (debugLevel == 9)
				{
					send(msgDebugReturnString.set(F("hwReboot")));
					hwReboot();
				}
			}
			break;
		}
	}
}

void onPulse()
{
	uint32_t newPulseTime = micros();
	uint32_t interval = newPulseTime-lastPulseTime;
	// if (interval<10000L) { // Sometimes we get interrupt on RISING
	if (interval<(uint32_t)10000) { // Sometimes we get interrupt on RISING
		return;
	}
	// watt = (3600000000.0 /interval) / ppwh;
	watt = (3600000000.0 /interval);
	lastPulseTime = newPulseTime;
	pulseCount++;
	DEBUG_PRINT("onPulse:");
	DEBUG_PRINTLN(pulseCount);
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


void ClearEeprom()
{
	// debugMessage("ClearEeprom", "");
	send(msgDebugReturnString.set(F("EEPROM cleared")));
	for (uint8_t i = 0; i < (EEPROM_METER_VALUE+5);i++)
	{
		Serial.print("Clearing Pos: "); 
		Serial.println(i);
		saveState(i,0xFF);
	}
}