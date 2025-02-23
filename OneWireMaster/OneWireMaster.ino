/**

#############################		Versionen		###################################
 
20221115 Version 2.01		Versionierung begonnen
							Präprozessoranweisungen für OneWireMaster hinzugefügt
20230124 Version 2.02		SLEEP_TIME auf 300000 statt 600000
20230124 Version 2.03		wait(SEND_WAIT), RF24_PA_MIN damit der Repeater (MYSEnergy) nicht überlagert wird
20230124 Version 2.04		Statistische Werte und myHeartBeatLoop nur noch alle 10 Durchläufe senden
20230209 Version 2.05		Anpassungen, damit neue Devices schneller in FHEM angezeigt werden
20230209 Version 2.06		Erweitertes Debugging, womit der Speicherplatz für die Sensoren manipuliert werden kann.
20230228 Version 2.07		Code umstrukturiert, ohne funktionelle Änderungen
20230228 Version 2.08		MY_INDICATION_HANDLER automatisch zwischen NodeID 150 und 180

*/


// ToDo: über Debug oder anderen Child immer nur einen Speicherplatz für einen defekten Sensor zurücksetzen (statt ClearEeprom)

#define SKETCH_VER            				"2.08"        			// Sketch version
#define SKETCH_NAME           				"OneWireMaster"   		// Optional child sensor name



//	###################   Debugging   #####################
// #define MY_DEBUG											//Output kann im LogParser analysiert werden https://www.mysensors.org/build/parser
#define SER_DEBUG											// aus CommonFunctions.h für eigenes DEBUG_PRINT
#define MY_SPECIAL_DEBUG									// für Extended Debug in FHEM
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
// #define MY_DEFAULT_RX_LED_PIN				6
// #define MY_DEFAULT_TX_LED_PIN 				7
// #define MY_DEFAULT_ERR_LED_PIN				8
// #define MY_DEFAULT_LED_BLINK_PERIOD 		10

// ###################   Transport   #####################
/*
RF24_PA_MIN = 	-18dBm 		0	R_TX_Powerlevel_Pct 	25
RF24_PA_LOW = 	-12dBm 		1	R_TX_Powerlevel_Pct
RF24_PA_HIGH = 	-6dBm 		2	R_TX_Powerlevel_Pct
RF24_PA_MAX = 	 0dBm		3	R_TX_Powerlevel_Pct
*/

#define MY_RF24_PA_LEVEL 					RF24_PA_MAX			//default für MYSTemp1(NodeID 150) = RF24_PA_LOW (solange er im Heizraum hängt)
#define MY_RADIO_RF24
// #define MY_RF24_CE_PIN 						10 				//nur für RF-Nano verwenden
// #define MY_RF24_CS_PIN 						9				//nur für RF-Nano verwenden
// #define MY_RF24_DATARATE 					RF24_1MBPS		//nur verwenden, wenn mindestens 1 RF-Nano im Netzwerk eingebucht werden soll

#define MY_RF24_CHANNEL 					96
#define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)
// #define MY_TRANSPORT_SANITY_CHECK			//enable regular transport sanity checks -> wirkt nur bei Gateway oder Repeater, ist dort per default aktiviert


// ########   Node MYSTemp1  #######################################
#define MY_NODE_ID 							150				//MYSTemp1
#define MY_PARENT_NODE_ID 					0				//without this the node broadcasts everything to parent 255 (dont know what happens, if 2 repeater receive this at the same time)
#define MY_PARENT_NODE_IS_STATIC
#define SLEEP_TIME							180000				//default:	180000 bei Batterie Sensoren 600000

// ########  Batterie Node  #######################################
// #define MY_NODE_ID 							180				//Test
// #define MY_PARENT_NODE_ID 					0				//without this the node broadcasts everything to parent 255 (dont know what happens, if 2 repeater receive this at the same time)
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE										//default: deaktiviert -> Nur bei Batteriesensoren sillvoll, die im Grenzbereich für den Empfang liegen
// #define SLEEP_TIME							600000				//default:	180000 bei Batterie Sensoren 600000

// ########  ToDo andere Nodes  #######################################

// #define MY_NODE_ID 							181				//Teichwasser Batterie Sensor
// #define MY_NODE_ID 							182				//Gartenhaus Temperatur Sensors



// ###################   Node Spezifisch   #####################

#define HEARTBEAT_INTERVAL        			10        //wird bei OneWireMaster als Anzahl der Loops benutzt

