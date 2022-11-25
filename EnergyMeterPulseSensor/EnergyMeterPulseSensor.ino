/**
 
20181112 Version 1.32		millis() von "uint32_t now" nach "unsigned long currentTime" und wieder zurück
20181112 Version 1.33		Sleep_mode ausgebaut und DEBUG_PRINT eingeführt
20181114 Version 1.34		boolean informGW = false; - > bool informGW = false;
20181114 Version 1.4-003	MY_REPEATER_FEATURE deaktiviert und sendSketchInfo(SKETCH_NAME, SKETCH_VER " " __TIME__ " " __DATE__);
							Es gibt immer wieder Aussetzer von mehreren Stunden. Danach läuft alles wieder normal. Zählerwerte gehen jedoch nicht verloren. Laufzeit akutell 36 Tage.
20220516 Version 1.5-007	Anpassungen an MySensors 2.3.2 (bei receive -> !message.isEcho() statt !mGetAck(message) (produktiv seit 13.5.2022)
							Anpassugnen für PULSE_LED (geht jetzt sofort bei onPulse an)
20220927 Version 1.5-008	Anpassungen für Übermittlung von 0 Watt Werten, falls PV-Einspeisung den Zähler stoppt
20220927 Version 1.5-009	DEBUG_SERIAL(MY_BAUD_RATE) von before() nach preHwInit()
20221125 Version 1.5-010	PULSE_FACTOR von 1000 auf 10000, onPulse korrigiert und PULSE_FACTOR mit eingebaut


*************************************************/


#define SKETCH_VER            				"1.5-010"        		// Sketch version
#define SKETCH_NAME           				"EnergyMeter"   		// Optional child sensor name





//	###################   Debugging   #####################
// #define MY_DEBUG												//Output kann im LogParser analysiert werden https://www.mysensors.org/build/parser
// #define SER_DEBUG											// aus CommonFunctions.h für eigenes DEBUG_PRINT
#define MY_SPECIAL_DEBUG									// für Extended Debug in FHEM
// #define MY_DEBUG_VERBOSE_RF24								//Testen, welche zusätzlichen Infos angezeigt werden
#define MY_SPLASH_SCREEN_DISABLED
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


#define MY_NODE_ID 							100
// #define MY_PARENT_NODE_ID 					100
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


#define MY_INDICATION_HANDLER									//erlaubt rewrite der Funktion void indication(indication_t ind) 



// ###################   Node Spezifisch   #####################

#define PULSE_FACTOR						10000				// Nummber of blinks per KWH of your meeter
#define MAX_WATT							12000				// Max watt value to report. This filetrs outliers.

#define EEPROM_DEVICE_DEBUG_LEVEL			0					//8  Bit	Position im Flash
#define EEPROM_METER_VALUE					1					//32 Bit	Position im Flash
#define	DEFAULT_METER_VALUE					18010892			// last seen Value from Enerny Meter to set to the EEPROM
#define MAX_DEBUG_LEVEL         			9

// Sonstige Werte


#define HEARTBEAT_INTERVAL					300000				//default: 300000
#define INTERNALS_UPDATE_INTERVAL			3600000				//default: 3600000	jede Stunde Update senden (Debug, Threshold usw)
#define SEND_FREQUENCY						30000				//default: 30000	Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
#define MAX_CYCLES_LOW_ENERGY				3					//default: 3		nach so vielen Durchläufen 0 Watt anzeigen




/** Werte für Debug überschreiben **/
// #ifdef SER_DEBUG

// #define HEARTBEAT_INTERVAL					10000				//default: 300000
// #define INTERNALS_UPDATE_INTERVAL			3600000				//default: 3600000	jede Stunde Update senden (Debug, Threshold usw)
// #define SEND_FREQUENCY						15000				//default: 30000	Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
// #define MY_NODE_ID 							103

// #endif







#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen



volatile bool 		informGW = false;
bool 				firstLoop = true;

uint8_t 			debugLevel = 0;								// sets the debug level, 0 = basic info. 1 = streaming level info. 2 = sent level streaming to gateway.
uint8_t 			LowEnergyCounter = 0;						// Falls PV-Leistung höher als Bezug, dann soll nach 5xSEND_FREQUENCY 0 Watt angezeigt werden


uint32_t 			oldPulseCount = 0;
uint32_t 			oldWatt = 0;
uint32_t 			lastSend;
uint32_t 			lastHeartBeat = 0;
uint32_t 			lastInternalsUpdate = 0; 

volatile uint32_t 	pulseCount = 0;
volatile uint32_t 	lastPulseTime = 0;
volatile uint32_t 	watt = 0;



void preHwInit() //kein serieller Output 
{
	//Serielles Interface nur setzten, falls nicht schon von MySensors Framework erledigt
	#if !defined MY_DEBUG
		DEBUG_SERIAL(MY_BAUD_RATE);	// MY_BAUD_RATE from MyConfig.h 115200ul, gehört nach preHwInit. Falls in before(), friert der Arduino ein
	#endif
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
		writeEeprom32(EEPROM_METER_VALUE, DEFAULT_METER_VALUE);	
		DEBUG_PRINT("EEPROM_METER_VALUE to default");
		MeterValue=DEFAULT_METER_VALUE;
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
	digitalWrite(PULSE_LED,LED_OFF);
	
	pinMode(DIGITAL_INPUT_SENSOR,INPUT_PULLUP);

	lastHeartBeat=millis();
	lastSend=lastHeartBeat;
	lastPulseTime = lastSend;
	lastInternalsUpdate = lastSend;
	
	attachInterrupt(digitalPinToInterrupt(DIGITAL_INPUT_SENSOR), onPulse, RISING);
	
}

