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
#define MY_RF24_PA_LEVEL 					RF24_PA_LOW
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
// #define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)
#define MY_RF24_SANITY_CHECK

#define MY_NODE_ID 							224
#define MY_PARENT_NODE_ID 					0
#define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


#define SKETCH_NAME 						"RemoteReceiverActuator"
#define SKETCH_VER 							"1.0-005"

#define	SERVO_BIG_STEP						20
#define	SERVO_SMALL_STEP					5
// #define LED_PIN 							3      				// Arduino pin attached to MOSFET Gate pin
// #define FADE_DELAY 						2  					// Delay in ms for each percentage fade up/down (10ms = 1s full-range dim)


#define SENDER_REMOTE_NODE					130

#define HEARTBEAT_INTERVAL					30000        //später alle 5 Minuten, zum Test alle 30 Sekunden



#include <MySensors.h>
#include <Servo.h>
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen


uint32_t lastHeartBeat = HEARTBEAT_INTERVAL - 5000; //das erste Mal sollte nach 5 Sekunden etwas passieren

// MyMessage MsgMultiButton					(CHILD_BUTTON , V_TEXT);
// MyMessage MsgServoState				(CHILD_SERVO_STATE , V_TEXT);
// MyMessage MsgHwTime					(CHILD_HWTIME, V_TEXT);
// MyMessage MsgSendingRSSI			(CHILD_TX_POWER_LEVEL, V_TEXT);

Servo myservo;  // create servo object to control a servo

uint8_t ServoPos = 90;
uint8_t LastServoPos = 0;
// char CompileDate[4];



void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit...");
	myservo.attach(14);  // attaches the servo on pin 9 to the servo object A0=14
	myservo.write(50);
	wait(500);
	myservo.write(110);
}

void before()
{
	DEBUG_PRINTLN("before...");
    // DEBUG_PRINTLN("Started clearing. Please wait...");
    // for (uint16_t i=0; i<EEPROM_LOCAL_CONFIG_ADDRESS; i++) {
		// DEBUG_PRINTLN(i);
        // hwWriteConfig(i,0xFF);
    // }
    // DEBUG_PRINTLN("Clearing done.");
}

void setup()
{
	DEBUG_PRINTLN("Setup");

}

void presentation()
{
	DEBUG_PRINTLN("presentation...");
	mySendSketchInfo();
	myPresentation();
	present(CHILD_MULTI_BUTTON,		S_INFO,	CHILD_MULTI_BUTTON_TEXT);
	present(CHILD_SERVO_STATE,		S_INFO, CHILD_SERVO_STATE_TEXT);
}

void loop()
{
	uint32_t currentTime = millis();
	
	if (currentTime - lastHeartBeat > (uint32_t)HEARTBEAT_INTERVAL)
	{
		myHeartBeatLoop();
		lastHeartBeat = currentTime;
	}
	
	if (ServoPos != LastServoPos)
	{
		LastServoPos = ServoPos;
		DEBUG_PRINT("ServoPos: ");
		DEBUG_PRINTLN(ServoPos);
	}
	
}

void receive(const MyMessage &message)
{
	

	
		
	// if ((message.sender == SENDER_REMOTE_NODE) && !mGetAck(message))
	if (!mGetAck(message))
	{
		if ((message.type == V_TEXT) && (message.sensor == CHILD_MULTI_BUTTON))
		{

			String sData = message.getString();
			// DEBUG_PRINT("getString: ");
			// DEBUG_PRINTLN(sData);
			
			if (sData[0] == 'U')
			{
				ServoUp(SERVO_BIG_STEP);
				DEBUG_PRINTLN("U");
			}
			else if (sData[0] == 'u')
			{
				ServoUp(SERVO_SMALL_STEP);
				DEBUG_PRINTLN("u");
			}
			else if (sData[0] == 'd')
			{
				ServoDown(SERVO_SMALL_STEP);
				DEBUG_PRINTLN("d");
			}
			else if (sData[0] == 'D')
			{
				ServoDown(SERVO_BIG_STEP);
				DEBUG_PRINTLN("D");
			}
			else if (sData[0] == 'R')
			{
				ServoReset();
				// PrintRF24Transport();
				DEBUG_PRINTLN("R");
			}
			else
			{
				DEBUG_PRINT("unknown V_TEXT: ");
				DEBUG_PRINTLN(sData);
			}
		}
	}
	else
	{
		DEBUG_PRINTLN("unknown receive");
		
		DEBUG_PRINT("sensor: ");
		DEBUG_PRINTLN(message.sensor);

		DEBUG_PRINT("mGetPayloadType: ");
		DEBUG_PRINTLN(mGetPayloadType(message));
		
		DEBUG_PRINT("sender: ");
		DEBUG_PRINTLN(message.sender);

		DEBUG_PRINT("version: ");
		DEBUG_PRINTLN(message.version);
		
		DEBUG_PRINT("getByte: ");
		uint8_t Temp = message.getByte();
		DEBUG_PRINTLN(Temp);
	
	}

}

void ServoReset()
{
	DEBUG_PRINTLN("ServoReset");
	ServoPos=90;
	myservo.write(ServoPos);
	mySendInt8(MsgServoState, ServoPos);
}

void ServoDown(uint8_t steps)
{
	DEBUG_PRINTLN("ServoDown");
	if (ServoPos < (180-steps))
	{
		ServoPos+=steps;
	}
	else
	{
		ServoPos=180;
	}
	myservo.write(ServoPos);
	mySendInt8(MsgServoState, ServoPos);
}

void ServoUp(uint8_t steps)
{
	DEBUG_PRINTLN("ServoUp");
	if (ServoPos > (0+steps))
	{
		ServoPos-=steps;
	}
	else
	{
		ServoPos=0;
	}
	myservo.write(ServoPos); 
	mySendInt8(MsgServoState, ServoPos);
}

