// #define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_RF24_CHANNEL 96
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_PARENT_NODE_ID 0


// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE
#define HEARTBEAT_INTERVAL        	300000        //später alle 5 Minuten, zum Test alle 30 Sekunden

unsigned long lastHeartBeat = 0;

#include <MySensors.h>

void setup()
{

}

void presentation()
{
	//Send the sensor node sketch version information to the gateway
	sendSketchInfo("Repeater Node", "1.1");
}

void loop()
{
	unsigned long currentTime = millis();
	if (currentTime - lastHeartBeat > (unsigned long)HEARTBEAT_INTERVAL)
	{
		sendHeartbeat();  
		lastHeartBeat = currentTime;
		Serial.println("HEARTBEAT_INTERVAL erreicht");

	}
}