void presentation()
{
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);
	myPresentation();
	present(CHILD_POWER_METER, S_POWER, CHILD_POWER_METER_TEXT);
	// wait(100);
	if (not(firstLoop))
	{
		myHeartBeatLoop(); //bei presentation direkt noch Reboot ist der Node noch nicht registriert und kann keine Werte senden
	}
}

void loop()
{
	if (pulseCount != oldPulseCount) 
	{	
		wait(150);//LED noch etwas länger leuchten lassen
		digitalWrite(PULSE_LED,LED_OFF);
	}
	
	uint32_t currentTime = millis();
	

	if ((((currentTime - lastHeartBeat) > (uint32_t)HEARTBEAT_INTERVAL)) || firstLoop || informGW)
	{
		lastHeartBeat = currentTime;
		DEBUG_PRINT("informGW: ");
		DEBUG_PRINTLN(informGW);
		if (informGW)
		{
			
			informGW = false;
			send(msgPowerMeter.setType(V_VAR1).set(pulseCount));
		}
		// onPulse();//debug ohne angeschlossenen Sensor
		myHeartBeatLoop(); //deaktivert lassen, war schon bei presentation gesendet worden
	}

	//Serviceroutine, welche alle 60 Minuten läuft um Werte für FHEM Grafik aktuell zu halten
	if (((currentTime - lastInternalsUpdate) > (uint32_t)INTERNALS_UPDATE_INTERVAL) || firstLoop)
	{
		lastInternalsUpdate = currentTime;
		if (firstLoop)
		{
			DEBUG_PRINTLN("firstLoop IT");
			firstLoop = false;
		}
		else
		{
			DEBUG_PRINTLN("IT: Write EEPROM_METER_VALUE");
			writeEeprom32(EEPROM_METER_VALUE, pulseCount);	
			myHeartBeatLoop();
		}
		send(msgDebugLevel.set(debugLevel));
		send(msgNewMeterValue.set(pulseCount));


		
	}
	
	if (LowEnergyCounter >= (uint8_t) MAX_CYCLES_LOW_ENERGY)
	{
		LowEnergyCounter=0;
		watt = oldWatt = 0;
		send(msgPowerMeter.setType(V_WATT).set(watt));  // Send watt value to gw
	}
	
	if ((currentTime - lastSend) > (uint32_t)SEND_FREQUENCY) 
	{
		lastSend = currentTime;
		#ifdef SER_DEBUG
			DEBUG_PRINT("SEND_FREQUENCY: ");
			DEBUG_PRINTLN(SEND_FREQUENCY);
			// onPulse();
		#endif
		if (watt != oldWatt) 
		{
			// Check that we dont get unresonable large watt value.
			// could hapen when long wraps or false interrupt triggered
			if (watt<(uint32_t)MAX_WATT) 
			{
				send(msgPowerMeter.setType(V_WATT).set(watt));  // Send watt value to gw
			}
			else
			{			
				// Debug Kanal informieren
				send(msgDebugReturnString.set(watt));
			}
			DEBUG_PRINT("Watt:");
			DEBUG_PRINTLN(watt);
			oldWatt = watt;
		}


		// Pulse cout has changed
		if (pulseCount != oldPulseCount) 
		{	
			DEBUG_PRINTLN("New PULSE");
			send(msgPowerMeter.setType(V_VAR1).set(pulseCount));  
			float kwh = ((float)pulseCount/((float)PULSE_FACTOR));
			oldPulseCount = pulseCount;
			send(msgPowerMeter.setType(V_KWH).set(kwh, 3));  // Send kwh value to gw
			LowEnergyCounter=0;	//Counter resetten

		}
		else
		{
			DEBUG_PRINTLN("OLD PULSE");
			LowEnergyCounter++;
		}

	} 
}

void receive(const MyMessage &message)
{
	if ((message.sensor == CHILD_NEW_METER_VALUE) && !message.isEcho())
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
				debugLevel = message.getByte();
				if (debugLevel == 0)
				{
					send(msgDebugReturnString.set(F("dbg3 0")));
					//ToDo: noch nicht implementiert. debugLevel Variable wird auch nicht im EEPROM gespeichert
				}	
				if (debugLevel == 1)
				{
					send(msgDebugReturnString.set(F("dbg3 1")));
				}
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
	uint32_t newPulseTime = millis();
	uint32_t interval = newPulseTime-lastPulseTime;

	if (interval<(uint32_t)100) { // Sometimes we get interrupt on RISING
		DEBUG_PRINTLN("onPulse: return");
		return;
	}
	//3600 seconds per hour = 3600J per pulse i.e. 1 Wh = 3600J therefore, instantaneous power P = 3600 / T where T is the time between the falling edge of each pulse.
	watt = ((float)(3600000000.0 / PULSE_FACTOR ) / interval ); //wegen millis 3600 * 1000 = 3600000.0
	lastPulseTime = newPulseTime;
	pulseCount++;
	DEBUG_PRINT("onPulse:");
	DEBUG_PRINTLN(pulseCount);
	DEBUG_PRINT("interval:");
	DEBUG_PRINTLN(interval);
	DEBUG_PRINT("pulse-watt:");
	DEBUG_PRINTLN(watt);
	digitalWrite(PULSE_LED,LED_ON);
}





