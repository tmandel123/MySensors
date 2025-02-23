/** ToDo: 2 Knöpfe (später für An und Aus?)

// LED blickt, wenn ACK vom Gateway kommt, sonst wird erneut gesendet. nach 3 Sendeversuchen blinkt die LED 3 Mal

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
#define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)
#define MY_TRANSPORT_SANITY_CHECK

#define MY_NODE_ID 							130
#define MY_PARENT_NODE_ID 					50
#define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.0-006"        			// Sketch version
#define SKETCH_NAME           				"BatteryRemoteButton"   		// Optional child sensor name
#define NODE_TXT 							"Info"
#define CHILD_ID_TEXT						0


// #include <MySensors.h>
// #include <Servo.h>

			

// #define BATTERY_SENSE_PIN A0

// Node and sketch information

#define LED_PIN 							8							// Arduino pin attached to MOSFET Gate pin
#define RECEIVER_NODE						224
 

#define CHILD_ID_BAT_ANLG       			0							//ID für Batterie Werte an A0 --> Wird angewendet, wenn 3,3V StepUp Regler am Arduino angeschlossen ist. 
																		//Batterie ohne Spannungsteiler direkt an A0 anschließen und analogReference(DEFAULT);(VREF bringt hier immer den gleichen Wert)
#define CHILD_ID_BAT_VREF       			1         					//ID für Batterie Werte Intern per VRef --> Bei Betrieb mit 2-3 Batterien an Arduino mit abgelöteten Spannungsregler
	
#define BUTTON_CHILD	 					2

#define SEND_WAIT							40							// 20ms reicht nicht aus, dann kommt immer NACK, bei 22ms läuft es (mit Repeater sollten es 50ms sein)							
// #define REQUEST_ACK							true


#define PRIMARY_BUTTON_PIN 					2
#define SECONDARY_BUTTON_PIN 				3
#define DEBOUNCE_TICKS						15
#define CLICK_TICKS							250

// #define	BAT_VREF_MAX_VOLTATE				3150
// #define	BAT_VREF_MIN_VOLTATE				1800
// #define BAT_MESSURED						3510
// #define BAT_VREF_CORRECTION_VALUE		(float)BAT_VREF_MAX_VOLTATE/(float)BAT_MESSURED
// #define BAT_VREF_CORRECTION_VALUE		1


#include <MySensors.h>
// #include <VoltageReference.h>			// https://github.com/rlogiacco/VoltageReference Version 1.2.2
#include <OneButton.h>					// http://www.mathertel.de/Arduino/OneButtonLibrary.aspx Version 1.3
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen

// VoltageReference vRef;

OneButton button1(PRIMARY_BUTTON_PIN,1,true); //Pin, 1=button connected to Ground, true = INPUT_PULLUP
OneButton button2(SECONDARY_BUTTON_PIN,1,true);

// MyMessage BatvRefValue			(CHILD_ID_BAT_VREF,		V_VOLTAGE);
MyMessage ButtonMsg				(BUTTON_CHILD, 			V_VAR1);
// MyMessage ButtonVarMsg			(BUTTON_CHILD, 			V_VAR2);

// bool TransportUplink = true;
// static uint8_t counter=0;
int8_t wakeupReason = 0;
uint32_t LastButtonEventTime;

void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit...");

	// pinMode(LED_PIN, OUTPUT);   // sets the pin as output
	// digitalWrite( LED_PIN, HIGH );
	// LED_Blink(1,1);//Anzahl, Geschwindigkeit
}

void before()
{
	DEBUG_PRINTLN("before...");
	DEBUG_PRINT("Version: ");
	DEBUG_PRINTLN(SKETCH_VER);

	vRef.begin();
	// LED_Blink(1,2);//Anzahl, Geschwindigkeit
	
}

void setup()
{
	// button1.attachDoubleClick(doubleclick1);
	button1.attachClick(click1);
	// button1.attachLongPressStart(longPressReset);
	// button1.setDebounceTicks(DEBOUNCE_TICKS);
	// button1.setClickTicks(CLICK_TICKS);
	
	// button2.attachDoubleClick(doubleclick2);
	// button2.attachClick(click2);
	// button2.attachLongPressStart(longPressReset);
	// button2.setDebounceTicks(DEBOUNCE_TICKS);
	// button2.setClickTicks(CLICK_TICKS);	
}


void presentation()
{
	DEBUG_PRINTLN("presentation...");
	mySendSketchInfo();
	present(BUTTON_CHILD, S_CUSTOM);
	present(CHILD_ID_BAT_VREF, S_MULTIMETER);
}


void loop()
{
	static int counter=0;
	uint32_t currentTime = millis();
	
	button1.tick();
	button2.tick();
	
	if ((currentTime - LastButtonEventTime) > 3000)
	{
		DEBUG_PRINTLN("TimeToSleep");	
		wakeupReason = sleep(PRIMARY_BUTTON_PIN-2, CHANGE, SECONDARY_BUTTON_PIN-2, CHANGE, 0);
		
		button1.tick();
		button2.tick();
		
		currentTime = millis();

		LastButtonEventTime = currentTime;
		// DEBUG_PRINT("wakeupReason: "); // 0 oder 1 je nach Button
		// DEBUG_PRINTLN(wakeupReason);
		counter++;
	}

	if (counter >= 10)
	{
		counter = 0;
		BatteryVRef();
	}
}

void mySendString(MyMessage ThisMessage, const char *myString)//BatteryRemoteButton.ino
{
	int myCounter=0;
	bool sendStatus=false;
	DEBUG_PRINT("mySend: ");
	DEBUG_PRINTLN(myString);
	// ThisMessage.setDestination(RECEIVER_NODE);
	while ( !sendStatus and (myCounter < 5))
	{
		// ThisMessage.setDestination(RECEIVER_NODE);
		sendStatus = send(ThisMessage.set(myString), REQUEST_ACK);
		DEBUG_PRINT("sendStatus >");
		DEBUG_PRINT(sendStatus);
		DEBUG_PRINT("< myCounter >");
		DEBUG_PRINT(myCounter);
		DEBUG_PRINTLN("<");
		myCounter++;
		wait(100*myCounter*2);
	}
}

void click1()
{
	mySendString(ButtonMsg,"d");
	LastButtonEventTime=millis();
}

// void click2()
// {
	// const char *myString = "u";
	// mySend(myString);
	// LastButtonEventTime=currentTime;
// }

// void longPressReset()
// {
	// const char *myString = "R";
	// mySend(myString);
	// LastButtonEventTime=currentTime;
// }


// void doubleclick1()
// {
	// const char *myString = "D";
	// mySend(myString);
	// LastButtonEventTime=currentTime;
// }


// void doubleclick2()
// {
	// const char *myString = "U";
	// mySend(myString);	
	// LastButtonEventTime=currentTime;
// }

// void longPressStop1()
// void longPress1()



