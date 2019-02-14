/***

Echo Node soll immer dort aufgestellt werden, wo später ein aktiver Aktor oder Sensor betrieben wird.
Grundsätzliche funktionsweise:

	
	Neue Idee:
	
	Node sendet jede 3 Sekunden eine Nachricht ans Gateway (mit Ack) und Zeitstempel (setzt lokal variable lastSend)
	Gateway leitet an Controller und Controller spiegelt die Nachricht zurück (ohne Ack)
	Node empfängt und misst die Zeitspanne oder macht nach Wartezeitspanne weiter
	Dann sendet Node die Statistik Datens ans Gateway
	Nach ablauf der Wartezeitspanne auf die Antwort sendet Node die nächste Nachricht
	
	
	Messung der Laufzeit von Nachrichten
	Statistik, wie viele Nachrichten durchkommen.
	
	statt eigenem LED_Blink folgendes verwenden #include <jled.h> //https://github.com/jandelgado/jled

*/

//	###################   Debugging   #####################
#define MY_DEBUG
#define SER_DEBUG
// #define MY_DEBUG_VERBOSE_RF24								//Testen, welche zusätzlichen Infos angezeigt werden
#define MY_SPLASH_SCREEN_DISABLED
// #define MY_SIGNAL_REPORT_ENABLED

//	###################   Features   #####################
// #define MY_REPEATER_FEATURE

//	###################   LEDs   #####################
// #define MY_WITH_LEDS_BLINKING_INVERSE
#define MY_DEFAULT_TX_LED_PIN 				8
#define MY_DEFAULT_RX_LED_PIN				7
#define MY_DEFAULT_LED_BLINK_PERIOD 		10

// ###################   Transport   #####################
/*
RF24_PA_MIN = -18dBm 
RF24_PA_LOW = -12dBm 
RF24_PA_HIGH = -6dBm 
RF24_PA_MAX = 0dBm
*/
#define MY_RF24_PA_LEVEL 					RF24_PA_LOW
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
// #define MY_TRANSPORT_WAIT_READY_MS			(5000ul)
// #define MY_RF24_SANITY_CHECK

#define MY_NODE_ID 							225
#define MY_PARENT_NODE_ID 					0
#define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.0-005"        			// Sketch version
#define SKETCH_NAME           				"Echo Node"   		// Optional child sensor name
#define MY_ECHO_NODE											//for some mir myPresentation() sendings

// #define LED_PIN 							6						// Arduino pin attached to MOSFET Gate pin
// #define SEND_WAIT							20						// 20ms reicht nicht aus, dann kommt immer NACK, bei 22ms läuft es (mit Repeater sollten es 50ms sein)							
// #define REQUEST_ACK							true

#define HEARTBEAT_INTERVAL        			300000        //später alle 5 Minuten, zum Test alle 30 Sekunden
// #define ECHO_TXFAIL_RESET_TIME     			10000
#define MAX_ECHO_WAIT	        			3000



#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen

bool 		GotEchoResponse = true;
uint32_t	TxFailCounter = 0;
uint32_t	TxGoodCounter = 0;
uint16_t	maxPingTime = 1;
uint16_t	minPingTime = 1000;
uint32_t 	lastHeartBeat = 0;
uint32_t	EchoLastSend = 0;
uint32_t	EchoTimeStamp = 0;
uint32_t	EchoReturn = 0;
uint32_t 	EchoRuntime = 0;


void preHwInit()
{
	
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit: ");
	// pinMode(LED_PIN, OUTPUT);   // sets the pin as output
}

void before()
{
	DEBUG_PRINTLN("before...");
}


void presentation()
{
	DEBUG_PRINTLN("presentation...");;
	mySendSketchInfo();
	myPresentation();
	myHeartBeatLoop();
}

void setup()
{
	DEBUG_PRINTLN("Setup...");
}




void loop()
{
	uint32_t currentTime = millis();
	uint32_t TimeSinceEchoSend = currentTime - EchoLastSend;
	uint32_t TimeSinceHeartBeat = currentTime - lastHeartBeat;
	

	
	
	if ((TimeSinceEchoSend > (uint32_t)MAX_ECHO_WAIT) || GotEchoResponse)
	{


		if (GotEchoResponse)
		{
			if (EchoRuntime > 0)
			{
				TxGoodCounter++;
				send(msgEchoRunTime.set(EchoRuntime));
				if (EchoRuntime > maxPingTime)
				{
					maxPingTime=EchoRuntime;
				}
				if (EchoRuntime < minPingTime)
				{
					minPingTime=EchoRuntime;
				}
			}
		}
		else
		{
			DEBUG_PRINTLN("NOT GotEchoResponse: ");
			TxFailCounter++;
		}
		
		uint32_t PacketRatio=(TxGoodCounter/(TxGoodCounter+TxFailCounter)*100);
		send(msgPacketRatio.set(PacketRatio));
		
		nowRSSI=RF24_getSendingRSSI();
		avgRSSI=((avgRSSI*7)+(nowRSSI))/8;
		send(msgSendingRSSI.set(avgRSSI));
	
		DEBUG_PRINT("TX Fail: ");
		DEBUG_PRINT(TxFailCounter);
		DEBUG_PRINT(" Ratio ");
		DEBUG_PRINT(PacketRatio);
		DEBUG_PRINT(" avgRSSI ");
		DEBUG_PRINT(avgRSSI);
		DEBUG_PRINT(" Ping ");
		DEBUG_PRINT(EchoRuntime);
		DEBUG_PRINT(" minPing ");
		DEBUG_PRINT(minPingTime);
		DEBUG_PRINT(" maxPing ");
		DEBUG_PRINTLN(maxPingTime);		
		
		EchoLastSend=millis();
		EchoTimeStamp=EchoLastSend;
		send(msgEchoTimeStamp.set(EchoTimeStamp));
		GotEchoResponse=false;
	}
	
	
	if ((TimeSinceHeartBeat > (uint32_t)HEARTBEAT_INTERVAL))
	{
		lastHeartBeat = currentTime;
		myHeartBeatLoop();
	}
}

void receive(const MyMessage &message)
{
	// DEBUG_PRINTLN("receive");
	if (!mGetAck(message) && message.sensor == CHILD_ECHO_TIMESTAMP && message.sender == 0)
	{
		if (message.type == V_TEXT)
		{
			EchoReturn = message.getULong();

			if (EchoReturn == EchoTimeStamp)
			{
				EchoRuntime=millis()-EchoTimeStamp;
				GotEchoResponse=true;
				EchoTimeStamp=0;
			}			
		}
	}
	
	// else
	// {
		// DEBUG_PRINTLN("unknown receive");
		
		// DEBUG_PRINT("sensor: ");
		// DEBUG_PRINTLN(message.sensor);

		// DEBUG_PRINT("mGetPayloadType: ");
		// DEBUG_PRINTLN(mGetPayloadType(message));
		
		// DEBUG_PRINT("sender: ");
		// DEBUG_PRINTLN(message.sender);

		// DEBUG_PRINT("version: ");
		// DEBUG_PRINTLN(message.version);
		
		// DEBUG_PRINT("getByte: ");
		// uint8_t Temp = message.getByte();
		// DEBUG_PRINTLN(Temp);
	// }
	// DEBUG_PRINTLN("receive");
	
	
	// DEBUG_PRINT("sensor: ");
	// DEBUG_PRINTLN(message.sensor);

	// DEBUG_PRINT("mGetPayloadType: ");
	// DEBUG_PRINTLN(mGetPayloadType(message));
	
	// DEBUG_PRINT("sender: ");
	// DEBUG_PRINTLN(message.sender);
		
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

