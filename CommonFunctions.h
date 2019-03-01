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

#define DIGITAL_INPUT_SENSOR				3					// EnergyMeterPulseSensor, 
#define LED_PWM_PIN							5
#define LED_DIGITAL_PIN						3
#define PULSE_LED							8

#define CHILD_OW_TEMP						10
#define	CHILD_OW_TEMP_TEXT					("OW_Temp")			//flashhelper is not working with functions in OneWireMaster.ino
#define CHILD_OW_CONNECTED					18
#define	CHILD_OW_CONNECTED_TEXT				(F("OW_Con_Lst"))
#define CHILD_OW_DEV_COUNT					19
#define	CHILD_OW_DEV_COUNT_TEXT				(F("OW_Dev_Cnt"))
#define CHILD_OW_TEMP_NAME					20
#define	CHILD_OW_TEMP_NAME_TEXT				("OW_Name")			//flashhelper is not working with functions in OneWireMaster.ino

#define CHILD_OW_RESOLUTION					28
#define	CHILD_OW_RESOLUTION_TEXT			(F("OW_Resolution"))

#define CHILD_MULTI_BUTTON 					30
#define CHILD_MULTI_BUTTON_TEXT				(F("Mlt_Button"))

#define CHILD_POWER_METER 					35
#define CHILD_POWER_METER_TEXT				(F("PowerMeter"))

#define CHILD_SERVO_STATE					40
#define CHILD_SERVO_STATE_TEXT				(F("Servo_State"))

#define CHILD_SINGLE_LED_DIMMER				50
#define CHILD_SINGLE_LED_DIMMER_TEXT		(F("Dimmer"))
#define CHILD_SINGLE_LED_SWITCH				51
#define CHILD_SINGLE_LED_SWITCH_TEXT		(F("Switch"))

#define CHILD_DEBUG_LEVEL					70
#define	CHILD_DEBUG_LEVEL_TEXT				(F("Dbg_Level"))
#define CHILD_NEW_METER_VALUE				71
#define	CHILD_NEW_METER_VALUE_TEXT			(F("SetMeterValue"))

#define CHILD_PASSIVE_NODE					80
#define	CHILD_PASSIVE_NODE_TEXT				(F("PassiveNode"))
#define CHILD_PARENT_NODE					81
#define	CHILD_PARENT_NODE_TEXT				(F("ParentNode"))

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
#define CHILD_PACKET_RATIO					97
#define	CHILD_PACKET_RATIO_TEXT				(F("Pkt_Ratio"))

#define CHILD_BAT_ANLG 						98
#define CHILD_BAT_VREF 						99
#define CHILD_BAT_VREF_TEXT 				(F("Bat"))

#include <VoltageReference.h>				// https://github.com/rlogiacco/VoltageReference Version 1.2.2

MyMessage msgOwTemp							(CHILD_OW_TEMP,  				V_TEMP);			//10-17
MyMessage msgDebugOWConList					(CHILD_OW_CONNECTED,			V_TEXT);			//18
MyMessage msgDebugOWDevCount				(CHILD_OW_DEV_COUNT,			V_TEXT);			//19
MyMessage msgOwName							(CHILD_OW_TEMP_NAME,  			V_TEXT);			//20-27
MyMessage msgOwResolution					(CHILD_OW_RESOLUTION,  			V_TEXT);			//28


MyMessage msgMultiButton					(CHILD_MULTI_BUTTON,			V_TEXT);			//30

MyMessage msgPowerMeter						(CHILD_POWER_METER,				V_WATT);			//35

MyMessage msgServoState						(CHILD_SERVO_STATE,				V_TEXT);			//40

MyMessage msgDimmerState					(CHILD_SINGLE_LED_DIMMER,		V_DIMMER);			//50
MyMessage msgSwitchState					(CHILD_SINGLE_LED_SWITCH,		V_STATUS);			//51

MyMessage msgDebugLevel						(CHILD_DEBUG_LEVEL,				V_TEXT);			//70
MyMessage msgNewMeterValue					(CHILD_NEW_METER_VALUE,			V_TEXT);			//71

MyMessage msgPassiveNode					(CHILD_PASSIVE_NODE, 			V_TEXT);			//80
MyMessage msgParentNode						(CHILD_PARENT_NODE, 			V_TEXT);			//81

MyMessage msgHwTime							(CHILD_HWTIME, 					V_TEXT);
MyMessage msgSendingRSSI					(CHILD_TX_RSSI, 				V_TEXT);
MyMessage msgPaLevel						(CHILD_RF24_PA_LEVEL, 			V_TEXT);
MyMessage msgRFChannel						(CHILD_RF24_CHANNEL, 			V_TEXT);
MyMessage msgEchoTimeStamp					(CHILD_ECHO_TIMESTAMP, 			V_TEXT);
MyMessage msgEchoRunTime					(CHILD_ECHO_RUNTIME, 			V_TEXT);
MyMessage msgDebugReturnString				(CHILD_DEBUG_RETURN, 			V_TEXT);
MyMessage msgPacketRatio					(CHILD_PACKET_RATIO, 			V_TEXT);			//97