#define ONE_WIRE_BUS 4 // Pin where dallas sensor is connected 
#define OW_RESOLUTION						11
#define MAX_ATTACHED_DS18B20      			8					//mehr als 8 funktioniert nicht mit den vorhandenen Methoden

#define EEPROM_DEVICE_NAME_LENGTH   		8
#define EEPROM_DEVICE_ID_LENGTH     		8
#define EEPROM_DEVICE_CNT_STEP				(EEPROM_DEVICE_NAME_LENGTH+EEPROM_DEVICE_ID_LENGTH) //Klammer könnte evtl. weg
//Werte für 
#define EEPROM_DEVICE_DEBUG_LEVEL   		0		// DebugLevel, die nächsten 7 uint8_ts sind für Debug reserviert
#define EEPROM_DEVICE_CHECKSUM				1		// Checksum of connected devices (first + second byte from Device ID)
#define EEPROM_DEVICE_DEBUG_7 				7

#define EEPROM_DEVICE_TEMP_ID_START			EEPROM_DEVICE_DEBUG_7+1+8
#define EEPROM_DEVICE_TEMP_NAME_START		EEPROM_DEVICE_TEMP_ID_START+EEPROM_DEVICE_ID_LENGTH

#define MAX_DEBUG_LEVEL         			9



/** Werte für Debug überschreiben **/
// #ifdef SER_DEBUG

// #define SLEEP_TIME          				600000       //default: 600000
// #define MY_NODE_ID               			180

// #endif



// ###################   Allgemeine MySensors Funktionen aus CommonFunctions.h (erhöht den Speicherverbrauch   #####################

#if MY_NODE_ID >= 150 and MY_NODE_ID < 180
	// #define	WITH_BATTERY						//DS18B20 Sensoren funktionieren nicht mit weniger als 2,5V, eigentlich müsste OneWireMaster mit 5V und Netzteil betriebern werden
	#define WITH_HWTIME
	#define WITH_RF24_INFO						// übermittel RSSI, PA_Level, RF_Channel
	#define WITH_NODE_INFO						// übermittel NodeID, ParentNodeID
	#define MY_INDICATION_HANDLER               //erlaubt rewrite der Funktion void indication(indication_t ind) Tx_ERR, Tx_OK, TX_RSSI
	#define AUTO_REBOOT							// falls MY_INDICATION_HANDLER aktiviert wurde und RSSI schlechter als -70, dann Node rebooten


#else
	#define	WITH_BATTERY						//DS18B20 Sensoren funktionieren nicht mit weniger als 2,5V, eigentlich müsste OneWireMaster mit 5V und Netzteil betriebern werden
	// #define WITH_HWTIME
	// #define WITH_RF24_INFO						// übermittel RSSI, PA_Level, RF_Channel
	#define WITH_NODE_INFO						// übermittel NodeID, ParentNodeID

#endif



#include <MySensors.h>
#include <DallasTemperature.h>	// https://github.com/milesburton/Arduino-Temperature-Control-Library Version 3.8.0
#include <OneWire.h>			// https://www.pjrc.com/teensy/td_libs_OneWire.html Version 2.3.4
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen
// #include "/home/tmandel/1_Entwicklung/git/MySensors/CommonFunctions.h" //muss nach allen anderen #defines stehen


OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
DeviceAddress tempDeviceAddress; 		//typedef uint8_t DeviceAddress[8];


char cThermoName[]="-8Name8-";
char cThermoKnown[]="FFFFFFFF";		// wird mittels initializeThermoKnown neu initialisiert

volatile bool	doPresentation	= true;
volatile bool	doReadTemp		= true;
uint8_t			debugLevel		= 0; 
uint8_t			DevCheckSum 	= 0; 
uint8_t			DevLastCheckSum = 0; 
uint8_t			loopCount 		= (uint8_t)HEARTBEAT_INTERVAL; //vorbelegen, damit beim gleich beim Neustart statistische Werte übermittelt werden
uint32_t		lastReadTemp	= 0;

// debugLevel switches

//	0 		off
//	1		showEEpromHex()
//	2		scanne nach aktuell angeschlossenen OwDevices und präsentiere diese
//	3		wie 2, aber vorher ClearEeprom (OneWire Adresse freigeben)
//	4 		wie 3, aber mit anschließendem hwReboot()
//	6		myHeartBeatLoop
//	9		Reboot /Arduino hängt sich auf, wenn default Bootloader installiert ist. Optiboot V8 von 2018 funktioniert (https://github.com/Optiboot/optiboot/releases/tag/v8.0)
//	20-27	Hardware-Adresse im EEPROM von Sensor 0-7 ungültig machen (Sensor wird dann als D=Disconneted angezeigt)
//	30-37	Hardware-Adresse im EEPROM von Sensor 0-7 freigeben (0xFF setzen)

