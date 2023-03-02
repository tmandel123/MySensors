/*
1.0-007		2022-05-10	Anpassungen für Mysensors 2.3.2 begonnen

*/


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

#define SKETCH_VER            				"1.08"        			// Sketch version
#define SKETCH_NAME           				"Echo Node"   		// Optional child sensor name


//	###################   Debugging   #####################
#define MY_DEBUG												//Output kann im LogParser analysiert werden https://www.mysensors.org/build/parser
#define SER_DEBUG											// aus CommonFunctions.h für eigenes DEBUG_PRINT
#define MY_SPECIAL_DEBUG									// für Extended Debug in FHEM 'R': routing info 'V': CPU voltage 'F': CPU frequency 'M': free memory 'E': clear MySensors EEPROM area and reboot

// #define MY_DEBUG_VERBOSE_RF24								//Testen, welche zusätzlichen Infos angezeigt werden
// #define MY_SPLASH_SCREEN_DISABLED
// #define MY_SIGNAL_REPORT_ENABLED

//	###################   Features   #####################
// #define MY_REPEATER_FEATURE

//	###################   LEDs   #####################
// #define MY_WITH_LEDS_BLINKING_INVERSE
// #define MY_DEFAULT_RX_LED_PIN				17
// #define MY_DEFAULT_TX_LED_PIN 				18
// #define MY_DEFAULT_ERR_LED_PIN				19
// #define MY_DEFAULT_LED_BLINK_PERIOD 		10

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
// #define MY_TRANSPORT_SANITY_CHECK

#define MY_NODE_ID 							225
// #define MY_PARENT_NODE_ID 					50
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE

#define MY_INDICATION_HANDLER									//erlaubt rewrite der Funktion void indication(indication_t ind) 



// ###################   Node Spezifisch   #####################
#define MY_ECHO_NODE											//for some more myPresentation() sendings

// #define SEND_WAIT							20						// 20ms reicht nicht aus, dann kommt immer NACK, bei 22ms läuft es (mit Repeater sollten es 50ms sein)							
// #define REQUEST_ACK							true

#define HEARTBEAT_INTERVAL        			30000        		//später alle 5 Minuten, zum Test alle 30 Sekunden
#define MAX_ECHO_WAIT	        			500					//nach dieser Zeit wird der TxFailCounter hochgezählt
#define PING_INTERVAL	        			3000

#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen

volatile	bool 		GotEchoResponse = false;
volatile	bool 		CheckPingTimeout = false;
uint32_t				TxFailCounter = 0;
uint32_t				TxGoodCounter = 0;
uint16_t				maxPingTime = 1;
uint16_t				minPingTime = 1000;
uint32_t 				lastHeartBeat = 0;
// uint32_t				PingLastSend = 0;
uint32_t				PingTimeStamp = 0;
volatile	uint32_t	EchoReturn = 0;
volatile	uint32_t 	EchoRuntime = 0;


void preHwInit() //kein serieller Output 
{

}

void before() 
{
	DEBUG_PRINT("Channel / NodeID: ");
	DEBUG_PRINT(MY_RF24_CHANNEL);
	DEBUG_PRINT(" / ");
	DEBUG_PRINTLN(MY_NODE_ID);

}

void presentation()
{
	DEBUG_PRINTLN("presentation...");;
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);
	myPresentation();
	// kein eigenes present erforderlich, weil mittels MY_ECHO_NODE über CommonFunctions.h erledigt
	myHeartBeatLoop();
}

void setup()
{
	DEBUG_PRINTLN("Setup...");
}




