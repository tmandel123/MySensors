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
20221125 Version 2.0-001	Umstellung auf SML Daten (D0) beim Stromzähler -> Wichtig: MY_BAUD_RATE muss 9600 sein, sowohl für RX (vom Zähler) als auch TX (zur Konsole)

*************************************************/


#define SKETCH_VER            				"2.0-001"        		// Sketch version
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


#define MY_NODE_ID 							103
// #define MY_PARENT_NODE_ID 					100
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


#define MY_INDICATION_HANDLER									//erlaubt rewrite der Funktion void indication(indication_t ind) 



// ###################   Node Spezifisch   #####################

#define PULSE_FACTOR						1000				// Nummber of blinks per KWH of your meeter
#define MAX_WATT							12000				// Max watt value to report. This filetrs outliers.

#define EEPROM_DEVICE_DEBUG_LEVEL			0					//8  Bit	Position im Flash
#define EEPROM_METER_VALUE					1					//32 Bit	Position im Flash
#define	DEFAULT_METER_VALUE					18010892			// last seen Value from Enerny Meter to set to the EEPROM
#define MAX_DEBUG_LEVEL         			9

// Sonstige Werte


#define HEARTBEAT_INTERVAL					300000				//default: 300000
#define INTERNALS_UPDATE_INTERVAL			3600000				//default: 3600000	jede Stunde Update senden (Debug, Threshold usw)
#define SEND_FREQUENCY						5000				//default: 30000	Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
#define WAIT_TIME							2000				//default: 2000		Wartezeit am Ende von loop(), wait verarbeitet eingehende Nachrichten
// #define MAX_CYCLES_LOW_ENERGY				3					//default: 3		nach so vielen Durchläufen 0 Watt anzeigen


#define MY_BAUD_RATE						(9600ul)			//DEBUG_PRINT nur mit dieser Datenrate ausgeben, weil Hardware Serial auch zum Einlesen benötigt wird
// #define LED 								LED_BUILTIN
/** Werte für Debug überschreiben **/
// #ifdef SER_DEBUG

// #define HEARTBEAT_INTERVAL					10000				//default: 300000
// #define INTERNALS_UPDATE_INTERVAL			3600000				//default: 3600000	jede Stunde Update senden (Debug, Threshold usw)
// #define SEND_FREQUENCY						15000				//default: 30000	Minimum time between send (in milliseconds). We don't wnat to spam the gateway.
// #define MY_NODE_ID 							103

// #endif


// ###################   Allgemeine MySensors Funktionen aus CommonFunctions.h (erhöht den Speicherverbrauch   #####################

// #define	WITH_BATTERY						//DS18B20 Sensoren funktionieren nicht mit weniger als 2,5V, eigentlich müsste OneWireMaster mit 5V und Netzteil betriebern werden
#define WITH_HWTIME
#define WITH_RF24_INFO						// übermittel RSSI, PA_Level, RF_Channel
#define WITH_NODE_INFO						// übermittel NodeID, ParentNodeID



#include "sml.h"
// #include <stdio.h>

#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen



// volatile bool 		informGW = false;
bool 				firstLoop = true;

uint8_t 			debugLevel = 0;								// sets the debug level, 0 = basic info. 1 = streaming level info. 2 = sent level streaming to gateway.
uint32_t 			lastSend;
uint32_t 			lastHeartBeat = 0;
uint32_t 			lastInternalsUpdate = 0; 

uint32_t 			lastPulseTime = 0;


signed long 		PowerAll		= 0;
signed long 		PowerL1			= 0;
signed long 		PowerL2			= 0;
signed long 		PowerL3			= 0;
signed long 		SumWhConsum		= -2;
signed long 		SumWhFeed		= -2;

// AltSoftSerial 		inputSerial;
sml_states_t 		currentState;



typedef struct {
  const unsigned char OBIS[6];
  void (*Handler)();
} OBISHandler;

void fPowerAll() {
  smlOBISW(PowerAll);
}
void fPowerL1()  {
  smlOBISW(PowerL1);
}
void fPowerL2()  {
  smlOBISW(PowerL2);
}
void fPowerL3()  {
  smlOBISW(PowerL3);
}

void fCounterSumIn()  {
  smlOBISWh(SumWhConsum);
}
void fCounterSumOut() {
  smlOBISWh(SumWhFeed);
}


OBISHandler OBISHandlers[] = {
  {{0x01, 0x00, 0x10, 0x07, 0x00, 0xff}, &fPowerAll},
  {{0x01, 0x00, 0x24, 0x07, 0x00, 0xff}, &fPowerL1},
  {{0x01, 0x00, 0x38, 0x07, 0x00, 0xff}, &fPowerL2},
  {{0x01, 0x00, 0x4c, 0x07, 0x00, 0xff}, &fPowerL3},
  {{0x01, 0x00, 0x01, 0x08, 0x00, 0xff}, &fCounterSumIn},
  {{0x01, 0x00, 0x02, 0x08, 0x00, 0xff}, &fCounterSumOut},
  {{0}, 0}
};

void readByte(unsigned char currentChar)
{
	currentState = smlState(currentChar);
	unsigned int iHandler = 0;
	if (currentState == SML_LISTEND) {
        for (iHandler = 0; OBISHandlers[iHandler].Handler != 0 && !(smlOBISCheck(OBISHandlers[iHandler].OBIS)); iHandler++);
       
        if (OBISHandlers[iHandler].Handler != 0) {
			OBISHandlers[iHandler].Handler();
        }
	}
	
}

