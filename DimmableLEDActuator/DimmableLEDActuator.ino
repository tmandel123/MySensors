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
// #define MY_DEFAULT_RX_LED_PWM_PIN				6
// #define MY_DEFAULT_TX_LED_PWM_PIN 				7
// #define MY_DEFAULT_ERR_LED_PWM_PIN				8
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
// #define MY_PARENT_NODE_ID 					51		//without this the node broadcasts everything to parent 255 (dont know what happens, if 2 repeater receive this at the same time)
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.0-001"        			// Sketch version
#define SKETCH_NAME           				"DimmableLED"   		// Optional child sensor name

#define HEARTBEAT_INTERVAL        			600000        //später alle 5 Minuten, zum Test alle 30 Sekunden



// #define	WITH_BATTERY

#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen



#define FADE_DELAY 					2  					// Delay in ms for each percentage fade up/down (10ms = 1s full-range dim)

#define CHILD_ID_LED       			0     				//ID für Child, welches den MOSFET per PWM ansteuert
#define CHILD_ID_LED_TEXT			"LED Dimming Test Node"
#define LED_CHILD_0_EEPROM			0


unsigned long lastHeartBeat = HEARTBEAT_INTERVAL - 5000; //das erste Mal sollte nach 5 Sekunden etwas passieren
static int16_t currentLevel = 0;  // Current dim level...
MyMessage dimmerMsg(0, V_DIMMER);
MyMessage lightMsg(0, V_LIGHT);


void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	// delay(20); // bei weniger als 18ms kommt soetwas hier: p⸮Y⸮%⸮⸮⸮⸮ 
	// DEBUG_PRINTLN("preHwInit");
}

void before() 
{
	pinMode(LED_PWM_PIN, OUTPUT);   // sets the pin as output
	DEBUG_PRINTLN("before: ");
	// showEEprom();
	int16_t EEPROMLevel = loadState(LED_CHILD_0_EEPROM);
	DEBUG_PRINT("EEPROMLevel: ");
	DEBUG_PRINTLN(EEPROMLevel);
	fadeToLevel( EEPROMLevel );
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
	send( dimmerMsg.set(currentLevel) );
}

void presentation()
{
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);  
	present(CHILD_ID_LED, S_DIMMER, CHILD_ID_LED_TEXT);
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
		lastHeartBeat = currentTime;
		send( dimmerMsg.set(currentLevel), false );
	}
}



void receive(const MyMessage &message)
{
	if (message.type == V_LIGHT || message.type == V_DIMMER) {

		//  Retrieve the power or dim level from the incoming request message
		int requestedLevel = atoi( message.data );
		// Adjust incoming level if this is a V_LIGHT variable update [0 == off, 1 == on]
		requestedLevel *= ( message.type == V_LIGHT ? 100 : 1 );//es kommt 0 oder 1 und das wird mit 100 multipliziert, falls 1 kommt (sonst so gelassen)
		// Clip incoming level to valid range of 0 to 100
		requestedLevel = requestedLevel > 100 ? 100 : requestedLevel;
		requestedLevel = requestedLevel < 0   ? 0   : requestedLevel;

		Serial.print( "Changing level to " );
		Serial.print( requestedLevel );
		Serial.print( ", from " );
		Serial.println( currentLevel );

		fadeToLevel( requestedLevel );
		
		saveState(LED_CHILD_0_EEPROM,	requestedLevel);
		DEBUG_PRINT("LED_CHILD_0_EEPROM New Value: ");
		DEBUG_PRINTLN(loadState(LED_CHILD_0_EEPROM));
		// Inform the gateway of the current DimmableLED's SwitchPower1 and LoadLevelStatus value...
		send(lightMsg.set(currentLevel > 0));

		// hek comment: Is this really nessesary?
		send( dimmerMsg.set(currentLevel) );


	}
}

/***
 *  This method provides a graceful fade up/down effect
 */
void fadeToLevel( int toLevel )
{
	DEBUG_PRINTLN("fadeToLevel()");
	int delta = ( toLevel - currentLevel ) < 0 ? -1 : 1;

	while ( currentLevel != toLevel ) {
		currentLevel += delta;
		analogWrite( LED_PWM_PIN, (int)(currentLevel / 100. * 255) );
		delay( FADE_DELAY );
	}
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
		Serial.println("");
	}
}