void preHwInit() //kein serieller Output 
{
	#if defined(MY_DISABLED_SERIAL)
	  //Serielles Interface nur setzten, falls nicht schon von MySensors Framework erledledigt
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
	  //Serielles Interface nur setzten, falls nicht schon von MySensors Framework erledledigt
		#if defined SER_DEBUG
			DEBUG_PRINTLN(F("NO MySensors Serial Interface. Starting own Interface for Debug"));
		#else
			Serial.println(F("NO MySensors Serial Interface. No Debug"));
		#endif
	#else
		Serial.println(F("MySensors already activated Serial Interface"));
	#endif
  
	DEBUG_PRINTLN(F("before"));
	
	Serial.print(F("RF24_CHANNEL "));
	Serial.println(MY_RF24_CHANNEL);
  
	
	debugLevel = loadState(EEPROM_DEVICE_DEBUG_LEVEL); 	//8 Bit
	DevCheckSum = loadState(EEPROM_DEVICE_CHECKSUM); 	//8 Bit
	DevLastCheckSum = DevCheckSum;
	if (debugLevel>MAX_DEBUG_LEVEL)
	{
		debugLevel=0;
		DEBUG_PRINTLN(F("Save: debugLevel to 0"));
		saveState(EEPROM_DEVICE_DEBUG_LEVEL, debugLevel);//8 Bit
	}
	DEBUG_PRINT(F("DevCheckSum from EEPROM "));
	DEBUG_PRINTLN(DevCheckSum);
	
	// Startup up the OneWire library
	sensors.begin();

	#ifdef SER_DEBUG
		showEEpromHex();
		showEEpromChar();
	#endif

	sortOwAdresses();//prüft das EEPROM, welche TempAddress für OW Geräte bekannt sind und waren
	checkResolution();

}


void setup()
{
	DEBUG_PRINTLN(F("setup"));
	sensors.setWaitForConversion(false);//damit mysensors nicht blockiert

}

void presentation()
{
	doPresentation=false;
	DEBUG_PRINTLN(F("Present OneWire"));
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);
	
	char ChildTextTemp[sizeof(CHILD_OW_TEMP_TEXT)+1]=CHILD_OW_TEMP_TEXT;			// vorbelegen mit OW_Temp
	char ChildTextName[sizeof(CHILD_OW_TEMP_NAME_TEXT)+1]=CHILD_OW_TEMP_NAME_TEXT;	// vorbelegen mit OW_Name
	char SendString[25] = ""; //mehr als 25 Zeichen werden nicht übertragen
	
	wait(SEND_WAIT);	
	present(CHILD_OW_CONNECTED, 		S_INFO, 		CHILD_OW_CONNECTED_TEXT);
	wait(SEND_WAIT);
	present(CHILD_OW_DEV_COUNT, 		S_INFO, 		CHILD_OW_DEV_COUNT_TEXT);
	wait(SEND_WAIT);
	present(CHILD_OW_RESOLUTION,		S_INFO,			CHILD_OW_RESOLUTION_TEXT);

	for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)
	{
		if (cThermoKnown[i] == 'C')//weitermachen, wenn Y
		{
			sprintf(SendString, "%s_%d",ChildTextTemp,i);		// ergibt OW_Temp _ 0-7
			DEBUG_PRINT(F("SendString ChildTextTemp "));
			DEBUG_PRINTLN(SendString);
			present(CHILD_OW_TEMP+i, 		S_TEMP,		SendString);
			wait(SEND_WAIT);
			sprintf(SendString, "%s_%d",ChildTextName,i);		// ergibt OW_Name _ 0-7
			DEBUG_PRINT(F("SendString ChildTextName "));		
			DEBUG_PRINTLN(SendString);
			present(CHILD_OW_TEMP_NAME+i, 	S_INFO, 	SendString);
			wait(SEND_WAIT);
			
		}
	}
	
	myPresentation();
}

