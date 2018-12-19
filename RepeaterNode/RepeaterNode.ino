#define MY_DEBUG								//nur mit Debug aktiviert können Sends im Abstand von 50ms weitergeleitet werden. Sonst gibt es zu viele NACKs

// Enable and select radio type attached
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 96
#define MY_NODE_ID 51
#define MY_PARENT_NODE_ID 100
#define MY_PARENT_NODE_IS_STATIC

// #define MY_RF24_PA_LEVEL RF24_PA_MAX
// #define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_RF24_PA_LEVEL RF24_PA_LOW //läuft ohne Fehler, aber keine große Reichweite

/*
RF24_PA_MIN = -18dBm 
RF24_PA_LOW = -12dBm 
RF24_PA_HIGH = -6dBm 
RF24_PA_MAX = 0dBm
*/
#define MY_TRANSPORT_WAIT_READY_MS (5000ul)


#define SKETCH_VER            			"1.2-003"        			// Sketch version
#define SKETCH_NAME           			"Repeater Node"   		// Optional child sensor name

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE
#define HEARTBEAT_INTERVAL        		60000        //später alle 5 Minuten, zum Test alle 30 Sekunden
#define CHILD_ID_TEXT					0


#include <MySensors.h>


MyMessage hwTime		(CHILD_ID_TEXT, V_TEXT);
unsigned long lastHeartBeat = HEARTBEAT_INTERVAL - 8000;


void setup()
{

}

void presentation()
{
	present(CHILD_ID_TEXT, S_INFO, "Info Child");
	//Send the sensor node sketch version information to the gateway
	sendSketchInfo(SKETCH_NAME, SKETCH_VER); 
	
}

void loop()
{
	unsigned long currentTime = millis();
	if (currentTime - lastHeartBeat > (unsigned long)HEARTBEAT_INTERVAL)
	{
		// sendHeartbeat();  
		send(hwTime.set(currentTime),true);
		lastHeartBeat = currentTime;
		// Serial.println("HEARTBEAT_INTERVAL erreicht");

	}
}
