/**
Repository:		https://github.com/tmandel123/MySensors

ToDo in FHEM

attr MYSENSOR_110 setCommands on:status_Switch:on off:status_Switch:off
attr MYSENSOR_110 stateFormat status_Switch

ToDo in OneButton.h (to remove segmentation fault error in Arduino 1.8.8)

	Change to: 	OneButton(int pin, int activeLow, bool pullupActive = true);
	from:		OneButton(int pin, int active, bool pullupActive = true);
	
	
	smartSleep Patch in MySensorsCore.cpp Line 619 
		//Tmandel:
		// wait(MY_SMART_SLEEP_WAIT_DURATION_MS);		// listen for incoming messages
		if (wait(MY_SMART_SLEEP_WAIT_DURATION_MS, C_SET, I_VERSION)) //cancel sleeping request if there is a new C_SET Command
		{
			#define MY_SMART_SLEEP_REVOKE_WAIT_DURATION_MS (300ul) 	// 300 seems to be OK (smaller values let the controller think that the node is still sleeping)
			CORE_DEBUG(PSTR("!MCO:SLP:RVKE\n")); //sleep revoked
			wait(MY_SMART_SLEEP_REVOKE_WAIT_DURATION_MS);
			CORE_DEBUG(PSTR("MCO:SLP:WUP=%" PRIi8 "\n"), MY_SLEEP_NOT_POSSIBLE);	// inform controller about wake-up
			(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL,
							   I_POST_SLEEP_NOTIFICATION).set(MY_SMART_SLEEP_WAIT_DURATION_MS + MY_SMART_SLEEP_REVOKE_WAIT_DURATION_MS));
			return MY_SLEEP_NOT_POSSIBLE;
		}
				
**/


//	###################   Debugging   #####################
// #define MY_DEBUG
// #define SER_DEBUG
// #define MY_DEBUG_VERBOSE_RF24
#define MY_SPLASH_SCREEN_DISABLED
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
// #define MY_PARENT_NODE_ID 					100		//without this passive node broadcasts everything to parent 255 (dont know what happens if 2 repeater receive this at the same time)
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE
// #define MY_CORE_ONLY						// does not call preHwInit() and before(), IRQ seems not to work

// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.0-003"        			// Sketch version
#define SKETCH_NAME           				"LEDSwitch"   		// Optional child sensor name

#define SLEEP_TIME							30000
#define MY_SMART_SLEEP_WAIT_DURATION_MS		(1000ul)
#define	TOGGLE_BUTTON						3




#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen


#define MAX_LED_LEVEL						1
#define LED_LEVEL_EEPROM					0
#define	BLINK_DELAY							50
#define HEARTBEAT_INTERVAL					600000				//später alle 5 Minuten, zum Test alle 30 Sekunden

const int debounceTime = 15;  // debounce in milliseconds
bool justReceived = false;
bool IrqIsOn = false;
uint8_t	lastLevel = 0;
uint8_t	heartBeatCounter = 0;
// uint32_t lastIRQCounter = 999;				//for first loop it must be different from IRQCounter
uint32_t lastButtonPressed = 0;
uint32_t lastHeartBeat = 0;
volatile uint8_t currentLevel = 1; 			// Current LED level 0 or 1
volatile uint32_t lastPulseTime = 0;
// volatile uint32_t IRQCounter = 0;



void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	pinMode(LED_PWM_PIN, OUTPUT);   // sets the pin as output Active High, LED+ on Port 5 LED- on GND
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
	DEBUG_PRINT(F("restore last LED level: "));
	DEBUG_PRINTLN(currentLevel);
	digitalWrite( LED_PWM_PIN, currentLevel );
}