void loop()
{
	if (doPresentation)
	{
		sortOwAdresses();
		presentation();
	}
	
	
	//	Temp Sensor ohne Batterie sollte ständig auf Änderungen von debugLevel reagieren. 
	//	deshalb wird loop ständig durchlaufen, statt per wait() gewartet
	
	#ifdef WITH_BATTERY
		doReadTemp=1;
	#else
		uint32_t currentTime = millis();
		if ((currentTime - lastReadTemp) > (uint32_t)SLEEP_TIME)
		{
			lastReadTemp=currentTime;
			doReadTemp=1;
		}
	
	#endif
	
	if (doReadTemp)
	{
		#ifndef WITH_BATTERY
			lastReadTemp=millis();
		#endif
		
		
		
		
		sortOwAdresses();
		sensors.requestTemperatures();
		//int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
		
		#ifdef WITH_BATTERY
			DEBUG_PRINTLN(F("smartSleep 750"));
			smartSleep(750);//conversionTime usually is 750 ms 
		#else
			DEBUG_PRINTLN(F("wait 750"));
			wait(750);//conversionTime usually is 750 ms 
		#endif
		if (DevCheckSum != DevLastCheckSum)	//New Devices on OW Bus have to be presented
		{
			DEBUG_PRINT(F("Save new DevCheckSum "));
			DEBUG_PRINTLN(DevCheckSum);
			DevLastCheckSum = DevCheckSum;
			saveState(EEPROM_DEVICE_CHECKSUM, DevCheckSum);
			presentation();
		}


		for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)
		{
			if (cThermoKnown[i] == 'C')//weitermachen, wenn 1
			{
				for (uint8_t j=0;j<EEPROM_DEVICE_ID_LENGTH;j++)
				{
					tempDeviceAddress[j]=loadState(EEPROM_DEVICE_TEMP_ID_START+(i*EEPROM_DEVICE_CNT_STEP)+j);
				}
				float temperature = sensors.getTempC(tempDeviceAddress);
				if (temperature < -50.00 || temperature > 80.00) 
				{
					DEBUG_PRINT(F("WrongTemp: "));
					DEBUG_PRINTLN(temperature);
					send(msgDebugReturnString.set(F("Wrong Temp < -50 our > 80")));
					wait(SEND_WAIT);
				}
				else
				{
					send(msgOwTemp.setSensor(CHILD_OW_TEMP+i).setType(V_TEMP).set(temperature,1));//Temp mit einer Nachkommastelle senden
					wait(SEND_WAIT);
					#ifdef SER_DEBUG
						DEBUG_PRINT(F("GoodTemp: "));
						DEBUG_PRINT(temperature);
						LoadName(i);//Load from EEPROM to volatile char cThermoName
						DEBUG_PRINT(F(" Idx: "));
						DEBUG_PRINT(i);
						DEBUG_PRINT(F(" Nm: "));
						DEBUG_PRINTLN(cThermoName);
					#endif
					
				}
			}
		}
		doReadTemp=0;
		loopCount++;
	}
	
	if (loopCount >= (uint8_t)HEARTBEAT_INTERVAL)
	{
		DEBUG_PRINTLN(F("Heartbeat Loop"));
		loopCount=0;
		send(msgDebugLevel.set(debugLevel));
		wait(SEND_WAIT);
		send(msgDebugOWConList.set(cThermoKnown));
		wait(SEND_WAIT);
		send(msgOwResolution.set(OW_RESOLUTION));
		wait(SEND_WAIT);
		
		for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)
		{
			if (cThermoKnown[i] == 'C')//weitermachen, wenn C = connected
			{
				DEBUG_PRINT("Pos ");
				DEBUG_PRINT(i);
				DEBUG_PRINT(" ");
				for (uint8_t j=0;j<EEPROM_DEVICE_ID_LENGTH;j++)
				{
					tempDeviceAddress[j]=loadState(EEPROM_DEVICE_TEMP_ID_START+(i*EEPROM_DEVICE_CNT_STEP)+j);
				}
				LoadName(i);//Load from EEPROM to volatile char cThermoName
				DEBUG_PRINTLN(cThermoName);
				send(msgOwName.setSensor(CHILD_OW_TEMP_NAME+i).set(cThermoName));
				wait(SEND_WAIT);
				send(msgOwTemp.setSensor(CHILD_OW_TEMP+i).setType(V_ID).set(tempDeviceAddress,8));
				wait(SEND_WAIT);

			}
		}
		myHeartBeatLoop();
	}


	#ifdef WITH_BATTERY
		DEBUG_PRINT(F("smartSleep "));
		DEBUG_PRINTLN(SLEEP_TIME);
		smartSleep((uint32_t)SLEEP_TIME);
		// doReadTemp=1;
	#endif

}


