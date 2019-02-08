#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif

#define REQUEST_ACK							true


#if MY_RF24_PA_LEVEL == RF24_PA_MAX
	#define PA_LEVEL_TEXT (F("RF24_PA_MAX"))
#elif MY_RF24_PA_LEVEL == RF24_PA_HIGH
	#define PA_LEVEL_TEXT (F("RF24_PA_HIGH"))
#elif MY_RF24_PA_LEVEL == RF24_PA_LOW
	#define PA_LEVEL_TEXT (F("RF24_PA_LOW"))
#elif MY_RF24_PA_LEVEL == RF24_PA_MIN
	#define PA_LEVEL_TEXT (F("RF24_PA_MIN"))
#endif

#define	BAT_VREF_MAX_VOLTATE				3150
#define	BAT_VREF_MIN_VOLTATE				1800

#define	MIN_ASCII_CHAR						32
#define MAX_ASCII_CHAR						128


#define CHILD_OW_TEMP						10
#define	CHILD_OW_TEMP_TEXT					("OW_Temp")			//flashhelper is not working with functions in OneWireMaster.ino
#define CHILD_OW_CONNECTED					18
#define	CHILD_OW_CONNECTED_TEXT				(F("OW_Con_Lst"))
#define CHILD_OW_DEV_COUNT					19
#define	CHILD_OW_DEV_COUNT_TEXT				(F("OW_Dev_Cnt"))
#define CHILD_OW_TEMP_NAME					20
#define	CHILD_OW_TEMP_NAME_TEXT				("OW_Name")			//flashhelper is not working with functions in OneWireMaster.ino


#define CHILD_SERVO_STATE					20
#define CHILD_SERVO_STATE_TEXT				(F("Servo_State"))

#define CHILD_MULTI_BUTTON 					30
#define CHILD_MULTI_BUTTON_TEXT				(F("Mlt_Button"))

#define CHILD_DEBUG_LEVEL					70
#define	CHILD_DEBUG_LEVEL_TEXT				(F("Dbg_Level"))

#define CHILD_HWTIME						90
#define	CHILD_HWTIME_TEXT					(F("HwTime"))
#define CHILD_TX_RSSI						91
#define	CHILD_TX_RSSI_TEXT					(F("TX_RSSI"))
#define CHILD_RF24_PA_LEVEL					92
#define	CHILD_RF24_PA_LEVEL_TEXT			(F("PA_Level"))
#define CHILD_RF24_CHANNEL					93
#define	CHILD_RF24_CHANNEL_TEXT				(F("RF_Channel"))
#define CHILD_ECHO_TIMESTAMP				94
#define	CHILD_ECHO_TIMESTAMP_TEXT			(F("Echo_TS"))
#define CHILD_ECHO_RUNTIME					95
#define	CHILD_ECHO_RUNTIME_TEXT				(F("Echo_RT"))
#define CHILD_DEBUG_RETURN					96
#define	CHILD_DEBUG_RETURN_TEXT				(F("Dbg_Return"))
#define CHILD_BAT_ANLG 						98
#define CHILD_BAT_VREF 						99
#define CHILD_BAT_VREF_TEXT 				(F("Bat"))

#include <VoltageReference.h>				// https://github.com/rlogiacco/VoltageReference Version 1.2.2

MyMessage MsgOwTemp							(CHILD_OW_TEMP,  				V_TEMP);			//10-17
MyMessage MsgDebugOWConList					(CHILD_OW_CONNECTED,			V_TEXT);			//18
MyMessage MsgDebugOWDevCount				(CHILD_OW_DEV_COUNT,			V_TEXT);			//19
MyMessage MsgOwName							(CHILD_OW_TEMP_NAME,  			V_TEXT);			//20-27


MyMessage MsgMultiButton					(CHILD_MULTI_BUTTON,			V_TEXT);			//30

MyMessage MsgServoState						(CHILD_SERVO_STATE,				V_TEXT);			//40

MyMessage MsgDebugLevel						(CHILD_DEBUG_LEVEL,				V_TEXT);			//70


