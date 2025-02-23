/**

20221125 Version 1.0-009	Neuerungen anderer Nodes aus CommonFunctions.h übernommen





Repository:		https://github.com/tmandel123/MySensors

ToDo in FHEM

attr MYSENSOR_110 setCommands on:status_Switch:on off:status_Switch:off
attr MYSENSOR_110 stateFormat status_Switch

ToDo in OneButton.h (to remove segmentation fault error in Arduino 1.8.8)

	Change to: 	OneButton(int pin, int activeLow, bool pullupActive = true);
	from:		OneButton(int pin, int active, bool pullupActive = true);
	
	
	smartSleep Patch in MySensorsCore.cpp Line 619 ( MySensors 2.3.2 Line 685)
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


/*
ACHTUNG: Button mit Interrupt funktioniert nicht zuverlässig, wenn der Arduino mit 3,3V betrieben wird. (Gut möglich, dass sogar weniger als 3,3V beim Arduino ankommen. 
Das Netzteil von IKEA Stranne liefert ca. 5,8V was auch für einen 5V Arduino zu wenig ist, wenn vorher noch ein Spannungswandler kommt (der Spannungswandler benötigt ca. 1V mehr als die Ziel-Spannung)

Testen mit Arduino Nano und LEDPIN 13 statt 5
*/


//	###################   Debugging   #####################
// #define MY_DEBUG
// #define SER_DEBUG
#define MY_SPECIAL_DEBUG									// für Extended Debug in FHEM
// #define MY_DEBUG_VERBOSE_CORE
// #define MY_SPLASH_SCREEN_DISABLED
// #define MY_SIGNAL_REPORT_ENABLED



// ###################   Transport   #####################
/*
RF24_PA_MIN = 	-18dBm 		0	R_TX_Powerlevel_Pct 	25
RF24_PA_LOW = 	-12dBm 		1	R_TX_Powerlevel_Pct
RF24_PA_HIGH = 	-6dBm 		2	R_TX_Powerlevel_Pct
RF24_PA_MAX = 	 0dBm		3	R_TX_Powerlevel_Pct
*/

#define MY_RF24_PA_LEVEL 					RF24_PA_MAX
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
#define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)

#define MY_NODE_ID 							110
#define MY_PARENT_NODE_ID 					0		//without this passive node broadcasts everything to parent 255 (dont know what happens if 2 repeater receive this at the same time)
#define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE
// #define MY_CORE_ONLY						// does not call preHwInit() and before(), IRQ seems not to work


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.0-009"        			// Sketch version
#define SKETCH_NAME           				"LEDSwitch"   				// Optional child sensor name



#define MY_SMART_SLEEP_REVOKE_WAIT_DURATION_MS (700ul) 	// 300 seems to be OK (smaller values let the controller think that the node is still sleeping)
#define MY_SMART_SLEEP_WAIT_DURATION_MS		(1000ul)						//war 1000ul
#define SLEEP_TIME							30000
#define	TOGGLE_BUTTON						3




// ###################   Allgemeine MySensors Funktionen aus CommonFunctions.h (erhöht den Speicherverbrauch   #####################

// #define WITH_BUTTON
#define WITH_SLEEP

#define WITH_HWTIME
#define WITH_RF24_INFO						// übermittel RSSI, PA_Level, RF_Channel
#define WITH_NODE_INFO						// übermittel NodeID, ParentNodeID
#define MY_INDICATION_HANDLER				//erlaubt rewrite der Funktion void indication(indication_t ind) 
#define AUTO_REBOOT							// falls MY_INDICATION_HANDLER aktiviert wurde und txERR > 100, dann Node rebooten


#ifdef MY_REPEATER_FEATURE 
	#undef WITH_SLEEP 
#endif

#include <MySensors.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen


#define MAX_LED_LEVEL						1
#define LED_LEVEL_EEPROM					0					//	Speicherstelle im EEPROM
#define LED_LEVEL_START						1					//  Startwert nach Stromwiederkehr
#define	BLINK_DELAY							50
#define HEARTBEAT_INTERVAL					1800000				//	default: 1800000

const int 	debounceTime 		= 15;  // debounce in milliseconds
bool 		justReceived 		= false;
bool 		firstRun 			= true;
uint8_t		lastLevel 			= LED_LEVEL_START;
uint8_t		heartBeatCounter 	= 0;
uint32_t	lastButtonPressed 	= 0;
uint32_t 	lastHeartBeat		= 0;
volatile uint8_t currentLevel 	= LED_LEVEL_START; 			// Current LED level 0 or 1	// 8Bit, weil uint8_t so besser als bool ins EEPROM passt



void preHwInit() //kein serieller Output 
{
	#if defined(MY_DISABLED_SERIAL)
	  Serielles Interface nur setzten, falls nicht schon von MySensors Framework erledledigt
		#if defined SER_DEBUG
			DEBUG_SERIAL(MY_BAUD_RATE); // MY_BAUD_RATE from MyConfig.h 115200ul, gehört nach preHwInit. Falls in before(), friert der Arduino ein

		#else
			Serial.begin(MY_BAUD_RATE);

		#endif
	#endif
}