void checkResolution()//schreibt die Temperatur Auflösung ins EEPROM des DS18B20. Die Schreibzyklen sind begrenzt. Es reicht, wenn diese Routine im Setup aufgerufen wird.
{
	DEBUG_PRINTLN(F("checkResolution"));
	for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)
	{
		if (cThermoKnown[i] == 'C')//weitermachen, wenn C = connected
		{
			for (uint8_t j=0;j<EEPROM_DEVICE_ID_LENGTH;j++)
			{
				// Call address of connected OW-Device from EEPROM instead of scanning OW-Bus
				tempDeviceAddress[j]=loadState(EEPROM_DEVICE_TEMP_ID_START+(i*EEPROM_DEVICE_CNT_STEP)+j);
			}
			uint8_t resolution = sensors.getResolution(tempDeviceAddress);
			if (resolution != OW_RESOLUTION) {
				sensors.setResolution(tempDeviceAddress,OW_RESOLUTION,false);
				DEBUG_PRINT(F("Changing Resolution to: "));
				DEBUG_PRINTLN(String(OW_RESOLUTION,DEC));
				wait(100);
			}
		}	
	}
}


void sortOwAdresses()
{
	oneWire.reset_search();
	initializeThermoKnown();	// cThermoKnown vorbelegen
	uint8_t Index=0;
	uint8_t DeviceCounter=0;
	uint8_t nextFreeAddress=0;
	uint8_t thisDevCheckSum=0;
	

	DEBUG_PRINTLN(F("\nLooking for OW Devices"));
	
	// DEBUG_PRINT(F("cThermoKnown: "));
	// DEBUG_PRINTLN(cThermoKnown);
	
	while (oneWire.search(tempDeviceAddress))
	{
		DEBUG_PRINT(F("Found OwID: "));
		DEBUG_PRINTLN(DevAddressToHex(tempDeviceAddress));

		// Serial.println(String(*tempDeviceAddress));
		// DEBUG_PRINT("DeviceCounter: ");
		// DEBUG_PRINTLN(DeviceCounter);	
		if (DeviceCounter < MAX_ATTACHED_DS18B20)
		{
			Index=getTempDevAddressIndex(tempDeviceAddress);
			// DEBUG_PRINT(F("EEPROM Idx: "));
			// DEBUG_PRINTLN(Index);
			
			if (Index < MAX_ATTACHED_DS18B20)
			{
				cThermoKnown[Index]='C';
				DEBUG_PRINT(F("Dev is known Idx: "));
				DEBUG_PRINTLN(Index);
			}
			else//owID wurde im EEPROM nicht gefunden und sollte deshalb gespeichert werden
			{
				nextFreeAddress=getNextFreeEepromAddress();
				if (nextFreeAddress < MAX_ATTACHED_DS18B20)
				{
					DEBUG_PRINT(F("Save New OW Device Idx: "));
					DEBUG_PRINTLN(nextFreeAddress);
					send(msgDebugReturnString.set(F("Neues OW Device")));
					SaveOwID(nextFreeAddress,tempDeviceAddress);
					cThermoKnown[nextFreeAddress]='C';
				}
				else
				{
					// cThermoKnown[nextFreeAddress]='N';
					DEBUG_PRINTLN(F("EEPROM full"));
					send(msgDebugReturnString.set(F("Speicher ist voll")));
				}
			}
		}
		else
		{
			DEBUG_PRINTLN(F("OneWire Bus ist ueberfuellt"));
			send(msgDebugReturnString.set(F("OneWire Bus ist ueberfuellt")));
		}
		thisDevCheckSum=thisDevCheckSum+tempDeviceAddress[1]+tempDeviceAddress[2]+Index;
		// DEBUG_PRINT("thisDevCheckSum: ");
		// DEBUG_PRINTLN(thisDevCheckSum);
		
		DeviceCounter++;
	}
	
	DEBUG_PRINT(F("cThermoKnown: "));
	DEBUG_PRINTLN(cThermoKnown);
	
	DevCheckSum=thisDevCheckSum;
	send(msgDebugOWDevCount.set(DeviceCounter)); //Update controller with number of found devices
}