MyMessage MsgHwTime							(CHILD_HWTIME, 					V_TEXT);
MyMessage MsgSendingRSSI					(CHILD_TX_RSSI, 				V_TEXT);
MyMessage MsgPaLevel						(CHILD_RF24_PA_LEVEL, 			V_TEXT);
MyMessage MsgRFChannel						(CHILD_RF24_CHANNEL, 			V_TEXT);
MyMessage MsgEchoTimeStamp					(CHILD_ECHO_TIMESTAMP, 			V_TEXT);
MyMessage MsgEchoRunTime					(CHILD_ECHO_RUNTIME, 			V_TEXT);
MyMessage MsgDebugReturnString				(CHILD_DEBUG_RETURN, 			V_TEXT);

MyMessage MsgBatvRefValue					(CHILD_BAT_VREF,				V_VOLTAGE);			//99

VoltageReference vRef;

int16_t 	avgRSSI = -29;
int16_t 	nowRSSI = 0;



void myPresentation()
{
	
	// present(CHILD_OW_CONNECTED, 	S_INFO, 			CHILD_OW_CONNECTED_TEXT);//OneWireMaster only
	present(CHILD_HWTIME, 			S_INFO, 			CHILD_HWTIME_TEXT);
	present(CHILD_TX_RSSI, 			S_INFO, 			CHILD_TX_RSSI_TEXT);
	present(CHILD_RF24_PA_LEVEL, 	S_INFO, 			CHILD_RF24_PA_LEVEL_TEXT);
	present(CHILD_RF24_CHANNEL, 	S_INFO,				CHILD_RF24_CHANNEL_TEXT);
	#ifdef MY_ECHO_NODE
	present(CHILD_ECHO_TIMESTAMP, 	S_INFO,				CHILD_ECHO_TIMESTAMP_TEXT);
	present(CHILD_ECHO_RUNTIME, 	S_INFO,				CHILD_ECHO_RUNTIME_TEXT);
	#endif
	present(CHILD_DEBUG_LEVEL,	 	S_INFO,				CHILD_DEBUG_LEVEL_TEXT);
	present(CHILD_DEBUG_RETURN, 	S_INFO,				CHILD_DEBUG_RETURN_TEXT);
	present(CHILD_BAT_VREF,		 	S_MULTIMETER,		CHILD_BAT_VREF_TEXT);
	
}

void myHeartBeatLoop()
{
	//CHILD_HWTIME
	char buffer[14];
	uint32_t currentSecs = ( millis() / 1000);
	uint8_t day = (currentSecs / 86400) % 365; 
	uint8_t hour = (currentSecs / 3600) % 24; 
	uint8_t min = (currentSecs / 60) % 24; 
	uint8_t sec = currentSecs % 60; 
	sprintf(buffer, "%d %02d:%02d:%02d",day,hour,min,sec);
	send(MsgHwTime.set(buffer));
	// DEBUG_PRINT("HWTIME: ");
	// DEBUG_PRINTLN(buffer);
	//CHILD_TX_RSSI
	nowRSSI=RF24_getSendingRSSI();
	avgRSSI=((avgRSSI*7)+(nowRSSI))/8;
	// DEBUG_PRINT("avgRSSI: ");
	// DEBUG_PRINTLN(avgRSSI);
	send(MsgSendingRSSI.set(avgRSSI));
	send(MsgPaLevel.set(PA_LEVEL_TEXT));
	send(MsgRFChannel.set(MY_RF24_CHANNEL));

}

void getCompileDateTime(char const *date, char *buff) {
    int month, day, year;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", buff, &day, &year);
    month = (strstr(month_names, buff)-month_names)/3+1;
    sprintf(buff, "%d%02d%02d", year, month, day);
}

