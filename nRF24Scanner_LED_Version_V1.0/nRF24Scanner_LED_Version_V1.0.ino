// #define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_NODE_ID 210
#define MY_PARENT_NODE_ID 50

// #define MY_RF24_CHANNEL 96									// Nach Testphase deaktivieren, damit Kanal 76 aktiv wird

#define NODE_TXT "Ack Test Node"
#define MY_INDICATION_HANDLER
#define MY_RF24_PA_LEVEL RF24_PA_LOW
#define MY_TRANSPORT_WAIT_READY_MS (1000ul)

#define SER_DEBUG

#include <MySensors.h>

#define SN "Ack-Tester"
#define SV "1.0-001"

#define LED_PIN 						6											// Arduino pin attached to MOSFET Gate pin
#define CHILD_ID_TEXT					0
#define SEND_WAIT						10
#define REQUEST_ACK						true

#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif

#define HEARTBEAT_INTERVAL	2000        //später alle 5 Minuten, zum Test alle 30 Sekunden
unsigned long lastHeartBeat = 0;
unsigned int counter=0;
bool TransportUplink = true;

// MyMessage dimmerMsg(0, V_DIMMER);
MyMessage hwTime		(CHILD_ID_TEXT, V_TEXT);


void preHwInit()
{
	Serial.begin(115200);
	Serial.println("preHwInit...");
	pinMode(LED_PIN, OUTPUT);   // sets the pin as output
	digitalWrite( LED_PIN, HIGH );
	LED_Blink(5,2);//Anzahl, Geschwindigkeit
	
}

void before()
{
	Serial.println("before...");
	LED_Blink(5,1);//Anzahl, Geschwindigkeit
}

/***
 * Dimmable LED initialization method
 */
void setup()
{
	Serial.println("Setup...");
	// Pull the gateway's current dim level - restore light level upon sendor node power-up
	request( 0, V_DIMMER );
	//wdt_reset();
	//wdt_disable();
	
	LED_Blink(2,2);//Anzahl, Geschwindigkeit
	LED_Blink(10,1);//Anzahl, Geschwindigkeit
}

void presentation()
{
	// Register the LED Dimmable Light with the gateway
	present(CHILD_ID_TEXT, S_INFO, NODE_TXT);
	// present( 0, S_DIMMER, NODE_TXT );
	sendSketchInfo(SN, SV);
	LED_Blink(1,2);//Anzahl, Geschwindigkeit
}

/***
 *  Dimmable LED main processing loop
 */
void loop()
{
	unsigned long currentTime = millis();
	
	TransportUplink = send(hwTime.set(currentTime), REQUEST_ACK);
	// DEBUG_PRINT("Uplink Check: ");
	// DEBUG_PRINTLN(TransportUplink);	
	wait(SEND_WAIT);
	wait(100);

	if (TransportUplink == false)
	{
		Serial.println(F("transportCheckUplink = false"));
		LED_Blink(5,1);//Anzahl, Geschwindigkeit();
		// LED_Blink(3,2);//Anzahl, Geschwindigkeit();
	}

}

 
// ....
void indication(indication_t ind)
{
  switch(ind)
  {
    case INDICATION_TX: 
		// Serial.println(F("TX"));
		// LED_Blink(1,1);//Anzahl, Geschwindigkeit
		break;
    case INDICATION_ERR_FIND_PARENT:          	Serial.println(F("ERR_FIND_PARENT")); break;
    case INDICATION_GOT_PARENT:          		Serial.println(F("GOT_PARENT")); break;
    case INDICATION_ERR_HW_INIT: 				Serial.println(F("ERR_HW_INIT"));
		LED_Blink(10,1);//Anzahl, Geschwindigkeit
		break;
    case INDICATION_ERR_TX:
		Serial.println(F("ERR_TX"));
		LED_Blink(1,2);//Anzahl, Geschwindigkeit
		break;
    case INDICATION_ERR_CHECK_UPLINK:      		Serial.println(F("ERR_CHECK_UPLINK")); break;

  }
}

void LED_Blink(int Anzahl, int Dauer)
{

	int OffTime;
	int BlinkTime;
	if (Dauer == 1)
	{
		BlinkTime=40;
		OffTime=20;
	}
	else
	{
		BlinkTime=200;
		OffTime=100;
	}
	// Serial.print("BlinkTime: ");
	// Serial.println(BlinkTime);

	for (int i=0; i<Anzahl; i++){
		digitalWrite( LED_PIN, HIGH );
		wait(BlinkTime);
		digitalWrite( LED_PIN, LOW );
		wait(OffTime);
	}
}

// void receive(const MyMessage &message)
// {
	// Serial.println("new message data");
	// Serial.print("type: ");
	// Serial.println(message.type);
	// Serial.print("data: ");
	// Serial.println(message.data);
// }