void SaveOwID(uint8_t nextFreeAddress, DeviceAddress tempDeviceAddress)
{
	send(msgDebugReturnString.set(F("SaveOwID")));
	uint8_t tempAddressIndex=EEPROM_DEVICE_TEMP_ID_START+nextFreeAddress*EEPROM_DEVICE_CNT_STEP;
	for (uint8_t j = 0; j<EEPROM_DEVICE_ID_LENGTH; j++)
	{
		saveState((tempAddressIndex+j),tempDeviceAddress[j]);
	}
}


String DevAddressToHex(uint8_t* data) //Input: 8 Char Arry Output HEX representation of that 28F8DA5E050000D6
{
	String strAddr;
	for (uint8_t i = 0; i < 8; i++) 
	{
		if (data[i] < 16)
		{
			strAddr = strAddr + 0;
		}
		strAddr = strAddr + String(data[i], HEX);
	}
	strAddr.toUpperCase();
	return strAddr;
}


uint8_t getTempDevAddressIndex(DeviceAddress devAddress)
{
	uint8_t TempAddress[8];
	uint8_t IndexCnt=0;
	for (uint8_t i = EEPROM_DEVICE_TEMP_ID_START; i < EEPROM_DEVICE_TEMP_ID_START+MAX_ATTACHED_DS18B20*EEPROM_DEVICE_CNT_STEP; i=i+EEPROM_DEVICE_CNT_STEP)
	{
		// debugMessage("Checking EEPROM on DevAddressIndex: ",String(i,DEC));
		uint8_t counter=0;
		for (uint8_t j=0;j<EEPROM_DEVICE_ID_LENGTH;j++)
		{
			
			TempAddress[j]=loadState(j+i);
			if (TempAddress[j] == devAddress[j])
			{
				// Serial.print(String(TempAddress[j],HEX));
				counter++;
			}
			else
			{
				break;
			}
		}
		
		if (counter == 8)//alle 8 Blöcke waren gleich
		{

			return IndexCnt;
		}
		IndexCnt++;
	}
	
	return IndexCnt;//counter ist 1 größer als MAX_ATTACHED_DS18B20, damit wird signalisiert, dass das neue OW-Device noch nicht abgespeichert wurde
}


void initializeThermoKnown()
{
	uint8_t TempChar;
	uint8_t counter=0;
	for (uint8_t i = EEPROM_DEVICE_TEMP_ID_START; i < EEPROM_DEVICE_TEMP_ID_START+MAX_ATTACHED_DS18B20*EEPROM_DEVICE_CNT_STEP; i=i+EEPROM_DEVICE_CNT_STEP)
	{
		TempChar=loadState(i);
		if (TempChar == 0xFF)
		{
			cThermoKnown[counter]='F';	//free
		}
		else
		{
			cThermoKnown[counter]='D';	//disconnected
		}
		counter++;
	}
}

uint8_t getNextFreeEepromAddress()
{
	uint8_t TempChar;
	uint8_t counter=0;
	for (uint8_t i = EEPROM_DEVICE_TEMP_ID_START; i < EEPROM_DEVICE_TEMP_ID_START+MAX_ATTACHED_DS18B20*EEPROM_DEVICE_CNT_STEP; i=i+EEPROM_DEVICE_CNT_STEP)
	{
		TempChar=loadState(i);
		if (TempChar == 0xFF)
		{
			return counter;
		}
		counter++;
	}
	return counter;//counter ist 1 größer als MAX_ATTACHED_DS18B20 und damit wird signalisiert, dass kein Speicher mehr frei ist.
}

void ClearEeprom()
{
	send(msgDebugReturnString.set(F("EEPROM cleared")));
	for (uint8_t i = EEPROM_DEVICE_TEMP_ID_START; i < EEPROM_DEVICE_TEMP_ID_START+MAX_ATTACHED_DS18B20*EEPROM_DEVICE_CNT_STEP; i++)
	// for (uint8_t i = EEPROM_DEVICE_TEMP_NAME_START; i < EEPROM_DEVICE_TEMP_ID_START+MAX_ATTACHED_DS18B20*EEPROM_DEVICE_CNT_STEP; i++)
	{
		Serial.print("Clearing Pos: "); 
		Serial.println(i);
		saveState(i,0xFF);
	}
	sprintf(cThermoKnown,"%s", "FFFFFFFF");	//Speicherstellen als F=Free markieren
}

void DestroyEepromSensor(uint8_t Sensor)
{
	DEBUG_PRINTLN(F("DestroyEeprom Sensor first Pos"));
	send(msgDebugReturnString.set(F("Destroy EEPROM Sensor")));
	uint8_t TempIDStart=(EEPROM_DEVICE_TEMP_ID_START + ((Sensor) * (EEPROM_DEVICE_ID_LENGTH + EEPROM_DEVICE_NAME_LENGTH )));
	saveState(TempIDStart,0xDD); // DD = Destroy Device
}