void mySendInt8(MyMessage ThisMessage, uint8_t Integer)
{
	int myCounter=0;
	bool sendStatus=false;
	// DEBUG_PRINT("mySend: ");
	// DEBUG_PRINTLN(Integer);
	// ThisMessage.setDestination(RECEIVER_NODE);
	while ( !sendStatus and (myCounter < 5))
	{
		// ThisMessage.setDestination(RECEIVER_NODE);
		sendStatus = send(ThisMessage.set(Integer), REQUEST_ACK);
		// DEBUG_PRINT("sendStatus >");
		// DEBUG_PRINT(sendStatus);
		// DEBUG_PRINT("< myCounter >");
		// DEBUG_PRINT(myCounter);
		// DEBUG_PRINTLN("<");
		myCounter++;
		wait(100*myCounter*2);
	}
}

void mySendString(MyMessage ThisMessage, const char *myString)
{
	int myCounter=0;
	bool sendStatus=false;
	// DEBUG_PRINT("mySend: ");
	// DEBUG_PRINTLN(myString);
	// ThisMessage.setDestination(RECEIVER_NODE);
	while ( !sendStatus and (myCounter < 5))
	{
		// ThisMessage.setDestination(RECEIVER_NODE);
		sendStatus = send(ThisMessage.set(myString), REQUEST_ACK);
		// DEBUG_PRINT("sendStatus >");
		// DEBUG_PRINT(sendStatus);
		// DEBUG_PRINT("< myCounter >");
		// DEBUG_PRINT(myCounter);
		// DEBUG_PRINTLN("<");
		myCounter++;
		wait(100*myCounter*2);
	}
}

void mySendSketchInfo()
{
	char CompileDate[8];
	getCompileDateTime(__DATE__, CompileDate);
	
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
}

void PrintRF24Transport()
{
	DEBUG_PRINT(F("RF24_getTxPowerLevel: "));
	DEBUG_PRINTLN(RF24_getTxPowerLevel());
	
	DEBUG_PRINT(F("RF24_getSendingRSSI:  "));
	DEBUG_PRINTLN(RF24_getSendingRSSI());
}

void BatteryVRef()
{
	uint8_t batteryPcntVcc;
	float batVoltage;
	uint16_t vcc = vRef.readVcc(); //5000 oder 3000 mA
	// uint16_t vccCorrect = (vcc * float(BAT_VREF_CORRECTION_VALUE));
	
	// DEBUG_PRINT("vcc vor constrain: ");
	// DEBUG_PRINTLN(vcc);

	vcc=constrain(vcc, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE);
	batteryPcntVcc = map(vcc, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE, 0, 100); 

	// DEBUG_PRINT("vcc: ");
	// DEBUG_PRINTLN(vcc);
	
	batVoltage  = (float)vcc / 1000;

	// DEBUG_PRINT("batVoltage: ");
	// DEBUG_PRINTLN(batVoltage);
	
	sendBatteryLevel(batteryPcntVcc);
	send(MsgBatvRefValue.set(batVoltage,3));
}

void showEEpromChar()
{
	// debugMessage("showEEprom", "");
	uint8_t counter=0;
	uint8_t Zeichen;
	for (uint8_t i = 0; i<16; i++)
	{
		for (uint8_t j = 0; j<16; j++)
		{
			Zeichen=loadState(counter);
			if ((Zeichen >= (uint8_t)MIN_ASCII_CHAR ) and (Zeichen <= (uint8_t)MAX_ASCII_CHAR))
			{
				Serial.print((char)Zeichen);
			}
			else//not printable chars	
			{
				Serial.print('_');
			}
			
			if (j < 15)
			{
				Serial.print(' ');
			}
			counter++;
		}	
		Serial.println("");
	}
}

void showEEpromHex()
{
	// debugMessage("showEEprom", "");
	uint8_t counter=0;
	uint8_t Zeichen;
	for (uint8_t i = 0; i<16; i++)
	{
		for (uint8_t j = 0; j<16; j++)
		{
			Zeichen=loadState(counter);
			if (Zeichen < 16)
			{
				Serial.print("0");
				
			}
			Serial.print(String(Zeichen,HEX));
			if (j < 15)
			{
				Serial.print("-");
			}
			counter++;
		}	
		Serial.println("");
	}
}
