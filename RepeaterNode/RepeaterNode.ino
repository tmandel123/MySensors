//	###################   Debugging   #####################
// #define MY_DEBUG								//nur mit Debug aktiviert können Sends im Abstand von 50ms weitergeleitet werden. Sonst gibt es zu viele NACKs
// #define SER_DEBUG
// #define MY_DEBUG_VERBOSE_RF24								//Testen, welche zusätzlichen Infos angezeigt werden
#define MY_SPLASH_SCREEN_DISABLED
// #define MY_SIGNAL_REPORT_ENABLED

//	###################   Features   #####################
#define MY_REPEATER_FEATURE

//	###################   LEDs   #####################
// #define MY_WITH_LEDS_BLINKING_INVERSE
#define MY_DEFAULT_RX_LED_PIN				6
#define MY_DEFAULT_TX_LED_PIN 				7
#define MY_DEFAULT_ERR_LED_PIN				8
#define MY_DEFAULT_LED_BLINK_PERIOD 		10

// ###################   Transport   #####################
/*
RF24_PA_MIN = 	-18dBm 		0	R_TX_Powerlevel_Pct 	25
RF24_PA_LOW = 	-12dBm 		1	R_TX_Powerlevel_Pct
RF24_PA_HIGH = 	-6dBm 		2	R_TX_Powerlevel_Pct
RF24_PA_MAX = 	 0dBm		3	R_TX_Powerlevel_Pct
*/
#define MY_RF24_PA_LEVEL 					RF24_PA_HIGH	//NodeID 50, seit MySensors 2.3.1 scheint auch PA_MAX zu funktionieren (Shielded Modul)
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
#define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)

#define MY_NODE_ID 							50
// #define MY_PARENT_NODE_ID 					50
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.2-007"        			// Sketch version
#define SKETCH_NAME           				"Repeater Node"   		// Optional child sensor name

#define HEARTBEAT_INTERVAL        			300000        //später alle 5 Minuten, zum Test alle 30 Sekunden


#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen



// MyMessage hwTime 							(CHILD_INFO, V_TEXT);

uint32_t lastHeartBeat = HEARTBEAT_INTERVAL - 5000; //das erste Mal sollte nach 5 Sekunden etwas passieren


void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit...");
}

void before()
{
	DEBUG_PRINTLN("before...");
}

void setup()
{

}

void presentation()
{
	DEBUG_PRINTLN("presentation...");
	mySendSketchInfo();
	myPresentation();
}

void loop()
{
	uint32_t currentTime = millis();
	if (currentTime - lastHeartBeat > (uint32_t)HEARTBEAT_INTERVAL)
	{
		sendHeartbeat();  
		lastHeartBeat = currentTime;
		myHeartBeatLoop();
	}
}
