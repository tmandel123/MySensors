//ToDo: 2 Knöpfe (später für An und Aus?)
// LED blickt, wenn ACK vom Gateway kommt, sonst wird erneut gesendet. nach 3 Sendeversuchen blinkt die LED 3 Mal

// Enable debug prints to serial monitor
// #define MY_DEBUG
#define SER_DEBUG

#define MY_RF24_CHANNEL 96									// Für Testphase deaktivieren, damit Kanal 76 aktiv wird (Prod=96 Test=76)
#define MY_RADIO_RF24
//#define MY_SMART_SLEEP_WAIT_DURATION (1000ul)
#define MY_NODE_ID 130
// #define MY_PARENT_NODE_ID 50
// #define MY_PARENT_NODE_IS_STATIC
#define MY_SPLASH_SCREEN_DISABLED
#define MY_TRANSPORT_WAIT_READY_MS (5000ul)

#define MY_WITH_LEDS_BLINKING_INVERSE
#define MY_DEFAULT_TX_LED_PIN (8)
#define MY_DEFAULT_LED_BLINK_PERIOD 10

// #define MY_PASSIVE_NODE

#include <MySensors.h>
#include <VoltageReference.h>			// https://github.com/rlogiacco/VoltageReference Version 1.2.2
#include <OneButton.h>					// http://www.mathertel.de/Arduino/OneButtonLibrary.aspx Version 1.3

// #define BATTERY_SENSE_PIN A0

// Node and sketch information
#define SKETCH_VER            			"1.0-005"        			// Sketch version
#define SKETCH_NAME           			"BatteryRemoteButton"   		// Optional child sensor name
#define LED_PIN 						8							// Arduino pin attached to MOSFET Gate pin
#define RECEIVER_NODE					224
 

#define CHILD_ID_BAT_ANLG       		0							//ID für Batterie Werte an A0 --> Wird angewendet, wenn 3,3V StepUp Regler am Arduino angeschlossen ist. 
																	//Batterie ohne Spannungsteiler direkt an A0 anschließen und analogReference(DEFAULT);(VREF bringt hier immer den gleichen Wert)
#define CHILD_ID_BAT_VREF       		1         					//ID für Batterie Werte Intern per VRef --> Bei Betrieb mit 2-3 Batterien an Arduino mit abgelöteten Spannungsregler

#define BUTTON_CHILD	 				2

#define SEND_WAIT						40							// 20ms reicht nicht aus, dann kommt immer NACK, bei 22ms läuft es (mit Repeater sollten es 50ms sein)							
#define REQUEST_ACK						true


#define PRIMARY_BUTTON_PIN 				2
#define SECONDARY_BUTTON_PIN 			3
#define DEBOUNCE_TICKS					15
#define CLICK_TICKS						250

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

OneButton button1(PRIMARY_BUTTON_PIN,1,true); //Pin, 1=button connected to Ground, true = INPUT_PULLUP
OneButton button2(SECONDARY_BUTTON_PIN,1,true);

MyMessage BatvRefValue			(CHILD_ID_BAT_VREF,		V_VOLTAGE);
MyMessage ButtonMsg				(BUTTON_CHILD, 			V_VAR1);
// MyMessage ButtonVarMsg			(BUTTON_CHILD, 			V_VAR2);

// bool TransportUplink = true;
// static uint8_t counter=0;
int8_t wakeupReason = 0;
uint32_t currentTime;
uint32_t LastButtonUseTime;

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
	button1.attachDoubleClick(doubleclick1);
	button1.attachClick(click1);
	button1.attachLongPressStart(longPressStart1);
	// button1.attachLongPressStop(longPressStop1);
	// button1.attachDuringLongPress(longPress1);
	button1.setDebounceTicks(DEBOUNCE_TICKS);
	button1.setClickTicks(CLICK_TICKS);
	
	button2.attachDoubleClick(doubleclick2);
	button2.attachClick(click2);
	button2.attachLongPressStart(longPressStart2);
	// button2.attachLongPressStop(longPressStop2);
	// button2.attachDuringLongPress(longPress2);
	button2.setDebounceTicks(DEBOUNCE_TICKS);
	button2.setClickTicks(CLICK_TICKS);	
	
	
}



