#define MY_DEBUG								//nur mit Debug aktiviert können Sends im Abstand von 50ms weitergeleitet werden. Sonst gibt es zu viele NACKs
#define MY_REPEATER_FEATURE
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 96
#define MY_TRANSPORT_WAIT_READY_MS (5000ul)

#define MY_NODE_ID 51
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC

#define MY_RF24_PA_LEVEL RF24_PA_MAX	//NodeID 50, seit MySensors 2.3.1 scheint auch PA_MAX zu funktionieren (Shielded Modul)
// #define MY_RF24_PA_LEVEL RF24_PA_HIGH
// #define MY_RF24_PA_LEVEL RF24_PA_LOW //läuft ohne Fehler, aber keine große Reichweite
/*
RF24_PA_MIN = -18dBm 
RF24_PA_LOW = -12dBm 
RF24_PA_HIGH = -6dBm 
RF24_PA_MAX = 0dBm
*/



#define SKETCH_VER            			"1.2-005"        			// Sketch version
#define SKETCH_NAME           			"Repeater Node"   		// Optional child sensor name

#define HEARTBEAT_INTERVAL        		60000        //später alle 5 Minuten, zum Test alle 30 Sekunden
#define CHILD_ID_TEXT					0


#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen
// #include <CommonFunctions.h>



MyMessage hwTime		(CHILD_ID_TEXT, V_TEXT);
unsigned long lastHeartBeat = HEARTBEAT_INTERVAL - 8000;


void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit...");
}

void before()
{
	DEBUG_PRINTLN("before...");
	DEBUG_PRINT("Version: ");
	DEBUG_PRINTLN(SKETCH_VER);
}

void setup()
{

}

void presentation()
{
	DEBUG_PRINTLN("presentation...");
	mySendSketchInfo();
	present(CHILD_ID_TEXT, S_INFO, "Info");
}

void loop()
{
	unsigned long currentTime = millis();
	if (currentTime - lastHeartBeat > (unsigned long)HEARTBEAT_INTERVAL)
	{
		sendHeartbeat();  
		send(hwTime.set(currentTime),true);
		lastHeartBeat = currentTime;
		// Serial.println("HEARTBEAT_INTERVAL erreicht");
	}
}
