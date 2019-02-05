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


#define CHILD_SERVO_STATE					20
#define CHILD_SERVO_STATE_TEXT				(F("SERVO_STATE"))

#define CHILD_MULTI_BUTTON 					30
#define CHILD_MULTI_BUTTON_TEXT				(F("MLT_BUTTON"))






#define CHILD_HWTIME						90
#define	CHILD_HWTIME_TEXT					(F("HWTIME"))
#define CHILD_TX_RSSI						91
#define	CHILD_TX_RSSI_TEXT					(F("TX_RSSI"))
#define CHILD_RF24_PA_LEVEL					92
#define	CHILD_RF24_PA_LEVEL_TEXT			(F("PA_LEVEL"))
#define CHILD_RF24_CHANNEL					93
#define	CHILD_RF24_CHANNEL_TEXT				(F("RF_CHANNEL"))
#define CHILD_ECHO_TIMESTAMP				94
#define	CHILD_ECHO_TIMESTAMP_TEXT			(F("ECHO_TS"))
#define CHILD_ECHO_RUNTIME					95
#define	CHILD_ECHO_RUNTIME_TEXT				(F("ECHO_RT"))
#define CHILD_BAT_ANLG 						98
#define CHILD_BAT_VREF 						99

MyMessage MsgMultiButton					(CHILD_MULTI_BUTTON,			V_TEXT);

MyMessage MsgServoState						(CHILD_SERVO_STATE,				V_TEXT);

MyMessage MsgHwTime							(CHILD_HWTIME, 					V_TEXT);
MyMessage MsgSendingRSSI					(CHILD_TX_RSSI, 				V_TEXT);
MyMessage MsgPaLevel						(CHILD_RF24_PA_LEVEL, 			V_TEXT);
MyMessage MsgRFChannel						(CHILD_RF24_CHANNEL, 			V_TEXT);
MyMessage MsgEchoTimeStamp					(CHILD_ECHO_TIMESTAMP, 			V_TEXT);
MyMessage MsgEchoRunTime					(CHILD_ECHO_RUNTIME, 			V_TEXT);

int16_t 	avgRSSI = -29;
int16_t 	nowRSSI = 0;



void myPresentation()
{
	
	
	
	present(CHILD_HWTIME, 			S_INFO, 	CHILD_HWTIME_TEXT);
	present(CHILD_TX_RSSI, 			S_INFO, 	CHILD_TX_RSSI_TEXT);
	present(CHILD_RF24_PA_LEVEL, 	S_INFO, 	CHILD_RF24_PA_LEVEL_TEXT);
	present(CHILD_RF24_CHANNEL, 	S_INFO,		CHILD_RF24_CHANNEL_TEXT);
	#ifdef MY_ECHO_NODE
	present(CHILD_ECHO_TIMESTAMP, 	S_INFO,		CHILD_ECHO_TIMESTAMP_TEXT);
	present(CHILD_ECHO_RUNTIME, 	S_INFO,		CHILD_ECHO_RUNTIME_TEXT);
	#endif
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
	//CHILD_RF24_PA_LEVEL
	send(MsgPaLevel.set(PA_LEVEL_TEXT));
	//CHILD_RF24_CHANNEL
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