void before() 
{
	
	#if defined(MY_DISABLED_SERIAL)
	  Serielles Interface nur setzten, falls nicht schon von MySensors Framework erledledigt
		#if defined SER_DEBUG
			DEBUG_PRINTLN(F("NO MySensors Serial Interface. Starting own Interface for Debug"));
		#else
			Serial.println(F("NO MySensors Serial Interface. No Debug"));
		#endif
	#else
		Serial.println(F("MySensors already activated Serial Interface"));
	#endif
	
	DEBUG_PRINTLN(F("before"));
	
	pinMode(LED_DIGITAL_PIN, OUTPUT);   // sets the pin as output Active High, LED+ on Port 5 LED- on GND
	#ifdef WITH_BUTTON
	pinMode(TOGGLE_BUTTON, INPUT_PULLUP);   // sets the pin as output
	#endif
	
	Blink();
	
	currentLevel = loadState(LED_LEVEL_EEPROM);
	if ((currentLevel>MAX_LED_LEVEL) || (currentLevel != LED_LEVEL_START))
	{
		currentLevel=LED_LEVEL_START;
		DEBUG_PRINT(F("Save: LED_LEVEL_EEPROM to "));
		DEBUG_PRINTLN(LED_LEVEL_START);
		saveState(LED_LEVEL_EEPROM, currentLevel);//8 Bit
	}
	DEBUG_PRINT(F("restore last LED level: "));
	DEBUG_PRINTLN(currentLevel);
	digitalWrite( LED_DIGITAL_PIN, currentLevel );
}


void setup()
{
	DEBUG_PRINTLN("Setup");
	Blink();
	setIrqOn(); // wird weiter unten nur definiert, falls #WITH_BUTTON definiert wurde
}

void presentation()
{
	DEBUG_PRINTLN(F("Present BinaryLEDSwitch"));
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);
	present(CHILD_SINGLE_LED_SWITCH, 	S_BINARY, 		CHILD_SINGLE_LED_SWITCH_TEXT);
	myPresentation();
}


void loop()
{
	uint32_t currentTime = millis();
	if (firstRun)
	{
		DEBUG_PRINTLN("firstRun");
		setLED(currentLevel);
		myHeartBeatLoop();
		firstRun=false;
	}
	
	if (justReceived)//
	{
		DEBUG_PRINTLN("justReceived");
		setIrqOn(); // wird weiter unten nur definiert, falls #WITH_BUTTON definiert wurde
		justReceived = false;
	}

	if (lastLevel != currentLevel) 
	{
		deBounce();//wait for button to be released
		lastLevel = currentLevel;
		setLED(currentLevel);
	}
	#ifdef WITH_SLEEP
	if (currentLevel == 0)
	{
		DEBUG_PRINTLN("prepare to sleep");
		#ifdef WITH_BUTTON
			setIrqOff();
			int8_t wakeupReason = sleep(digitalPinToInterrupt(TOGGLE_BUTTON), FALLING , SLEEP_TIME, true);
			if (wakeupReason == digitalPinToInterrupt(TOGGLE_BUTTON))
			{
				currentLevel = 1;
				deBounce();//wait for button to be released
				lastButtonPressed=millis();
				DEBUG_PRINTLN("LED on after wake by ButtonIRQ");
				setIrqOn();
			}
		#else
			sleep(SLEEP_TIME, true);		//true = smartSleep	
		#endif
		
		heartBeatCounter++;
		DEBUG_PRINT("leaving sleep: heartBeatCounter ");
		DEBUG_PRINTLN(heartBeatCounter);
	}
	else
	#endif
	{
		uint32_t TimeSinceHeartBeat = currentTime - lastHeartBeat;
		if (TimeSinceHeartBeat > (uint32_t)HEARTBEAT_INTERVAL)
		{
			lastHeartBeat = currentTime;
			myHeartBeatLoop();
			send( msgSwitchState.set(currentLevel) );
		}
	}
	#ifdef WITH_SLEEP
		if (heartBeatCounter > 3)//sendHeartbeat to flush retained messaged
		{
			sendHeartbeat(); 
			heartBeatCounter = 0;
			send( msgSwitchState.set(currentLevel) );
		}
	#endif
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

void setLED(uint8_t newLevel)
{
	DEBUG_PRINT( "setLED: " );
	DEBUG_PRINTLN( newLevel );	
	if (newLevel == 0)
	{
		digitalWrite(LED_DIGITAL_PIN,0);
	}
	else
	{
		digitalWrite(LED_DIGITAL_PIN,newLevel);
	}
	// saveState(LED_LEVEL_EEPROM, newLevel);//8 Bit
	send( msgSwitchState.set(newLevel) );
}

void Blink()
{
	DEBUG_PRINTLN(F("Blink"));
	digitalWrite( LED_DIGITAL_PIN, 1 );
	wait(BLINK_DELAY); 
	digitalWrite( LED_DIGITAL_PIN, 0 );
	wait(BLINK_DELAY*2);
	digitalWrite( LED_DIGITAL_PIN, currentLevel );
}

#ifdef WITH_BUTTON
	void setIrqOn()
	{
		DEBUG_PRINTLN("setIrqOn");
		noInterrupts();
		clearPendingInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON)); //MySensors Funktion
		attachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON), ToggleWhileButtonPressed, FALLING );
		interrupts();
	}

	void setIrqOff()
	{
		DEBUG_PRINTLN("setIrqOff");
		noInterrupts();
		detachInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON));
		clearPendingInterrupt(digitalPinToInterrupt(TOGGLE_BUTTON)); //MySensors Funktion	
		interrupts();
	}


	void deBounce() // inspired by https://gammon.com.au/interrupts
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


	void ToggleWhileButtonPressed()
	{
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

#else
	void setIrqOn(){};
	void setIrqOff(){};
	void deBounce(){};
	void ToggleWhileButtonPressed(){};
#endif