void ClearEepromSensor(uint8_t Sensor)
{
	DEBUG_PRINT(F("ClearEeprom Sensor Address and Name"));
	send(msgDebugReturnString.set(F("clear EEPROM Sensor")));
	uint8_t TempIDStart=(EEPROM_DEVICE_TEMP_ID_START + ((Sensor) * (EEPROM_DEVICE_ID_LENGTH + EEPROM_DEVICE_NAME_LENGTH )));
	
	for (uint8_t i = TempIDStart; i < (TempIDStart + (uint8_t)16); i++)
	{
		Serial.print("Clearing Pos: "); 
		Serial.println(i);
		saveState(i,0xFF);
	}
}

// void ClearDebug()
// {
	// for (int i = 0; i <= 7; i++)
	// {
		// Serial.print("Clearing Pos: "); 
		// Serial.println(i);
		// saveState(i,0x00);
	// }
// }

void receive(const MyMessage &message)
{
	DEBUG_PRINT(F("receive for Sensor: "));
	DEBUG_PRINTLN(message.sensor);
	
	if ((message.sensor >= CHILD_OW_TEMP_NAME) and (message.sensor <= CHILD_OW_TEMP_NAME+MAX_ATTACHED_DS18B20))
	{
		bool foundSensor=false;
		for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)//Speichern der Namen für die DS18B20 Devices
		{
			if (cThermoKnown[i] == 'C')//weitermachen, wenn C // "C" funktioniert nicht 'C' schon
			{
				if (message.sensor == CHILD_OW_TEMP_NAME+i)
				{
					switch (message.type) 
					{
						case V_TEXT: 
						{	
							SaveName(i,message.getString());
							loopCount=(uint8_t) HEARTBEAT_INTERVAL;
							foundSensor=true;
						}
					}
				}
			}
		}
		if (!foundSensor)
		{
			DEBUG_PRINT(F("Sensor not in List"));
		}
	}
	
	
	

//	0 		off
//	1		showEEpromHex()
//	2		scanne nach aktuell angeschlossenen OwDevices und präsentiere diese
//	3		wie 2, aber vorher ClearEeprom (OneWire Adresse freigeben)
//	4 		wie 3, aber mit anschließendem hwReboot()
//	5		saveState(EEPROM_DEVICE_TEMP_ID_START, 65);// OwID an Index 1 erste Stelle ungültig machen
//	6		myHeartBeatLoop
//	9		Reboot /Arduino hängt sich auf, wenn default Bootloader installiert ist. Optiboot V8 von 2018 funktioniert (https://github.com/Optiboot/optiboot/releases/tag/v8.0)
//	20-27	Hardware-Adresse im EEPROM von Sensor 0-7 ungültig machen (Sensor wird dann als D=Disconneted angezeigt)
//	30-37	Hardware-Adresse im EEPROM von Sensor 0-7 freigeben (0xFF setzen)

	if (message.sensor == CHILD_DEBUG_LEVEL)
	{ 
		switch (message.type) 
		{
			case V_TEXT: 
			{
				debugLevel = message.getByte();
				String debugString = message.getString();
				
				
				DEBUG_PRINT(F("rcv: got "));
				DEBUG_PRINTLN(debugLevel);
				// DEBUG_PRINT(F("Strg "));
				// DEBUG_PRINTLN(debugString);
				
				if (debugLevel == 0)
				{
					debugLevel=0;
					saveState(EEPROM_DEVICE_DEBUG_LEVEL, debugLevel);//8 Bit
					send(msgDebugReturnString.set(F("dbg0")));
				}
				
				if (debugLevel == 1)
				{
					debugLevel=0;
					#ifdef SER_DEBUG
						send(msgDebugReturnString.set(F("showEEpromHex")));
						showEEpromHex();
					#else
						send(msgDebugReturnString.set(F("dbg1 not possible")));
					#endif

				}
				if (debugLevel == 2)
				{
					debugLevel=0;
					send(msgDebugReturnString.set(F("doPresentation")));
					doPresentation=1;
					doReadTemp=1;
					loopCount=(uint8_t) HEARTBEAT_INTERVAL;
					#ifdef SER_DEBUG
						showEEpromHex();
					#endif
				}
				if (debugLevel == 3)
				{
					debugLevel=0;
					send(msgDebugReturnString.set(F("ResetOneWire")));
					ClearEeprom();
					doPresentation=1;
					doReadTemp=1;
					loopCount=(uint8_t) HEARTBEAT_INTERVAL;
					#ifdef SER_DEBUG
						showEEpromHex();
					#endif
				}
				if (debugLevel == 4)
				{
					debugLevel=0;
					ClearEeprom();
				
					saveState(EEPROM_DEVICE_DEBUG_LEVEL, debugLevel);//8 Bit
					send(msgDebugLevel.set(debugLevel));
					#ifdef SER_DEBUG
						showEEpromHex();
					#endif
					hwReboot();
				}
				if (debugLevel == 6)
				{
					debugLevel=0;
					loopCount=(uint8_t) HEARTBEAT_INTERVAL;
				}
				if (debugLevel == 9)
				{
					send(msgDebugReturnString.set(F("hwReboot")));
					hwReboot();
				}
				if ((debugLevel >= 20) and (debugLevel <= 27))	//Sensor ungültig machen
				{
					uint8_t Sensor = debugLevel - 20;
					DEBUG_PRINT(F("destroy Sensor "));
					DEBUG_PRINTLN(Sensor);
					DestroyEepromSensor(Sensor);
					doPresentation=1;
					doReadTemp=1;
					loopCount=(uint8_t) HEARTBEAT_INTERVAL;
				}
				if ((debugLevel >= 30) and (debugLevel <= 37))	//Speicher freigeben (z.B. weil Sensor defekt ist)
				{
					uint8_t Sensor = debugLevel - 30;
					DEBUG_PRINT(F("Free Sensor EEPROM "));
					DEBUG_PRINTLN(Sensor);
					ClearEepromSensor(Sensor);
					doPresentation=1;
					doReadTemp=1;
					loopCount=(uint8_t) HEARTBEAT_INTERVAL;
				}
			}
			break;

		}
	}
}



