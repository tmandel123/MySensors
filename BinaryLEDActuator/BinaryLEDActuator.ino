/**
ToDo in FHEM

attr MYSENSOR_110 setCommands on:status_Switch:on off:status_Switch:off

**/


//	###################   Debugging   #####################
#define MY_DEBUG
#define SER_DEBUG
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
// #define MY_DEFAULT_RX_LED_PIN				17
// #define MY_DEFAULT_TX_LED_PIN 				18
// #define MY_DEFAULT_ERR_LED_PIN				19
// #define MY_DEFAULT_LED_BLINK_PERIOD 		10

// ###################   Transport   #####################
/*
RF24_PA_MIN = 	-18dBm 		0	R_TX_Powerlevel_Pct 	25
RF24_PA_LOW = 	-12dBm 		1	R_TX_Powerlevel_Pct
RF24_PA_HIGH = 	-6dBm 		2	R_TX_Powerlevel_Pct
RF24_PA_MAX = 	 0dBm		3	R_TX_Powerlevel_Pct
*/

#define MY_RF24_PA_LEVEL 					RF24_PA_LOW
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
// #define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)
#define MY_TRANSPORT_SANITY_CHECK

#define MY_NODE_ID 							110
// #define MY_PARENT_NODE_ID 					100		//without this the node broadcasts everything to parent 255 (dont know what happens, if 2 repeater receive this at the same time)
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.0-001"        			// Sketch version
#define SKETCH_NAME           				"LEDSwitch"   		// Optional child sensor name

#define HEARTBEAT_INTERVAL        			300000        //später alle 5 Minuten, zum Test alle 30 Sekunden



// #define	WITH_BATTERY

#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen



#define FADE_DELAY 							2  					// Delay in ms for each percentage fade up/down (10ms = 1s full-range dim)
#define MAX_LED_LEVEL						1
#define LED_LEVEL_EEPROM					0

uint32_t lastHeartBeat = HEARTBEAT_INTERVAL - 10000; //das erste Mal sollte nach 5 Sekunden etwas passieren
uint8_t currentLevel = 0;  // Current dim level...



void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	delay(50); // bei weniger als 18ms kommt soetwas hier: p⸮Y⸮%⸮⸮⸮⸮ 
	DEBUG_PRINTLN("preHwInit");
}

void before() 
{
	DEBUG_PRINTLN("before");
	pinMode(LED_DIGITAL_PIN, OUTPUT);   // sets the pin as output
	// showEEprom();
	uint8_t EEPROMLevel = loadState(LED_LEVEL_EEPROM);
	
	if (EEPROMLevel>MAX_LED_LEVEL)
	{
		currentLevel=0;
		DEBUG_PRINTLN(F("Save: LED_LEVEL_EEPROM to 0"));
		saveState(LED_LEVEL_EEPROM, currentLevel);//8 Bit
	}
	
	DEBUG_PRINT("EEPROMLevel: ");
	DEBUG_PRINTLN(EEPROMLevel);
	digitalWrite( LED_DIGITAL_PIN, EEPROMLevel );
	currentLevel = EEPROMLevel;
}


/***
 * Dimmable LED initialization method
 */
void setup()
{
	
	DEBUG_PRINTLN("Setup");
	// first_start = true;
	// Pull the gateway's current dim level - restore light level upon node power-up
	send( msgSwitchState.set(currentLevel) );
}

void presentation()
{
	myPresentation();
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);  
	present(CHILD_SINGLE_LED_SWITCH, S_BINARY, CHILD_SINGLE_LED_SWITCH_TEXT);
}

/***
 *  Dimmable LED main processing loop
 */
void loop()
{
	unsigned long currentTime = millis();
	if (currentTime - lastHeartBeat > (unsigned long)HEARTBEAT_INTERVAL)
	{
		DEBUG_PRINTLN("HEARTBEAT_INTERVAL");
		sendHeartbeat();
		myHeartBeatLoop();
		lastHeartBeat = currentTime;
		send( msgSwitchState.set(currentLevel > 0) );
	}
}



void receive(const MyMessage &message)
{
	if (message.type == V_STATUS) 
	{
		//  Retrieve the power or dim level from the incoming request message
		int requestedLevel = atoi( message.data );
		// Clip incoming level to valid range of 0 to 100
		requestedLevel = requestedLevel > 1 ? 1 : requestedLevel;
		requestedLevel = requestedLevel < 0 ? 0 : requestedLevel;

		DEBUG_PRINT( "Changing level to " );
		DEBUG_PRINT( requestedLevel );
		DEBUG_PRINT( ", from " );
		DEBUG_PRINTLN( currentLevel );

		digitalWrite(LED_DIGITAL_PIN, requestedLevel );
		
		saveState(LED_LEVEL_EEPROM,	requestedLevel);
		DEBUG_PRINT("LED_LEVEL_EEPROM New Value: ");
		DEBUG_PRINTLN(loadState(LED_LEVEL_EEPROM));
		// Inform the gateway of the current DimmableLED's SwitchPower1 and LoadLevelStatus value...
		currentLevel = requestedLevel;
		send(msgSwitchState.set(currentLevel > 0));
	}
}

