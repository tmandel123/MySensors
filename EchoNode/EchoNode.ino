/***

Echo Node soll immer dort aufgestellt werden, wo später ein aktiver Aktor oder Sensor betrieben wird.
Grundsätzliche funktionsweise:

	- Der Controller (in Perl unter FHEM) sendet eine Info an den EchoNode 
	- Der Test dauert nicht ewig, sondern eine festgelegte Zeitspanne. z.B. 30 Sekunden
	- EchoNode wertet die empfangenen Nachrichten (auf Absender, Datentyp und ChildID) und sendet genau die selben Daten zurück
	- der Absender (Controller oder anderer Node) misst die Laufzeit und die Rückläuferquote
	
	
	Messung der Laufzeit von Nachrichten
	Statistik, wie viele Nachrichten durchkommen.
	
	statt eigenem LED_Blink folgendes verwenden #include <jled.h> //https://github.com/jandelgado/jled

*/

//	###################   Debugging   #####################
#define MY_DEBUG								//nur mit Debug aktiviert können Sends im Abstand von 50ms weitergeleitet werden. Sonst gibt es zu viele NACKs
#define SER_DEBUG
// #define MY_DEBUG_VERBOSE_RF24								//Testen, welche zusätzlichen Infos angezeigt werden
#define MY_SPLASH_SCREEN_DISABLED
#define MY_SIGNAL_REPORT_ENABLED

//	###################   Features   #####################
// #define MY_REPEATER_FEATURE

//	###################   LEDs   #####################
#define MY_WITH_LEDS_BLINKING_INVERSE
#define MY_DEFAULT_TX_LED_PIN 				(8)
#define MY_DEFAULT_LED_BLINK_PERIOD 		10

// ###################   Transport   #####################
/*
RF24_PA_MIN = -18dBm 
RF24_PA_LOW = -12dBm 
RF24_PA_HIGH = -6dBm 
RF24_PA_MAX = 0dBm
*/
#define MY_RF24_PA_LEVEL 					RF24_PA_LOW	//NodeID 50, seit MySensors 2.3.1 scheint auch PA_MAX zu funktionieren (Shielded Modul)
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
#define MY_TRANSPORT_WAIT_READY_MS			(5000ul)

#define MY_NODE_ID 							210
#define MY_PARENT_NODE_ID 					50
#define MY_PARENT_NODE_IS_STATIC
#define MY_PASSIVE_NODE


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.0-001"        			// Sketch version
#define SKETCH_NAME           				"Echo Node"   		// Optional child sensor name
#define NODE_TXT 							"Echo"
#define CHILD_ID_TEXT						0



#define LED_PIN 							6						// Arduino pin attached to MOSFET Gate pin
#define SEND_WAIT							20						// 20ms reicht nicht aus, dann kommt immer NACK, bei 22ms läuft es (mit Repeater sollten es 50ms sein)							
#define REQUEST_ACK							true

#define HEARTBEAT_INTERVAL        			60000        //später alle 5 Minuten, zum Test alle 30 Sekunden


#include <MySensors.h>

uint32_t lastHeartBeat = 0;
bool TransportUplink = true;

// MyMessage dimmerMsg(0, V_DIMMER);
MyMessage hwTime (CHILD_ID_TEXT, V_TEXT);


void preHwInit()
{
	
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit: ");
	pinMode(LED_PIN, OUTPUT);   // sets the pin as output

	
}

void before()
{
	DEBUG_PRINTLN("before...");
}


void presentation()
{
	DEBUG_PRINTLN("presentation...");
	mySendSketchInfo();
	present(CHILD_ID_TEXT, S_INFO, NODE_TXT);
}

void setup()
{
	DEBUG_PRINTLN("Setup...");
	// Pull the gateway's current dim level - restore light level upon sendor node power-up
	//wdt_reset();
	//wdt_disable();
}




void loop()
{
	uint32_t currentTime = millis();
	int16_t pRSSI=0;
	
	TransportUplink = send(hwTime.set(currentTime), REQUEST_ACK);
	wait(SEND_WAIT);

	//ToDo:
	pRSSI=RF24_getSendingRSSI();
	DEBUG_PRINT(F("pRSSI: "));
	DEBUG_PRINTLN(pRSSI);
	
	wait(1000);
	
	
	// if (TransportUplink == false)
	// {
		// Serial.println(F("No Ack"));
		// LED_Blink(5,1);//Anzahl, Geschwindigkeit();
		// LED_Blink(3,2);//Anzahl, Geschwindigkeit();
	// }

}

 
// ....
// void indication(indication_t ind)
// {
  // switch(ind)
  // {
    // case INDICATION_TX: 
		// Serial.println(F("TX"));
		// LED_Blink(1,1);//Anzahl, Geschwindigkeit
		// break;
    // case INDICATION_ERR_FIND_PARENT:          	Serial.println(F("ERR_FIND_PARENT")); break;
    // case INDICATION_GOT_PARENT:          		Serial.println(F("GOT_PARENT")); break;
    // case INDICATION_ERR_HW_INIT: 				Serial.println(F("ERR_HW_INIT"));
		// LED_Blink(10,1);//Anzahl, Geschwindigkeit
		// break;
    // case INDICATION_ERR_TX:
		// Serial.println(F("ERR_TX"));
		// LED_Blink(1,2);//Anzahl, Geschwindigkeit
		// break;
    // case INDICATION_ERR_CHECK_UPLINK:      		Serial.println(F("ERR_CHECK_UPLINK")); break;

  // }
// }