void presentation()
{
	DEBUG_PRINTLN("presentation...");
	char CompileDate[8];
	getCompileDateTime(__DATE__, CompileDate);
	DEBUG_PRINTLN(CompileDate);
	
	char SendString[25] = ""; //mehr als 25 Zeichen werden nicht übertragen
	uint8_t i=0;
	for (uint8_t j=0;j<(sizeof(SKETCH_VER)-1);j++)
	{
		SendString[j]=SKETCH_VER[j];
		i++;
	}
	SendString[i]=' ';
	i++;
	for (uint8_t j=0;j<(sizeof(CompileDate));j++)
	{
		SendString[i]=CompileDate[j];
		i++;
	}
	SendString[i]=' ';
	i++;

	for (uint8_t j=0;j<5;j++)
	{
		SendString[i]=__TIME__[j];
		i++;
	}

	sendSketchInfo(SKETCH_NAME, SendString );
	present(BUTTON_CHILD, S_CUSTOM);
	present(CHILD_ID_BAT_VREF, S_MULTIMETER);
}

// Loop will iterate on changes on the BUTTON_PINs
void loop()
{
	static int counter=0;
	currentTime = millis();
	
	button1.tick();
	button2.tick();
	
	if ((currentTime - LastButtonUseTime) > 3000)
	{
		DEBUG_PRINTLN("TimeToSleep");	
		wakeupReason = sleep(PRIMARY_BUTTON_PIN-2, CHANGE, SECONDARY_BUTTON_PIN-2, CHANGE, 0);
		
		button1.tick();
		button2.tick();
		
		currentTime = millis();
		LastButtonUseTime = currentTime;
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

void mySend(const char *myString)
{
	int myCounter=0;
	bool sendStatus=false;
	DEBUG_PRINT("mySend: ");
	DEBUG_PRINTLN(myString);
	// ButtonMsg.setDestination(RECEIVER_NODE);
	while ( !sendStatus and (myCounter < 5))
	{
		// ButtonMsg.setDestination(RECEIVER_NODE);
		sendStatus = send(ButtonMsg.set(myString), REQUEST_ACK);
		DEBUG_PRINT("sendStatus >");
		DEBUG_PRINT(sendStatus);
		DEBUG_PRINT("< myCounter >");
		DEBUG_PRINT(myCounter);
		DEBUG_PRINTLN("<");
		myCounter++;
		wait(100*myCounter*2);
	}
	LastButtonUseTime=currentTime;
}

void click1()
{
	const char *myString = "d";
	mySend(myString);
}

void click2()
{
	const char *myString = "u";
	mySend(myString);
}

void longPressStart1()
{
	const char *myString = "R";
	mySend(myString);
	// mySend(String("R"));
}

void longPressStart2()
{
	const char *myString = "R";
	mySend(myString);
	// mySend(String("R"));
}


void doubleclick1()
{
	const char *myString = "D";
	mySend(myString);
	// mySend(String("D"));
}


void doubleclick2()
{
	const char *myString = "U";
	mySend(myString);	
	
	// mySend(String("U"));
}


// void longPressStop1()
// {
	// DEBUG_PRINTLN("longPressStop1");
	// LastButtonUseTime=currentTime;
// }

// void longPress1()
// {
	// DEBUG_PRINTLN("longPress1");
	// LastButtonUseTime=currentTime;
// }

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


// void LED_Blink(int Anzahl, int Dauer)
// {

	// int OffTime;
	// int BlinkTime;
	// if (Dauer == 1)
	// {
		// BlinkTime=20;
		// OffTime=10;
	// }
	// else
	// {
		// BlinkTime=200;
		// OffTime=200;
	// }


	// for (int i=0; i<Anzahl; i++){
		// digitalWrite( LED_PIN, HIGH );
		// wait(BlinkTime);
		// digitalWrite( LED_PIN, LOW );
		// wait(OffTime);
	// }
// }

void getCompileDateTime(char const *date, char *buff) {
    int month, day, year;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", buff, &day, &year);
    month = (strstr(month_names, buff)-month_names)/3+1;
    sprintf(buff, "%d%02d%02d", year, month, day);
}