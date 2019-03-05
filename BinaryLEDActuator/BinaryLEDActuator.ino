/**
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




**/


//	###################   Debugging   #####################
#define MY_DEBUG
#define SER_DEBUG
// #define MY_DEBUG_VERBOSE_RF24								//Testen, welche zusätzlichen Infos angezeigt werden
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
// #define MY_PARENT_NODE_ID 					100		//without this the node broadcasts everything to parent 255 (dont know what happens, if 2 repeater receive this at the same time)
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.0-002"        			// Sketch version
#define SKETCH_NAME           				"LEDSwitch"   		// Optional child sensor name

#define HEARTBEAT_INTERVAL        			300000        //später alle 5 Minuten, zum Test alle 30 Sekunden
#define SLEEP_TIME							8888
#define MY_SMART_SLEEP_WAIT_DURATION_MS		(1234ul)
#define	TOGGLE_BUTTON						3
// #define	WITH_BATTERY

#include <MySensors.h>
// #include <OneButton.h>					// http://www.mathertel.de/Arduino/OneButtonLibrary.aspx Version 1.3
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen


// OneButton button1(TOGGLE_BUTTON,1,true); //Pin, 1=button connected to Ground, true = INPUT_PULLUP




#define MAX_LED_LEVEL						1
#define LED_LEVEL_EEPROM					0
#define	BLINK_DELAY							180


volatile uint8_t currentLevel = 0;  // Current dim level...
uint32_t lastHeartBeat = HEARTBEAT_INTERVAL - 10000; //das erste Mal sollte nach 5 Sekunden etwas passieren
uint32_t oldPulseCount = 0;
volatile uint32_t pulseCount = 0;
volatile uint32_t lastPulseTime = 0;


void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	pinMode(LED_DIGITAL_PIN, OUTPUT);   // sets the pin as output
	pinMode(TOGGLE_BUTTON, INPUT_PULLUP);   // sets the pin as output
	// Blink();
	DEBUG_PRINTLN("preHwInit done");

}

void before() 
{
	DEBUG_PRINTLN("before");
	// Blink();
	
	// showEEprom();
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
	digitalWrite( LED_DIGITAL_PIN, currentLevel );
	
}



void setup()
{
	DEBUG_PRINTLN("Setup");
	// Blink();
	send( msgSwitchState.set(currentLevel) );
	// button1.attachClick(click1);
	// button1.setDebounceTicks(DEBOUNCE_TICKS);	//defined in CommonFunctions.h
	// button1.setClickTicks(CLICK_TICKS);			//defined in CommonFunctions.h
	// attachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON), onPulse, FALLING );
}

void presentation()
{
	mySendSketchInfo();
	myPresentation();
	present(CHILD_SINGLE_LED_SWITCH, 	S_BINARY, 		CHILD_SINGLE_LED_SWITCH_TEXT);
	// present(CHILD_MULTI_BUTTON, 		S_INFO, 		CHILD_MULTI_BUTTON_TEXT);
	wait(100);
	myHeartBeatLoop();
}


void loop()
{
	uint32_t currentTime = millis();
	// button1.tick();
	if ((currentTime - lastHeartBeat > (uint32_t)HEARTBEAT_INTERVAL) and (currentLevel == 1))
	{
		DEBUG_PRINTLN("HEARTBEAT_INTERVAL");
		sendHeartbeat();
		myHeartBeatLoop();
		lastHeartBeat = currentTime;
		send( msgSwitchState.set(currentLevel > 0) );
	}
	

	
	if (currentLevel == 0)
	{
		// sendHeartbeat();
		// myHeartBeatLoop();
		
		detachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON));
		DEBUG_PRINTLN("going to sleep");
		int8_t wakeupReason = smartSleep(digitalPinToInterrupt(TOGGLE_BUTTON), FALLING , SLEEP_TIME);
		if (wakeupReason == digitalPinToInterrupt(TOGGLE_BUTTON))
		{
			pulseCount++;
			attachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON), onPulse, FALLING );
		}
		DEBUG_PRINT("wakeup reason: ");
		DEBUG_PRINTLN(wakeupReason);

	}

	if (pulseCount != oldPulseCount) 
	{
		oldPulseCount = pulseCount;
		toggleLED();
	}
	// printLevel();
}



void receive(const MyMessage &message)
{
	DEBUG_PRINT( "receive: " );
	// DEBUG_PRINTLN( String(message.data) );	
	DEBUG_PRINTLN( message.getByte() );	
	if (message.type == V_STATUS) 
	{
		// uint8_t requestedLevel = atoi( message.data );
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
		analogWrite(LED_DIGITAL_PIN,0);
	}
	else
	{
		analogWrite(LED_DIGITAL_PIN,newLevel<<3);
	}
}

void printLevel()
{
	DEBUG_PRINT(F("printLevel currentLevel: "));
	DEBUG_PRINTLN(currentLevel);
}

void toggleLED()
{
	if (currentLevel == 0)
	{
		currentLevel=1;
	}
	else
	{
		currentLevel=0;
	}
	DEBUG_PRINTLN(F("toggleLED and saveState"));
	// digitalWrite(LED_DIGITAL_PIN, currentLevel );
	setLED(currentLevel);
	// saveState(LED_LEVEL_EEPROM,	currentLevel);
	send(msgSwitchState.set(currentLevel));
}

void Blink()
{
	DEBUG_PRINTLN(F("digitalWrite Blink"));
	digitalWrite( LED_DIGITAL_PIN, 1 );
	wait(BLINK_DELAY); 
	digitalWrite( LED_DIGITAL_PIN, 0 );
	wait(BLINK_DELAY);
	digitalWrite( LED_DIGITAL_PIN, currentLevel );
}

void onPulse()
{
	uint32_t newPulseTime = millis();
	uint32_t interval = newPulseTime-lastPulseTime;
	DEBUG_PRINT("onPulse interval:");
	DEBUG_PRINTLN(interval);
	// if (interval<10000L) { // Sometimes we get interrupt on RISING
	if (interval<(uint32_t)50) { // Sometimes we get interrupt on RISING
		DEBUG_PRINTLN("return OnPulse");
		return;
	}

	lastPulseTime = newPulseTime;
	pulseCount++;
	DEBUG_PRINT("onPulse:");
	DEBUG_PRINTLN(pulseCount);
}

// void onPulse()
// {
	// uint8_t ButtonState = digitalRead(TOGGLE_BUTTON);
	// DEBUG_PRINT("onPulse ButtonState: ");
	// DEBUG_PRINTLN(ButtonState);
	// if (ButtonState == 0)
	// {
		// Blink();
		// if (currentLevel == 0)
		// {
			// currentLevel=1;
		// }
		// else
		// {
			// currentLevel=0;
		// }

		// digitalWrite(LED_DIGITAL_PIN, currentLevel );
		// saveState(LED_LEVEL_EEPROM,	currentLevel);
		// DEBUG_PRINTLN(F("button click and toggle"));
		// send(msgSwitchState.set(currentLevel > 0));
	// }
	// DEBUG_PRINTLN("onPulse Ende");
// }

