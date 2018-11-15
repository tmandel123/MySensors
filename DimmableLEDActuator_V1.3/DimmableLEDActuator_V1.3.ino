
// Enable debug prints to serial monitor
// #define MY_DEBUG

// #define SER_DEBUG
#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif


// Enable and select radio type attached
#define MY_RADIO_NRF24

#define MY_NODE_ID 210
// #define MY_PARENT_NODE_ID 50
// #define MY_PARENT_NODE_IS_STATIC
#define MY_RF24_CHANNEL 1									// Nach Testphase deaktivieren, damit Kanal 76 aktiv wird


#define MY_RF24_PA_LEVEL RF24_PA_LOW
#define MY_TRANSPORT_WAIT_READY_MS (5000ul)
// #define MY_INDICATION_HANDLER



#include <MySensors.h>

#define SKETCH_NAME "DimmableLED"
#define SKETCH_VER "1.3-002"

#define LED_PIN 					3      				// Arduino pin attached to MOSFET Gate pin
#define FADE_DELAY 					2  					// Delay in ms for each percentage fade up/down (10ms = 1s full-range dim)

#define CHILD_ID_LED       			0     				//ID für Child, welches den MOSFET per PWM ansteuert
#define CHILD_ID_LED_TEXT			"LED Dimming Test Node"
#define LED_CHILD_0_EEPROM			0

#define HEARTBEAT_INTERVAL	30000        //später alle 5 Minuten, zum Test alle 30 Sekunden
unsigned long lastHeartBeat = HEARTBEAT_INTERVAL - 5000; //das erste Mal sollte nach 5 Sekunden etwas passieren
static int16_t currentLevel = 0;  // Current dim level...
MyMessage dimmerMsg(0, V_DIMMER);
MyMessage lightMsg(0, V_LIGHT);


void preHwInit() 
{
#ifdef SER_DEBUG
	Serial.begin(115200);
#endif

	// DEBUG_PRINTLN("preHWini: ");
	// showEEprom();
}

void before() 
{
	pinMode(LED_PIN, OUTPUT);   // sets the pin as output
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
		DEBUG_PRINT("LED_CHILD_0_EEPROM New Value");
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
		analogWrite( LED_PIN, (int)(currentLevel / 100. * 255) );
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