void setup()
{
	DEBUG_PRINTLN("Setup");
	Blink();
	send( msgSwitchState.set(currentLevel) );
	setIrqOn();
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
	uint32_t currentTime = millis();
	if (justReceived)//
	{
		DEBUG_PRINTLN("justReceived");
		setIrqOn();
		justReceived = false;
	}
	
	// if (lastIRQCounter != IRQCounter) 
	// {
		// DEBUG_PRINT("IRQCounter: ");
		// DEBUG_PRINTLN(IRQCounter);
		// lastIRQCounter = IRQCounter;
	// }
	
	if (lastLevel != currentLevel) 
	{
		deBounce();//wait for button to be released
		lastLevel = currentLevel;
		setLED(currentLevel);
	}
	
	if (currentLevel == 0)
	{
		DEBUG_PRINTLN("prepare to sleep");
		setIrqOff();
		int8_t wakeupReason = sleep(digitalPinToInterrupt(TOGGLE_BUTTON), FALLING , SLEEP_TIME, true);
		if (wakeupReason == digitalPinToInterrupt(TOGGLE_BUTTON))
		{
			currentLevel = 1;
			// IRQCounter++;
			deBounce();//wait for button to be released
			lastButtonPressed=millis();
			DEBUG_PRINTLN("LED on after wake by ButtonIRQ");
			setIrqOn();
		}
		heartBeatCounter++;
		DEBUG_PRINT("leaving sleep: heartBeatCounter ");
		DEBUG_PRINTLN(heartBeatCounter);
	}
	else
	{
		uint32_t TimeSinceHeartBeat = currentTime - lastHeartBeat;
		if (((TimeSinceHeartBeat > (uint32_t)HEARTBEAT_INTERVAL)))
		{
			lastHeartBeat = currentTime;
			sendHeartbeat();
		}
	}
	if (heartBeatCounter > 10)//to flush retained messaged
	{
		sendHeartbeat(); 
		heartBeatCounter = 0;
		send( msgSwitchState.set(currentLevel) );
	}
}



void receive(const MyMessage &message)
{
	DEBUG_PRINT( "receive: " );
	DEBUG_PRINTLN( message.getByte() );	
	if (message.type == V_STATUS) 
	{
		justReceived = true;
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

void setIrqOn()
{
	DEBUG_PRINTLN("setIrqOn");
	IrqIsOn = true;
	noInterrupts();
	clearPendingInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON)); //MySensors Funktion
	attachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON), ToggleWhileButtonPressed, FALLING );
	interrupts();
}

void setIrqOff()
{
	DEBUG_PRINTLN("setIrqOff");
	IrqIsOn = false;
	noInterrupts();
	detachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON));
	clearPendingInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON)); //MySensors Funktion	
	interrupts();
}

void setLED(uint8_t newLevel)
{
	DEBUG_PRINT( "setLED: " );
	DEBUG_PRINTLN( newLevel );	
	if (newLevel == 0)
	{
		digitalWrite(LED_PWM_PIN,0);
	}
	else
	{
		digitalWrite(LED_PWM_PIN,newLevel);
	}
	saveState(LED_LEVEL_EEPROM, newLevel);//8 Bit
	send( msgSwitchState.set(newLevel) );
}


void deBounce() 
{
	uint32_t now = millis ();
	do
	{
		// on bounce, reset time-out
		if (digitalRead (TOGGLE_BUTTON) == LOW)
		{
			now = millis();
		}
	}	
	while ((digitalRead (TOGGLE_BUTTON) == LOW) || ((millis() - now) <= debounceTime));

}

void Blink()
{
	DEBUG_PRINTLN(F("digitalWrite Blink"));
	digitalWrite( LED_PWM_PIN, 1 );
	wait(BLINK_DELAY); 
	digitalWrite( LED_PWM_PIN, 0 );
	wait(BLINK_DELAY*2);
	digitalWrite( LED_PWM_PIN, currentLevel );
}

void ToggleWhileButtonPressed()
{
	// IRQCounter++; //for debugging
	DEBUG_PRINT("TB");
	uint32_t now=millis();
	if ((now - lastButtonPressed) < 150)
	{
		DEBUG_PRINTLN(" JT");
		return;
	}
	DEBUG_PRINTLN(" OK");
	lastButtonPressed=now;

	if (currentLevel == 0)
	{
		currentLevel=1;
	}
	else
	{
		currentLevel=0;
	}

}
