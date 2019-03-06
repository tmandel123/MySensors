/**
Repository:		https://github.com/tmandel123/MySensors

ToDo in FHEM

attr MYSENSOR_110 setCommands on:status_Switch:on off:status_Switch:off

ToDo in OneButton.h (to remove segmentation fault error in Arduino 1.8.8)

	Change to: 	OneButton(int pin, int activeLow, bool pullupActive = true);
	from:		OneButton(int pin, int active, bool pullupActive = true);
	
	
smartSleep -> 	testen, was nach Aufwachen passiert. dafür sorgen, dass smartsleep nur ein mal ausgeführt wird. wird nach dem aufwachen etwas vom gateway empfangen?
				später vor dem einschlafen prüfen, ob neue werte per receive empfangen wurden und erst dann einschlafen.
				

disable/cancel smartSleep while sleep countdown is running

I am developing a LED switch node that should sleep as much as possible while the LED is off.
As a controller I use FHEM which supports the smartSleep feature.
When the node wakes up the controller sends new commands to the node (turn LED on). The node turns on the LED, but directly turns it off and goes to sleep mode again.
Afer sleeptime the node wakes up, turns on the LED and stays on.
I think the node has initiated to go to sleep while receiving the turn-on-command.
Is it possible to turn smartSleep off (or cancel while the smartsleep countdown is running) in the receive method.
I counld not find any method to do that. I tried sleep_disable() but this seems not to work.

I use Mysensors 2.3.1 on an arduino nano.


wakeup with smartsleep=false

24729 MCO:SLP:WUP=-1
24731 TSF:TRI:TSB
wakeup reason: -1
going to sleep
24737 MCO:SLP:MS=13333,SMS=0,I1=1,M1=2,I2=255,M2=255
24744 TSF:TDI:TSL

wakeup with smartsleep=true

4617 MCO:SLP:WUP=-1
4619 TSF:TRI:TSB
4627 TSF:MSG:SEND,110-110-0-0,s=255,c=3,t=33,pt=5,l=4,sg=0,ft=0,st=OK:13333
wakeup reason: -1
going to sleep
4634 MCO:SLP:MS=13333,SMS=1,I1=1,M1=2,I2=255,M2=255
4643 TSF:MSG:SEND,110-110-0-0,s=255,c=3,t=32,pt=5,l=4,sg=0,ft=0,st=OK:555
5206 TSF:TDI:TSL

wakeup with smartsleep=true and receiving new value

7737 MCO:SLP:WUP=-1
7739 TSF:TRI:TSB
7747 TSF:MSG:SEND,110-110-0-0,s=255,c=3,t=33,pt=5,l=4,sg=0,ft=0,st=OK:12292
wakeup reason: -1
going to sleep
7754 MCO:SLP:MS=13333,SMS=1,I1=1,M1=2,I2=255,M2=255
7763 TSF:MSG:SEND,110-110-0-0,s=255,c=3,t=32,pt=5,l=4,sg=0,ft=0,st=OK:555
7800 TSF:MSG:READ,0-0-110,s=51,c=1,t=2,pt=0,l=1,sg=0:1
receive: 1
8326 TSF:TDI:TSL

wakeup and process new value

8327 MCO:SLP:WUP=-1
8329 TSF:TRI:TSB
8337 TSF:MSG:SEND,110-110-0-0,s=255,c=3,t=33,pt=5,l=4,sg=0,ft=0,st=OK:13333
wakeup reason: -1
8346 TSF:MSG:SEND,110-110-0-0,s=51,c=1,t=2,pt=1,l=1,sg=0,ft=0,st=OK:1


**/


//	###################   Debugging   #####################
#define MY_DEBUG
#define SER_DEBUG
// #define MY_DEBUG_VERBOSE_RF24
// #define MY_SPLASH_SCREEN_DISABLED
// #define MY_SIGNAL_REPORT_ENABLED

//	###################   Features   #####################
// #define MY_REPEATER_FEATURE
// #define MY_GATEWAY_SERIAL
// #define MY_INCLUSION_MODE_FEATURE
// #define MY_INCLUSION_BUTTON_FEATURE
// #define MY_INCLUSION_MODE_BUTTON_PIN 3

//	###################   LEDs   #####################
// #define MY_WITH_LEDS_BLINKING_INVERSE
// #define MY_DEFAULT_RX_LED_PIN				17
// #define MY_DEFAULT_TX_LED_PIN 				18
// #define MY_DEFAULT_ERR_LED_PIN				19
// #define MY_DEFAULT_LED_BLINK_PERIOD 		10

// ###################   Transport   #####################
/*
RF24_PA_MIN = 	-18dBm 		0	R_TX_Powerlevel_Pct 	25
RF24_PA_LOW = 	-12dBm 		1	R_TX_Powerlevel_Pct
RF24_PA_HIGH = 	-6dBm 		2	R_TX_Powerlevel_Pct
RF24_PA_MAX = 	 0dBm		3	R_TX_Powerlevel_Pct
*/

