
// Enable debug prints to serial monitor
#define MY_DEBUG
#define MY_SPECIAL_DEBUG

// #define MY_REPEATER_FEATURE




// Enable and select radio type attached
#define MY_RADIO_RF24
#define MY_RF24_SANITY_CHECK

#define MY_NODE_ID 224
// #define MY_PARENT_NODE_ID 51
// #define MY_PARENT_NODE_IS_STATIC
#define MY_RF24_CHANNEL 96									// Für Testphase deaktivieren, damit Kanal 76 aktiv wird (Prod=96 Test=76)

/**
 * @def MY_RF24_PA_LEVEL
 * @brief Default RF24 PA level. Override in sketch if needed.
 *
 * - RF24_PA_MIN = -18dBm
 * - RF24_PA_LOW = -12dBm
 * - RF24_PA_HIGH = -6dBm
 * - RF24_PA_MAX = 0dBm
 */
 
#define MY_RF24_PA_LEVEL RF24_PA_LOW

#define MY_SIGNAL_REPORT_ENABLED


#define MY_TRANSPORT_WAIT_READY_MS (5000ul)
// #define MY_INDICATION_HANDLER



#include <MySensors.h>
#include <Servo.h>


#define SKETCH_NAME 				"RemoteReceiverActuator"
#define SKETCH_VER 					"1.0-003-2"

#define	SERVO_BIG_STEP				20
#define	SERVO_SMALL_STEP			5
// #define LED_PIN 					3      				// Arduino pin attached to MOSFET Gate pin
// #define FADE_DELAY 				2  					// Delay in ms for each percentage fade up/down (10ms = 1s full-range dim)

#define CHILD_ID_LED       			0     				//ID für Child, welches den MOSFET per PWM ansteuert
#define CHILD_ID_LED_TEXT			"BUTTON_0"
#define LED_CHILD_0_EEPROM			0
#define SENDER_REMOTE_NODE			130

#define HEARTBEAT_INTERVAL			300000        //später alle 5 Minuten, zum Test alle 30 Sekunden

#define SER_DEBUG
#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif

unsigned long lastHeartBeat = HEARTBEAT_INTERVAL - 5000; //das erste Mal sollte nach 5 Sekunden etwas passieren

MyMessage ButtonMsg					(CHILD_ID_LED, V_VAR1);

Servo myservo;  // create servo object to control a servo

uint8_t ServoPos = 90;
uint8_t LastServoPos = 0;
// char CompileDate[4];



void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit...");
	myservo.attach(14);  // attaches the servo on pin 9 to the servo object A0=14
	ServoReset();
}

void before() 
{
	DEBUG_PRINTLN("before: ");
	DEBUG_PRINT(SKETCH_NAME);
	DEBUG_PRINT(" ");
	DEBUG_PRINTLN(SKETCH_VER);
	DEBUG_PRINT(__TIME__);
	DEBUG_PRINT(" ");
	DEBUG_PRINTLN(__DATE__);
}



void setup()
{
	DEBUG_PRINTLN("Setup");

}



void presentation()
{
	DEBUG_PRINTLN("presentation...");
	char CompileDate[8];
	getCompileDateTime(__DATE__, CompileDate);
	DEBUG_PRINTLN(CompileDate);
	
	// char SendString[25] = "01234567890123          "; //mehr als 25 Zeichen werden nicht übertragen
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
	present(CHILD_ID_LED, S_CUSTOM, CHILD_ID_LED_TEXT);

}


void loop()
{
	uint32_t currentTime = millis();
	
	if (currentTime - lastHeartBeat > (unsigned long)HEARTBEAT_INTERVAL)
	{
		lastHeartBeat = currentTime;
		DEBUG_PRINTLN("HeartBeat");
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
	
	// DEBUG_PRINT("sensor: ");
	// DEBUG_PRINTLN(message.sensor);

	DEBUG_PRINT("mGetPayloadType: ");
	DEBUG_PRINTLN(mGetPayloadType(message));
	
	// DEBUG_PRINT("sender: ");
	// DEBUG_PRINTLN(message.sender);

	// DEBUG_PRINT("version: ");
	// DEBUG_PRINTLN(message.version);
	
	// DEBUG_PRINT("getByte: ");
	// uint8_t Temp = message.getByte();
	// DEBUG_PRINTLN(Temp);
	
	
		
	// if ((message.sender == SENDER_REMOTE_NODE) && !mGetAck(message))
	if (!mGetAck(message))
	{
		if (message.type == V_VAR1)
		{
			uint8_t bData = message.getByte();
			DEBUG_PRINT("getByte: ");
			DEBUG_PRINTLN(bData);
			
			String sData = message.getString();
			DEBUG_PRINT("getString: ");
			DEBUG_PRINTLN(sData);
			
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
				DEBUG_PRINTLN("R");
			}
		}
	}
	else
	{
		DEBUG_PRINTLN("unknown receive");
		


	}

}

void ServoReset()
{
	DEBUG_PRINTLN("ServoReset");
	ServoPos=90;
	myservo.write(ServoPos);  
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
}

void getCompileDateTime(char const *date, char *buff) {
    int month, day, year;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", buff, &day, &year);
    month = (strstr(month_names, buff)-month_names)/3+1;
    sprintf(buff, "%d%02d%02d", year, month, day);
}


