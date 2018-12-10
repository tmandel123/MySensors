// #define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_RF24_CHANNEL 96
#define MY_NODE_ID 50
#define MY_RF24_PA_LEVEL RF24_PA_MAX
// #define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_PARENT_NODE_ID 0

#define MY_TRANSPORT_WAIT_READY_MS (5000ul)

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE
#define HEARTBEAT_INTERVAL        	60000        //später alle 5 Minuten, zum Test alle 30 Sekunden
#define CHILD_ID_TEXT				0


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
	sendSketchInfo("Repeater Node", "1.2-001");
	
}

void loop()
{
	unsigned long currentTime = millis();
	if (currentTime - lastHeartBeat > (unsigned long)HEARTBEAT_INTERVAL)
	{
		// sendHeartbeat();  
		send(hwTime.set(currentTime),true);
		lastHeartBeat = currentTime;
		Serial.println("HEARTBEAT_INTERVAL erreicht");

	}
}
