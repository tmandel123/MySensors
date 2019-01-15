//ToDo: 2 Knöpfe (später für An und Aus?)
// LED blickt, wenn ACK vom Gateway kommt, sonst wird erneut gesendet. nach 3 Sendeversuchen blinkt die LED 3 Mal


// Enable debug prints to serial monitor
#define MY_DEBUG
#define SER_DEBUG

// #define MY_RF24_CHANNEL 96									// Für Testphase deaktivieren, damit Kanal 76 aktiv wird (Prod=96 Test=76)
#define MY_RADIO_RF24
//#define MY_SMART_SLEEP_WAIT_DURATION (1000ul)
#define MY_NODE_ID 130
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC
// #define MY_SPLASH_SCREEN_DISABLED
#define MY_TRANSPORT_WAIT_READY_MS (5000ul)


#include <MySensors.h>
#include <VoltageReference.h>	// https://github.com/rlogiacco/VoltageReference Version 1.2.2


#define BATTERY_SENSE_PIN A0

// Node and sketch information
#define SKETCH_VER            			"1.0-001"        			// Sketch version
#define SKETCH_NAME           			"BatteryRemoteButton"   		// Optional child sensor name
#define LED_PIN 						8						// Arduino pin attached to MOSFET Gate pin
 

#define CHILD_ID_BAT_ANLG       		0							//ID für Batterie Werte an A0 --> Wird angewendet, wenn 3,3V StepUp Regler am Arduino angeschlossen ist. 
																	//Batterie ohne Spannungsteiler direkt an A0 anschließen und analogReference(DEFAULT);(VREF bringt hier immer den gleichen Wert)
#define CHILD_ID_BAT_VREF       		1         					//ID für Batterie Werte Intern per VRef --> Bei Betrieb mit 2-3 Batterien an Arduino mit abgelöteten Spannungsregler

#define PRIMARY_CHILD_ID 				3
#define SECONDARY_CHILD_ID 				4

#define PRIMARY_BUTTON_PIN 2   // Arduino Digital I/O pin for button/reed switch
#define SECONDARY_BUTTON_PIN 3 // Arduino Digital I/O pin for button/reed switch



#define	BAT_VREF_MAX_VOLTATE			3000
// #define	BAT_VREF_MAX_VOLTATE			5000
#define	BAT_VREF_MIN_VOLTATE			1800
// #define	BAT_VREF_MIN_VOLTATE			2600

#define BAT_MESSURED					2980
#define BAT_VREF_CORRECTION_VALUE		(float)BAT_VREF_MAX_VOLTATE/(float)BAT_MESSURED


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

MyMessage BatAnalogValue		(CHILD_ID_BAT_ANLG,		V_VOLTAGE);
MyMessage BatvRefValue			(CHILD_ID_BAT_VREF,		V_VOLTAGE);
MyMessage msg					(PRIMARY_CHILD_ID, 		V_TRIPPED);
MyMessage msg2					(SECONDARY_CHILD_ID, 	V_TRIPPED);


uint16_t batteryBatAnalogValue = 0;
const uint16_t BAT_ANALOG_MIN=600;
const uint16_t BAT_ANALOG_MAX=1023;
static uint8_t counter=0;

void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit: ");

	pinMode(LED_PIN, OUTPUT);   // sets the pin as output
	digitalWrite( LED_PIN, HIGH );
	LED_Blink(3,1);//Anzahl, Geschwindigkeit
}

void before()
{
	DEBUG_PRINTLN("before....");
	DEBUG_PRINT("Version: ");
	DEBUG_PRINTLN(SKETCH_VER);
	analogReference(INTERNAL);

	wait(100);//Warten bis sich alles eingeschwungen hat
	batteryBatAnalogValue = analogRead(BATTERY_SENSE_PIN);
	wait(100);
	batteryBatAnalogValue = analogRead(BATTERY_SENSE_PIN);
	

	vRef.begin();
}




void setup()
{
	// Setup the buttons
	pinMode(PRIMARY_BUTTON_PIN, INPUT_PULLUP);
	pinMode(SECONDARY_BUTTON_PIN, INPUT_PULLUP);
}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);
	present(PRIMARY_CHILD_ID, S_DOOR);
	present(SECONDARY_CHILD_ID, S_DOOR);
}