void LoadName(uint8_t deviceIndex)//lädt in die globale Variable cThermoName von deviceIndex+(0 bis 7)
{
	// DEBUG_PRINT("LoadName for DevIndex: ");
	// DEBUG_PRINTLN(String(sdeviceIndex,DEC));
	uint8_t EepromIndex=EEPROM_DEVICE_TEMP_NAME_START+deviceIndex*EEPROM_DEVICE_CNT_STEP;
	for (uint8_t i = 0; i<EEPROM_DEVICE_NAME_LENGTH; i++)
	{
		// debugMessage("EepromIndex: ",String(EepromIndex+i));
		cThermoName[i]=loadState(EepromIndex+i); // von i (deviceIndex+0 bis deviceIndex+7) nach 0-7 kopieren ; i-deviceIndex ergibt 0-7
		if (((uint8_t)cThermoName[i] == 0xFF) || ((uint8_t)cThermoName[i] == 0x00))
		{
			cThermoName[i]='-';
		}
		
		if (((uint8_t)cThermoName[i] >= (uint8_t)MIN_ASCII_CHAR ) and ((uint8_t)cThermoName[i] <= (uint8_t)MAX_ASCII_CHAR))
		{
			// DEBUG_PRINT("OK: ");
			// DEBUG_PRINTLN((char)cThermoName[i]);			
		}
		else
		{
			
			// DEBUG_PRINT("cThermoName was: ");
			// DEBUG_PRINTLN((char)cThermoName[i]);
			cThermoName[i]='-'; // 45="-"
			// DEBUG_PRINT("cThermoName replace: ");
			// DEBUG_PRINTLN((char)cThermoName[i]);		
		}
		
	}
	// DEBUG_PRINTLN(cThermoName);
}


void SaveName(uint8_t deviceIndex, String sName)
{
	send(msgDebugReturnString.set(F("SaveName")));
	// debugMessage("SaveName für ID: ",String(deviceIndex,DEC));
	uint8_t EepromIndex=EEPROM_DEVICE_TEMP_NAME_START+deviceIndex*EEPROM_DEVICE_CNT_STEP;
	// debugMessage("Freie Adresse an :",String(EepromIndex,DEC));
	for (uint8_t i = 0; i<EEPROM_DEVICE_NAME_LENGTH; i++)
	{
		// debugMessage("EepromIndex: ",String(EepromIndex+i));
		// debugMessage("sName    : ",sName);
		saveState((EepromIndex+i),sName[i]);
	}

}