MyMessage msgBatvRefValue					(CHILD_BAT_VREF,				V_VOLTAGE);			//99

VoltageReference vRef;

int16_t 	avgRSSI = -29;
int16_t 	nowRSSI = 0;



void myPresentation()
{
	DEBUG_PRINTLN(F("myPresentation"));
	
	// present(CHILD_OW_CONNECTED, 		S_INFO, 			CHILD_OW_CONNECTED_TEXT);//OneWireMaster only
	// present(CHILD_OW_DEV_COUNT, 		S_INFO, 			CHILD_OW_DEV_COUNT_TEXT);//OneWireMaster only
	present(CHILD_PASSIVE_NODE, 		S_INFO,				CHILD_PASSIVE_NODE_TEXT);
	present(CHILD_PARENT_NODE, 			S_INFO,				CHILD_PARENT_NODE_TEXT);
	#if !defined WITH_BATTERY
		present(CHILD_HWTIME, 				S_INFO, 			CHILD_HWTIME_TEXT);
	#endif
	present(CHILD_TX_RSSI, 				S_INFO, 			CHILD_TX_RSSI_TEXT);
	present(CHILD_RF24_PA_LEVEL, 		S_INFO, 			CHILD_RF24_PA_LEVEL_TEXT);
	present(CHILD_RF24_CHANNEL, 		S_INFO,				CHILD_RF24_CHANNEL_TEXT);
	#ifdef MY_ECHO_NODE
		present(CHILD_ECHO_TIMESTAMP, 		S_INFO,				CHILD_ECHO_TIMESTAMP_TEXT);
		present(CHILD_ECHO_RUNTIME, 		S_INFO,				CHILD_ECHO_RUNTIME_TEXT);
		present(CHILD_PACKET_RATIO, 		S_INFO,				CHILD_PACKET_RATIO_TEXT);
	#endif
	present(CHILD_DEBUG_LEVEL,	 		S_INFO,				CHILD_DEBUG_LEVEL_TEXT);
	#if MY_NODE_ID > 99 && MY_NODE_ID < 110
		present(CHILD_NEW_METER_VALUE,		S_INFO,				CHILD_NEW_METER_VALUE_TEXT);
	#endif

	present(CHILD_DEBUG_RETURN, 		S_INFO,				CHILD_DEBUG_RETURN_TEXT);

	#ifdef WITH_BATTERY
		present(CHILD_BAT_VREF,		 		S_MULTIMETER,		CHILD_BAT_VREF_TEXT);
	#endif
	
}

void myHeartBeatLoop()
{
	DEBUG_PRINTLN(F("myHeartBeatLoop"));
	#if !defined WITH_BATTERY //millis is not counted during sleep
		//CHILD_HWTIME
		char buffer[16];
		uint32_t currentSecs = ( millis() / 1000);
		uint8_t day = (currentSecs / 86400) % 365; 
		uint8_t hour = (currentSecs / 3600) % 24; 
		uint8_t min = (currentSecs / 60) % 24; 
		uint8_t sec = currentSecs % 60; 
		sprintf(buffer, "%d %02d:%02d:%02d",day,hour,min,sec);
		send(msgHwTime.set(buffer));
		// DEBUG_PRINT("HWTIME: ");
		// DEBUG_PRINTLN(buffer);
	#endif

	//CHILD_TX_RSSI
	nowRSSI=RF24_getSendingRSSI();
	avgRSSI=((avgRSSI*7)+(nowRSSI))/8;
	// DEBUG_PRINT("avgRSSI: ");
	// DEBUG_PRINTLN(avgRSSI);
	send(msgSendingRSSI.set(avgRSSI));
	send(msgPaLevel.set(PA_LEVEL_TEXT));
	send(msgRFChannel.set(MY_RF24_CHANNEL));
	#ifdef MY_PASSIVE_NODE
		send(msgPassiveNode.set(1));
	#else
		send(msgPassiveNode.set(0));
	#endif
	#if MY_PARENT_NODE_ID == (AUTO)
		// #ifdef MY_PARENT_NODE_IS_STATIC
			// send(msgParentNode.set("S" + MY_PARENT_NODE_ID));
		// #else
		send(msgParentNode.set("AUTO"));
		// #endif
	#else
		send(msgParentNode.set("fixed"));
	#endif	


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

// void PrintRF24Transport()
// {
	// DEBUG_PRINT(F("RF24_getTxPowerLevel: "));
	// DEBUG_PRINTLN(RF24_getTxPowerLevel());
	
	// DEBUG_PRINT(F("RF24_getSendingRSSI:  "));
	// DEBUG_PRINTLN(RF24_getSendingRSSI());
// }

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
	send(msgBatvRefValue.set(batVoltage,3));
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