#define MY_RF24_PA_LEVEL 					RF24_PA_LOW
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
#define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)
#define MY_TRANSPORT_SANITY_CHECK

#define MY_NODE_ID 							110
// #define MY_PARENT_NODE_ID 					100		//without this passive node broadcasts everything to parent 255 (dont know what happens, if 2 repeater receive this at the same time)
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.0-002"        			// Sketch version
#define SKETCH_NAME           				"LEDSwitch"   		// Optional child sensor name

#define SLEEP_TIME							13333
#define MY_SMART_SLEEP_WAIT_DURATION_MS		(555ul)
#define	TOGGLE_BUTTON						3

#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen


#define MAX_LED_LEVEL						1
#define LED_LEVEL_EEPROM					0
#define	BLINK_DELAY							180

bool justReceived = false;
uint8_t	lastLevel = 0;
uint32_t lastIRQCounter = 999;				//for first loop it must be different from IRQCounter
volatile uint8_t currentLevel = 0; 			// Current LED level 0 or 1
volatile uint32_t lastPulseTime = 0;
volatile uint32_t IRQCounter = 0;



void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	pinMode(LED_PWM_PIN, OUTPUT);   // sets the pin as output
	pinMode(TOGGLE_BUTTON, INPUT_PULLUP);   // sets the pin as output
	Blink();
	DEBUG_PRINTLN("preHwInit done");

}

void before() 
{
	DEBUG_PRINTLN("before");
	Blink();
	
	currentLevel = loadState(LED_LEVEL_EEPROM);
	
	if (currentLevel>MAX_LED_LEVEL)
	{
		currentLevel=0;
		DEBUG_PRINTLN(F("Save: LED_LEVEL_EEPROM to 0"));
		saveState(LED_LEVEL_EEPROM, currentLevel);//8 Bit
	}
	DEBUG_PRINT("EEPROMLevel: ");
	DEBUG_PRINTLN(currentLevel);
	DEBUG_PRINTLN(F("restore last LED level"));
	digitalWrite( LED_PWM_PIN, currentLevel );
}


void setup()
{
	DEBUG_PRINTLN("Setup");
	Blink();
	send( msgSwitchState.set(currentLevel) );
	attachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON), ToggleWhileButtonPressed, FALLING );
}

void presentation()
{
	mySendSketchInfo();
	myPresentation();
	present(CHILD_SINGLE_LED_SWITCH, 	S_BINARY, 		CHILD_SINGLE_LED_SWITCH_TEXT);
	myHeartBeatLoop();
}


void loop()
{
	if (justReceived)//
	{
		DEBUG_PRINTLN("justReceived activate Interrupt");
		attachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON), ToggleWhileButtonPressed, FALLING );
	}
	
	if (lastIRQCounter != IRQCounter) 
	{
		DEBUG_PRINT("IRQCounter: ");
		DEBUG_PRINTLN(IRQCounter);
		lastIRQCounter = IRQCounter;
	}
	
	if (lastLevel != currentLevel) 
	{
		lastLevel = currentLevel;
		setLED(currentLevel);
		send( msgSwitchState.set(currentLevel) );
	}
	
	if (currentLevel == 0)
	{
		detachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON));
		DEBUG_PRINTLN("going to sleep");
		int8_t wakeupReason = sleep(digitalPinToInterrupt(TOGGLE_BUTTON), FALLING , SLEEP_TIME, true);
		DEBUG_PRINT("wakeup reason: ");
		DEBUG_PRINTLN(wakeupReason);
		if (wakeupReason == digitalPinToInterrupt(TOGGLE_BUTTON))
		{
			currentLevel = 1;
			attachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON), ToggleWhileButtonPressed, FALLING );
			DEBUG_PRINTLN("LED on after wake by ButtonIRQ");
		}

	}
}



void receive(const MyMessage &message)
{
	DEBUG_PRINT( "receive: " );
	DEBUG_PRINTLN( message.getByte() );	
	if (message.type == V_STATUS) 
	{
		uint8_t requestedLevel = message.getByte();
		if (requestedLevel == 0)
		{
			currentLevel=0;
		}
		else
		{
			currentLevel=1;
		}
	}
}

void setLED(uint8_t newLevel)
{
	if (newLevel == 0)
	{
		analogWrite(LED_PWM_PIN,0);
	}
	else
	{
		analogWrite(LED_PWM_PIN,newLevel<<3);
	}
}



void Blink()
{
	DEBUG_PRINTLN(F("digitalWrite Blink"));
	analogWrite( LED_PWM_PIN, 8 );
	wait(BLINK_DELAY); 
	analogWrite( LED_PWM_PIN, 0 );
	wait(BLINK_DELAY);
	analogWrite( LED_PWM_PIN, currentLevel << 3 );
}

void ToggleWhileButtonPressed()
{
	IRQCounter++;

	if (currentLevel == 0)
	{
		currentLevel=1;
	}
	else
	{
		currentLevel=0;
	}
	DEBUG_PRINT("ToggleWhileButtonPressed currentLevel: ");
	DEBUG_PRINTLN(currentLevel);
}