void preHwInit() //kein serieller Output 
{
	//Serielles Interface nur setzten, falls nicht schon von MySensors Framework erledledigt
	// #if defined SER_DEBUG
		// DEBUG_SERIAL(MY_BAUD_RATE);	// MY_BAUD_RATE from MyConfig.h 115200ul, gehört nach preHwInit. Falls in before(), friert der Arduino ein
		// delay(150);
		// DEBUG_PRINTLN("starting serial interface with debugging");
	// #else
		// Serial.begin(MY_BAUD_RATE);
		// delay(150);
		// Serial.println(F("starting serial interface without debugging"));
	// #endif
}

void before() 
{
	
	//Serielles Interface nur setzten, falls nicht schon von MySensors Framework erledledigt
	#if defined SER_DEBUG
		DEBUG_SERIAL(MY_BAUD_RATE);	// MY_BAUD_RATE from MyConfig.h 115200ul, gehört nach preHwInit. Falls in before(), friert der Arduino ein
		DEBUG_PRINTLN("starting serial interface with debugging");
	#else
		Serial.begin(MY_BAUD_RATE);
		Serial.println(F("starting serial interface without debugging"));
	#endif

	
	
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
}

void setup()
{
	DEBUG_PRINTLN("setup: ");

	lastHeartBeat=millis();
	lastSend=lastHeartBeat;
	lastPulseTime = lastHeartBeat - (uint32_t)SEND_FREQUENCY ;	//Wert reduzieren, damit gleich zu beginn im loop() die serielle Schnittstelle abgefragt wird.
	lastInternalsUpdate = lastHeartBeat;
}

void presentation()
{
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);
	myPresentation();
	present(CHILD_POWER_METER, S_POWER, 	CHILD_POWER_METER_TEXT);
	present(CHILD_POWER_PHASE, S_CUSTOM, 	CHILD_POWER_PHASE_TEXT);
	// wait(100);
	if (not(firstLoop))
	{
		myHeartBeatLoop(); //bei presentation direkt noch Reboot ist der Node noch nicht registriert und kann keine Werte senden
	}
}

void loop()
{
	uint32_t currentTime = millis();
	
	if ((currentTime - lastPulseTime) >  SEND_FREQUENCY)
	{
		unsigned char incomingByte;

		if (Serial.available() > 0) {

			incomingByte = Serial.read();
			// Serial.write(incomingByte);
			// Serial.println(incomingByte, HEX);
			readByte(incomingByte);
			if (currentState != SML_FINAL)
			{
				return; //Loop von vorne beginnen
			}
		}
	}

	if (currentState == SML_FINAL) {
		
	#if defined SER_DEBUG
		DEBUG_PRINTLN(F("SML_FINAL"));
	#else
		Serial.println(F("SML_FINAL"));
	#endif

		DEBUG_PRINTLN(F("SML_FINAL"));

		DEBUG_PRINT(F("Power All: "));
		DEBUG_PRINTLN(PowerAll);

		DEBUG_PRINT(F("Power L1: "));
		DEBUG_PRINTLN(PowerL1);

		DEBUG_PRINT(F("Power L2: "));
		DEBUG_PRINTLN(PowerL2);

		DEBUG_PRINT(F("Power L3: "));
		DEBUG_PRINTLN(PowerL3);

		DEBUG_PRINT(F("Bezug: "));
		DEBUG_PRINTLN(SumWhConsum);

		DEBUG_PRINT(F("Einspeisung: "));
		DEBUG_PRINTLN(SumWhFeed);
		
		send(msgPowerMeter.setType(V_WATT).set(PowerAll));  // Send watt value to gw	
		send(msgPowerMeter.setType(V_VAR).set(SumWhConsum));  // Send watt value to gw	
		send(msgPowerMeter.setType(V_VAR1).set(SumWhFeed));  // Send watt value to gw	
		wait(100);
		// 10_MYSENSORS_DEVICE.pm S_POWER => { receives => [V_VAR1], sends => [V_WATT,V_KWH,V_VAR,V_VA,V_POWER_FACTOR,V_VAR1] }, # Power measuring device, like power meters	
		send(msgPowerPhase.setType(V_VAR1).set(PowerL1));  
		send(msgPowerPhase.setType(V_VAR2).set(PowerL2));  
		send(msgPowerPhase.setType(V_VAR3).set(PowerL3));  
	
		float kwh = ((float)SumWhConsum/((float)PULSE_FACTOR));
		send(msgPowerMeter.setType(V_KWH).set(kwh, 3));  // Send kwh value to gw
			
		lastPulseTime = currentTime;
		currentState = SML_UNEXPECTED;
		

	}
	
	
	if ((currentTime - lastHeartBeat) > (uint32_t)HEARTBEAT_INTERVAL)
	{
		lastHeartBeat = currentTime;
		myHeartBeatLoop(); //deaktivert lassen, war schon bei presentation gesendet worden
	}

	//Serviceroutine, welche alle 60 Minuten läuft um Werte für FHEM Grafik aktuell zu halten
	if (((currentTime - lastInternalsUpdate) > (uint32_t)INTERNALS_UPDATE_INTERVAL) || firstLoop)
	{
		firstLoop = false;
		lastInternalsUpdate = currentTime;
		send(msgDebugLevel.set(debugLevel));
	}
	
	// wait(WAIT_TIME);
}

void receive(const MyMessage &message)
{
	DEBUG_PRINT(F("rec Sens: "));
	DEBUG_PRINTLN(message.sensor);
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





