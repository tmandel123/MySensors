/**

#############################		Versionen		###################################
 
20230301 Version 1.07		mySendSketchInfo nach sendSketchInfo
20230307 Version 1.08		firstLoop eingeführt

*/

#define SKETCH_VER            				"1.08"        			// Sketch version
#define SKETCH_NAME           				"Gateway"   			// Optional child sensor name





//	###################   Debugging   #####################
// #define MY_DEBUG
// #define SER_DEBUG
#define MY_SPECIAL_DEBUG									// für Extended Debug in FHEM
//#define MY_DEBUG_VERBOSE_RF24								//Testen, welche zusätzlichen Infos angezeigt werden
// #define MY_SPLASH_SCREEN_DISABLED
// #define MY_SIGNAL_REPORT_ENABLED

// #define MY_RF24_DATARATE RF24_1MBPS //falls RF-Nano im Netzwerk mitmachen sollen

//	###################   Features   #####################
// #define MY_REPEATER_FEATURE
#define MY_GATEWAY_SERIAL
// #define MY_INCLUSION_MODE_FEATURE
// #define MY_INCLUSION_BUTTON_FEATURE
// #define MY_INCLUSION_MODE_BUTTON_PIN 3

//	###################   LEDs   #####################
// #define MY_WITH_LEDS_BLINKING_INVERSE
// #define MY_DEFAULT_TX_LED_PIN 				8
// #define MY_DEFAULT_RX_LED_PIN				7
// #define MY_DEFAULT_LED_BLINK_PERIOD 		10
// ###################   Transport   #####################
/*
RF24_PA_MIN = 	-18dBm 		0	R_TX_Powerlevel_Pct 	25
RF24_PA_LOW = 	-12dBm 		1	R_TX_Powerlevel_Pct		#Gateway Serial Kanal 96 hat mehr Reichweite mit PA_LOW als mit PA_HIGH
RF24_PA_HIGH = 	-6dBm 		2	R_TX_Powerlevel_Pct
RF24_PA_MAX = 	 0dBm		3	R_TX_Powerlevel_Pct
*/
#define MY_RF24_PA_LEVEL 					RF24_PA_LOW  //EchoNote hatte Max -> Reichweite bis Gartenhaus (-29) und noch Empfangen (-149) bis hinter Steins Haus
// #define MY_RF24_PA_LEVEL 					RF24_PA_MAX
#define MY_RADIO_RF24
// #define MY_RF24_CHANNEL 					96
// #define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)

// #define MY_NODE_ID 							52
// #define MY_PARENT_NODE_ID 					50
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE



// ###################   Node Spezifisch   #####################

#define HEARTBEAT_INTERVAL        			300000        //später alle 5 Minuten, zum Test alle 30 Sekunden


// ###################   Allgemeine MySensors Funktionen aus CommonFunctions.h (erhöht den Speicherverbrauch   #####################


#define WITH_HWTIME
#define WITH_RF24_INFO						// übermittel RSSI, PA_Level, RF_Channel
#define WITH_NODE_INFO						// übermittel NodeID, ParentNodeID
#define MY_INDICATION_HANDLER                 //erlaubt rewrite der Funktion void indication(indication_t ind) Tx_ERR, Tx_OK, TX_RSSI



#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen

uint32_t 	lastHeartBeat	= 0;
bool		firstLoop		= true;


void setup()
{

}

void presentation()
{
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);
	myPresentation();
	myHeartBeatLoop();
}

void loop()
{
	uint32_t currentTime = millis();
	if ((currentTime - lastHeartBeat > (uint32_t)HEARTBEAT_INTERVAL) || firstLoop)
	{
		myHeartBeatLoop();
		lastHeartBeat = currentTime;
		firstLoop=false;
	}
}

void receive(const MyMessage &message)
{
	
	// DEBUG_PRINTLN("receive");
	
	// DEBUG_PRINT("sensor: ");
	// DEBUG_PRINTLN(message.sensor);

	// DEBUG_PRINT("mGetPayloadType: ");
	// DEBUG_PRINTLN(mGetPayloadType(message));
	
	// DEBUG_PRINT("sender: ");
	// DEBUG_PRINTLN(message.sender);

	
	if ((message.sensor == CHILD_ECHO_TIMESTAMP) && !message.isEcho())
	{
		if (message.type == V_TEXT)
		{
			char buffer[14];
			message.getString(buffer);
			DEBUG_PRINT("GOT ");
			DEBUG_PRINTLN(buffer);
			
			DEBUG_PRINTLN("send test");
			
			
			
			MyMessage msg (CHILD_ECHO_TIMESTAMP, V_TEXT);
			msg.setDestination(message.sender);
			msg.set(buffer);
			send(msg); //senden erstmal deaktivieren (später liegen die automatisch als Echo zurückgegebenen Werte mittels isEcho im Client auswerten)
			
			
			
			// gatewayTransportSend(msg.setDestination(225).set("test2"));
			// DEBUG_PRINTLN("transportSendRoute test2");
			// transportSendRoute(msg.set("test2"));
		}
	}
}
