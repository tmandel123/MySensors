//ToDo: 2 Knöpfe (später für An und Aus?)
// LED blickt, wenn ACK vom Gateway kommt, sonst wird erneut gesendet. nach 3 Sendeversuchen blinkt die LED 3 Mal


// Enable debug prints to serial monitor
// #define MY_DEBUG
#define SER_DEBUG

#define MY_RF24_CHANNEL 96									// Für Testphase deaktivieren, damit Kanal 76 aktiv wird (Prod=96 Test=76)
#define MY_RADIO_RF24
//#define MY_SMART_SLEEP_WAIT_DURATION (1000ul)
#define MY_NODE_ID 130
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC
// #define MY_SPLASH_SCREEN_DISABLED
#define MY_TRANSPORT_WAIT_READY_MS (0ul)
// #define MY_TRANSPORT_WAIT_READY_MS (5000ul)


#include <MySensors.h>
#include <VoltageReference.h>	// https://github.com/rlogiacco/VoltageReference Version 1.2.2


// #define BATTERY_SENSE_PIN A0

// Node and sketch information
#define SKETCH_VER            			"1.0-001"        			// Sketch version
#define SKETCH_NAME           			"BatteryRemoteButton"   		// Optional child sensor name
#define LED_PIN 						8							// Arduino pin attached to MOSFET Gate pin
#define RECEIVER_NODE					224
 

#define CHILD_ID_BAT_ANLG       		0							//ID für Batterie Werte an A0 --> Wird angewendet, wenn 3,3V StepUp Regler am Arduino angeschlossen ist. 
																	//Batterie ohne Spannungsteiler direkt an A0 anschließen und analogReference(DEFAULT);(VREF bringt hier immer den gleichen Wert)
#define CHILD_ID_BAT_VREF       		1         					//ID für Batterie Werte Intern per VRef --> Bei Betrieb mit 2-3 Batterien an Arduino mit abgelöteten Spannungsregler

#define PRIMARY_CHILD_ID 				3
#define SECONDARY_CHILD_ID 				4

#define SEND_WAIT						40							// 20ms reicht nicht aus, dann kommt immer NACK, bei 22ms läuft es (mit Repeater sollten es 50ms sein)							
#define REQUEST_ACK						true


#define PRIMARY_BUTTON_PIN 				2  							// Arduino Digital I/O pin for button/reed switch
#define SECONDARY_BUTTON_PIN 			3							// Arduino Digital I/O pin for button/reed switch

#define	BAT_VREF_MAX_VOLTATE			3150
#define	BAT_VREF_MIN_VOLTATE			1800
// #define BAT_MESSURED					3510
// #define BAT_VREF_CORRECTION_VALUE		(float)BAT_VREF_MAX_VOLTATE/(float)BAT_MESSURED
// #define BAT_VREF_CORRECTION_VALUE		1


#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif

VoltageReference vRef;


MyMessage BatvRefValue			(CHILD_ID_BAT_VREF,		V_VOLTAGE);
MyMessage msg					(PRIMARY_CHILD_ID, 		V_TRIPPED);
MyMessage msg2					(SECONDARY_CHILD_ID, 	V_TRIPPED);

bool TransportUplink = true;
static uint8_t counter=0;

void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit...");

	pinMode(LED_PIN, OUTPUT);   // sets the pin as output
	// digitalWrite( LED_PIN, HIGH );
	LED_Blink(1,1);//Anzahl, Geschwindigkeit

}

void before()
{
	DEBUG_PRINTLN("before...");
	DEBUG_PRINT("Version: ");
	DEBUG_PRINTLN(SKETCH_VER);

	vRef.begin();
	LED_Blink(1,2);//Anzahl, Geschwindigkeit
	
}




void setup()
{
	// Setup the buttons
	pinMode(PRIMARY_BUTTON_PIN, INPUT_PULLUP);
	pinMode(SECONDARY_BUTTON_PIN, INPUT_PULLUP);
}

void presentation()
{
	DEBUG_PRINTLN("presentation...");
	// Send the sketch version information to the gateway and Controller
	// sendSketchInfo(SKETCH_NAME, SKETCH_VER);
	sendSketchInfo(SKETCH_NAME, SKETCH_VER " " __TIME__ " " __DATE__);
	present(PRIMARY_CHILD_ID, S_DOOR);
	present(SECONDARY_CHILD_ID, S_DOOR);
	BatteryVRef();
	LED_Blink(1,1);//Anzahl, Geschwindigkeit
}

// Loop will iterate on changes on the BUTTON_PINs
void loop()
{
	DEBUG_PRINTLN("loop...");
	uint8_t value;
	// static uint8_t sentValue=2;
	static uint8_t sentValue2=2;
	// static uint8_t counter=0;

	// Short delay to allow buttons to properly settle
	sleep(5);

	value = digitalRead(SECONDARY_BUTTON_PIN);

	if (value != sentValue2) {
		// Value has changed from last transmission, send the updated value
		DEBUG_PRINT(F("Button"));
		DEBUG_PRINTLN(value);
		// send(SisternodeMsg.setDestination(MY_SISTER_NODE_ID).setSensor(CHILD_ID_SISTER_TEMP).set(temperature, 1));
		// https://forum.mysensors.org/topic/8941/mysensors-get-temperature-value-from-another-node-through-the-gateway/16
		msg2.setDestination(RECEIVER_NODE);
		TransportUplink = send(msg2.set(value), REQUEST_ACK);
		if (TransportUplink == false)
		{
			DEBUG_PRINTLN(F("No Ack"));
			LED_Blink(5,1);//Anzahl, Geschwindigkeit();
		}
		else
		{
			LED_Blink(1,1);
		}
		sentValue2 = value;
		counter++;
	}
	
	if (counter >= 10)
	{
		counter = 0;
		BatteryVRef();
	}
	
	
	// Sleep until something happens with the sensor
	sleep(PRIMARY_BUTTON_PIN-2, CHANGE, SECONDARY_BUTTON_PIN-2, CHANGE, 0);
}


void BatteryVRef()
{
	uint8_t batteryPcntVcc;
	float batVoltage;
	uint16_t vcc = vRef.readVcc(); //5000 oder 3000 mA
	// uint16_t vccCorrect = (vcc * float(BAT_VREF_CORRECTION_VALUE));
	
	DEBUG_PRINT("vcc vor constrain: ");
	DEBUG_PRINTLN(vcc);

	vcc=constrain(vcc, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE);
	batteryPcntVcc = map(vcc, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE, 0, 100); 

	DEBUG_PRINT("vcc: ");
	DEBUG_PRINTLN(vcc);
	
	batVoltage  = (float)vcc / 1000;

	DEBUG_PRINT("batVoltage: ");
	DEBUG_PRINTLN(batVoltage);
	
	sendBatteryLevel(batteryPcntVcc);
	send(BatvRefValue.set(batVoltage,3));
	
}


void LED_Blink(int Anzahl, int Dauer)
{

	int OffTime;
	int BlinkTime;
	if (Dauer == 1)
	{
		BlinkTime=20;
		OffTime=10;
	}
	else
	{
		BlinkTime=200;
		OffTime=200;
	}


	for (int i=0; i<Anzahl; i++){
		digitalWrite( LED_PIN, HIGH );
		wait(BlinkTime);
		digitalWrite( LED_PIN, LOW );
		wait(OffTime);
	}
}
