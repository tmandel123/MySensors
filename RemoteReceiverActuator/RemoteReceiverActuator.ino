
// Enable debug prints to serial monitor
#define MY_DEBUG

#define SER_DEBUG
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
#define MY_RADIO_RF24

#define MY_NODE_ID 224
// #define MY_PARENT_NODE_ID 0
// #define MY_PARENT_NODE_IS_STATIC
#define MY_RF24_CHANNEL 96									// Für Testphase deaktivieren, damit Kanal 76 aktiv wird (Prod=96 Test=76)


#define MY_RF24_PA_LEVEL RF24_PA_MAX
#define MY_TRANSPORT_WAIT_READY_MS (5000ul)
// #define MY_INDICATION_HANDLER



#include <MySensors.h>

#define SKETCH_NAME "RemoteReceiverActuator"
#define SKETCH_VER "1.0-001"

#define LED_PIN 					3      				// Arduino pin attached to MOSFET Gate pin
#define FADE_DELAY 					2  					// Delay in ms for each percentage fade up/down (10ms = 1s full-range dim)

#define CHILD_ID_LED       			0     				//ID für Child, welches den MOSFET per PWM ansteuert
#define CHILD_ID_LED_TEXT			"NANO IO SHIELD"
#define LED_CHILD_0_EEPROM			0

#define HEARTBEAT_INTERVAL	30000        //später alle 5 Minuten, zum Test alle 30 Sekunden
unsigned long lastHeartBeat = HEARTBEAT_INTERVAL - 5000; //das erste Mal sollte nach 5 Sekunden etwas passieren
static int16_t currentLevel = 0;  // Current dim level...
MyMessage dimmerMsg(0, V_DIMMER);
MyMessage lightMsg(0, V_LIGHT);


void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit...");
}

void before() 
{
	DEBUG_PRINTLN("before: ");
}



void setup()
{
	DEBUG_PRINTLN("Setup");
}



void presentation()
{
	DEBUG_PRINTLN("presentation...");
	sendSketchInfo(SKETCH_NAME, SKETCH_VER " " __TIME__ " " __DATE__);
	present(CHILD_ID_LED, S_DIMMER, CHILD_ID_LED_TEXT);

}


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
	DEBUG_PRINT("Type: ");
	DEBUG_PRINTLN(message.type);
	
	DEBUG_PRINT("data: ");
	DEBUG_PRINTLN(atoi( message.data ));	
	
	DEBUG_PRINT("sensor: ");
	DEBUG_PRINTLN(message.sensor);

	DEBUG_PRINT("getULong: ");
	DEBUG_PRINTLN(message.getULong());	
	
	DEBUG_PRINT("getString(): ");
	DEBUG_PRINTLN(message.getString());

}