// Loop will iterate on changes on the BUTTON_PINs
void loop()
{
	uint8_t value;
	// static uint8_t sentValue=2;
	static uint8_t sentValue2=2;
	// static uint8_t counter=0;

	// Short delay to allow buttons to properly settle
	// sleep(5);

	value = digitalRead(PRIMARY_BUTTON_PIN);

	// aktuell ist nur ein Button angeschlossen
	// if (value != sentValue) {
		// Value has changed from last transmission, send the updated value
		// send(msg.set(value==HIGH));
		// sentValue = value;
		// counter++;
		// DEBUG_PRINT("counter: ");
		// DEBUG_PRINTLN(counter);
		// LED_Blink(1,1);//Anzahl, Geschwindigkeit
	// }

	value = digitalRead(SECONDARY_BUTTON_PIN);

	if (value != sentValue2) {
		// Value has changed from last transmission, send the updated value
		send(msg2.set(value==HIGH));
		sentValue2 = value;
		counter++;
		DEBUG_PRINT("counter: ");
		DEBUG_PRINTLN(counter);
		LED_Blink(2,1);//Anzahl, Geschwindigkeit
	}
	
	if (counter > 10)
	{
		counter = 0;
		BatteryVRef();
	}
	
	// DEBUG_PRINT("counter: ");
	// DEBUG_PRINTLN(counter);
	
	// Sleep until something happens with the sensor
	sleep(PRIMARY_BUTTON_PIN-2, CHANGE, SECONDARY_BUTTON_PIN-2, CHANGE, 0);
}


void BatteryVRef()
{
	uint16_t batteryPcntVcc = 0;
	float batVoltage;
	uint16_t vcc = vRef.readVcc(); //5000 oder 3000 mA
	float vccCorrect = (vcc * float(BAT_VREF_CORRECTION_VALUE));
	
	DEBUG_PRINT("vcc: ");
	DEBUG_PRINTLN(vcc);

	vccCorrect=constrain(vccCorrect, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE);
	batteryPcntVcc = map(vccCorrect, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE, 0, 100); 


	batVoltage  = vccCorrect / 1000;

	sendBatteryLevel(batteryPcntVcc);
	send(BatvRefValue.set(batVoltage,3));
	

	
	uint8_t batteryPcnt = 0;
	float batteryVoltage = 0;
	// get the battery Voltage
	batteryBatAnalogValue = analogRead(BATTERY_SENSE_PIN);
	batteryBatAnalogValue=constrain(batteryBatAnalogValue, BAT_ANALOG_MIN, BAT_ANALOG_MAX);
	batteryPcnt = map(batteryBatAnalogValue, BAT_ANALOG_MIN, BAT_ANALOG_MAX, 0, 100); //zwischen 100% und 60% bewegt sich die Nutzungsmöglichkeit der Batterie. Darunter bricht die Spannung zusammen
	batteryVoltage  = batteryBatAnalogValue * 0.004594330400782;
	sendBatteryLevel(batteryPcnt);
	send(BatAnalogValue.set(batteryVoltage,3));
	// smartSleep(SLEEP_TIME);	
}

void BatteryAnalog()
{
	uint16_t vcc = vRef.readVcc(); //5000 oder 3000 mA

	// float correctionValue=BAT_VREF_CORRECTION_VALUE; //Soll / Ist = 1,034 //5000 oder 3000 mA

	
	float vccCorrect = (vcc * float(BAT_VREF_CORRECTION_VALUE));

	uint16_t batteryPcntVcc = 0;
	float batVoltage = 0;

	vccCorrect=constrain(vccCorrect, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE);
	batteryPcntVcc = map(vccCorrect, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE, 0, 100); 


	batVoltage  = vccCorrect / 1000;

	sendBatteryLevel(batteryPcntVcc);
	send(BatvRefValue.set(batVoltage,3));
	

	
	uint8_t batteryPcnt = 0;
	float batteryVoltage = 0;
	// get the battery Voltage
	batteryBatAnalogValue = analogRead(BATTERY_SENSE_PIN);
	batteryBatAnalogValue=constrain(batteryBatAnalogValue, BAT_ANALOG_MIN, BAT_ANALOG_MAX);
	batteryPcnt = map(batteryBatAnalogValue, BAT_ANALOG_MIN, BAT_ANALOG_MAX, 0, 100); //zwischen 100% und 60% bewegt sich die Nutzungsmöglichkeit der Batterie. Darunter bricht die Spannung zusammen
	batteryVoltage  = batteryBatAnalogValue * 0.004594330400782;
	sendBatteryLevel(batteryPcnt);
	send(BatAnalogValue.set(batteryVoltage,3));
	// smartSleep(SLEEP_TIME);	
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
		BlinkTime=100;
		OffTime=50;
	}


	for (int i=0; i<Anzahl; i++){
		digitalWrite( LED_PIN, HIGH );
		wait(BlinkTime);
		digitalWrite( LED_PIN, LOW );
		wait(OffTime);
	}
}