void loop()
{
	uint32_t currentTime = millis();
	uint32_t TimeSinceLastPing = currentTime - PingTimeStamp;
	uint32_t TimeoutTime = currentTime - PingTimeStamp;
	uint32_t TimeSinceHeartBeat = currentTime - lastHeartBeat;
	
	// DEBUG_PRINTLN("loop");
	
	if (GotEchoResponse)
	{

		EchoRuntime=currentTime-EchoReturn;
		send(msgEchoRunTime.set(EchoRuntime));
		GotEchoResponse=false;
		CheckPingTimeout=false;
		TxGoodCounter++;
		
		// DEBUG_PRINT("EchoRuntime: ");
		// DEBUG_PRINT(EchoRuntime);
		// DEBUG_PRINT(" EchoReturn: ");
		// DEBUG_PRINT(EchoReturn);
		// DEBUG_PRINT(" minPing ");
		// DEBUG_PRINT(minPingTime);
		// DEBUG_PRINT(" maxPing ");
		// DEBUG_PRINTLN(maxPingTime);
		
		if (EchoRuntime > maxPingTime)
		{
			maxPingTime=EchoRuntime;
		}
		if (EchoRuntime < minPingTime)
		{
			minPingTime=EchoRuntime;
		}

		
		float PacketRatio=float(float(TxGoodCounter)/float(TxGoodCounter+TxFailCounter)*100.0);
		send(msgPacketRatio.set(PacketRatio,2));
		
		// nowRSSI=RF24_getSendingRSSI();
		// avgRSSI=((avgRSSI*7)+(nowRSSI))/8;
		// send(msgSendingRSSI.set(avgRSSI));
	
		DEBUG_PRINT("TX Fail: ");
		DEBUG_PRINT(TxFailCounter);
		DEBUG_PRINT(" TX Good: ");
		DEBUG_PRINT(TxGoodCounter);
		DEBUG_PRINT(" Ratio ");
		DEBUG_PRINT(PacketRatio);
		// DEBUG_PRINT(" avgRSSI ");
		// DEBUG_PRINT(avgRSSI);
		DEBUG_PRINT(" Ping ");
		DEBUG_PRINT(EchoRuntime);
		DEBUG_PRINT(" minPing ");
		DEBUG_PRINT(minPingTime);
		DEBUG_PRINT(" maxPing ");
		DEBUG_PRINTLN(maxPingTime);		
	}
	
	
	if (TimeSinceLastPing > (uint32_t)PING_INTERVAL)
	{	
		// PingLastSend=currentTime;
		PingTimeStamp=currentTime;
		send(msgEchoTimeStamp.set(PingTimeStamp));
		// DEBUG_PRINT("SendPing: ");
		// DEBUG_PRINTLN(PingTimeStamp);
		CheckPingTimeout=true;
		TimeoutTime = 0;	//wird im nächsten Loop berechnet
	}
	
	if (CheckPingTimeout)
	{
		// DEBUG_PRINT(F("Check Timeout: "));
		// DEBUG_PRINTLN(TimeoutTime);
		if (TimeoutTime > (uint32_t)MAX_ECHO_WAIT)
		{	
			// DEBUG_PRINT(F("Timeout G: "));
			// DEBUG_PRINT(TxGoodCounter);
			// DEBUG_PRINT(F(" Timeout B: "));
			// DEBUG_PRINTLN(TxFailCounter);
			
			send(msgDebugReturnString.set(F("Timeout")));
			TxFailCounter++;
			CheckPingTimeout=false;
		}
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
	// DEBUG_PRINT("sensor: ");
	// DEBUG_PRINTLN(message.sensor);

	// DEBUG_PRINT("mGetPayloadType: ");
	// DEBUG_PRINTLN(mGetPayloadType(message));
	
	// DEBUG_PRINT("sender: ");
	// DEBUG_PRINTLN(message.sender);

	if ((message.sensor == CHILD_ECHO_TIMESTAMP) && (message.sender == 0) && !message.isEcho())
	{
		if (message.type == V_TEXT)
		{
			EchoReturn = message.getULong();

			if (EchoReturn == PingTimeStamp)
			{
				GotEchoResponse=true;
			}			
		}
	}
		
}


