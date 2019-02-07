//	20180205	1.5.06	Zusammenführen von MYSOneWireMaster_V1.4 und MYSOneWireMaster_V1.5
//						Steuern ob Sensor mit oder ohne Batterie betrieben wird
//	20180216	1.6.01	Scannen mit OneWire Library und bekannte Adressen ins EEPROM schreiben
//						bei before() auslesen der Resolution von DS18B20 und dauerhaft auf neuen Wert setzen, als dieser Wert von OW_RESOLUTION abweicht

// ToDo für Version 1.7: per #defines einstellen ob Strom oder Batteriebetrieb (Strom immer 5V, Batterie immer 3,3V (zumindest bei onewire)


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
// #define MY_DEFAULT_TX_LED_PIN 				(8)
// #define MY_DEFAULT_LED_BLINK_PERIOD 		10

// ###################   Transport   #####################
/*
RF24_PA_MIN = 	-18dBm 		0	R_TX_Powerlevel_Pct 	25
RF24_PA_LOW = 	-12dBm 		1	R_TX_Powerlevel_Pct
RF24_PA_HIGH = 	-6dBm 		2	R_TX_Powerlevel_Pct
RF24_PA_MAX = 	 0dBm		3	R_TX_Powerlevel_Pct
*/

#define MY_RF24_PA_LEVEL 					RF24_PA_MIN
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 					96
// #define MY_TRANSPORT_WAIT_READY_MS 			(5000ul)

#define MY_NODE_ID 							150
// #define MY_PARENT_NODE_ID 					50
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_PASSIVE_NODE


// ###################   Node Spezifisch   #####################
#define SKETCH_VER            				"1.7-001"        			// Sketch version
#define SKETCH_NAME           				"OneWireMaster"   		// Optional child sensor name

#define HEARTBEAT_INTERVAL        			600000        //später alle 5 Minuten, zum Test alle 30 Sekunden


#define ONE_WIRE_BUS 4 // Pin where dallas sensor is connected 


																	//Batterie ohne Spannungsteiler direkt an A0 anschließen und analogReference(DEFAULT);(VREF bringt hier immer den gleichen Wert)
// #define CHILD_ID_BAT_VREF       			1         					//ID für Batterie Werte Intern per VRef --> Bei Betrieb mit 2-3 Batterien an Arduino mit abgelöteten Spannungsregler
// #define CHILD_ID_DEBUG          			2							// Debug setzen und lesen

#define OW_RESOLUTION						10
#define SLEEP_TIME							60000
#define MAX_ATTACHED_DS18B20      			8							//mehr als 8 funktioniert nicht mit den vorhandenen Methoden

#define EEPROM_DEVICE_NAME_LENGTH   		8
#define EEPROM_DEVICE_ID_LENGTH     		8
#define EEPROM_DEVICE_CNT_STEP				(EEPROM_DEVICE_NAME_LENGTH+EEPROM_DEVICE_ID_LENGTH) //Klammer könnte evtl. weg
//Werte für 
#define EEPROM_DEVICE_DEBUG_Level   		0     // DebugLevel, die nächsten 7 uint8_ts sind für Debug reserviert
#define EEPROM_DEVICE_DEBUG_7 				7

#define EEPROM_DEVICE_TEMP_ID_START			EEPROM_DEVICE_DEBUG_7+1
#define EEPROM_DEVICE_TEMP_NAME_START		EEPROM_DEVICE_TEMP_ID_START+EEPROM_DEVICE_ID_LENGTH

#define MAX_DEBUG_LEVEL         			4

#define	MIN_ASCII_CHAR						32
#define MAX_ASCII_CHAR						128

#define	WITH_BATTERIE




#include <MySensors.h>
#include <DallasTemperature.h>	// https://github.com/milesburton/Arduino-Temperature-Control-Library Version 3.8.0
#include <OneWire.h>			// https://www.pjrc.com/teensy/td_libs_OneWire.html Version 2.3.4
#include "C:\_Lokale_Daten_ungesichert\Arduino\MySensors\CommonFunctions.h" //muss nach allen anderen #defines stehen


OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
DeviceAddress tempDeviceAddress; 		//typedef uint8_t DeviceAddress[8];


char cThermoName[]="-8Name8-";
char bThermoKnown[]="NNNNNNNN";

uint8_t debugLevel = 0; 
// debugLevel switches
// 0 	off
// 1
// 2
// 3	scanne nach aktuell angeschlossenen OwDevices und gebe für die restlichen den Speicher im EEPROM frei
// 4 	clear all EEprom, switsche danach automatisch zurück auf debugLevel=0


void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	delay(18); // bei weniger als 18ms kommt soetwas hier: p⸮Y⸮%⸮⸮⸮⸮ 
	DEBUG_PRINTLN("preHwInit");
}

void before()
{
	DEBUG_PRINTLN("before");

	// Startup up the OneWire library
	sensors.begin();
	vRef.begin();
	
	showEEpromHex();
	showEEpromChar();
	// ClearDebug();
	// ClearEeprom();
	sortOwAdresses();//prüft das EEPROM, welche TempAddress für OW Geräte bekannt sind und waren
	checkResolution();
	// saveState(EEPROM_DEVICE_TEMP_ID_START, 65);// OwID an Index 1 erste Stelle ungültig machen

}


void setup()
{
	DEBUG_PRINTLN("Setup");
	sensors.setWaitForConversion(false);//damit mysensors nicht blockiert
	debugLevel = loadState(EEPROM_DEVICE_DEBUG_Level); //8 Bit
	if (debugLevel>MAX_DEBUG_LEVEL)
	{
		debugLevel=0;
		// DEBUG_PRINTLN(F("Save: debugLevel to 0"));
		saveState(EEPROM_DEVICE_DEBUG_Level, debugLevel);//8 Bit
	}
	// DEBUG_PRINT(F("Debug level fetched from EEPROM "));
	// DEBUG_PRINTLN(debugLevel);
}

void presentation()
{
	DEBUG_PRINTLN("presentation...");
	mySendSketchInfo();
	myPresentation();
	present(CHILD_OW_CONNECTED, 	S_INFO, 			CHILD_OW_CONNECTED_TEXT);
	// present(CHILD_ID_DEBUG, S_CUSTOM);
	for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)
	{
		if (bThermoKnown[i] == 'J')//weitermachen, wenn 1
		{
			present(CHILD_OW_TEMP+i, S_TEMP);
			present(CHILD_OW_TEMP_NAME+i, S_INFO);
			
		}
	}
}

void loop()//ToDo: getResolution je Sensor-Adresse, wenn anders als festgeletzte Auflösung, dann setResolution (dies wird dauerhaft im EEPROM vom OneWire Gerät gespeichert)
{
	sendHeartbeat();  

	sortOwAdresses();

	sensors.requestTemperatures();
	//int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
	wait(750);//conversionTime liefert auch 750 ms zurück
	// wait(750);//conversionTime liefert auch 750 ms zurück



	for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)
	{
		if (bThermoKnown[i] == 'J')//weitermachen, wenn 1
		{
			for (uint8_t j=0;j<EEPROM_DEVICE_ID_LENGTH;j++)
			{
				tempDeviceAddress[j]=loadState(EEPROM_DEVICE_TEMP_ID_START+(i*EEPROM_DEVICE_CNT_STEP)+j);
			}
			float temperature = sensors.getTempC(tempDeviceAddress);
			// debugMessage("loop: gotTempC: ",String(temperature,DEC));
			if (temperature != -127.00 && temperature != 85.00) 
			{
				send(MsgOwTemp.setSensor(CHILD_OW_TEMP+i).setType(V_TEMP).set(temperature,1));//Temp mit einer Nachkommastelle senden
				send(MsgOwTemp.setSensor(CHILD_OW_TEMP+i).setType(V_ID).set(tempDeviceAddress,8));//Temp mit einer Nachkommastelle senden
				// send(msgOwID.setSensor(CHILD_OW_TEMP+i).set(tempDeviceAddress,8));
				LoadName(i);//Load from EEPROM to volatile char cThermoName
				// send(MsgOwName.setSensor(CHILD_OW_TEMP+i).set(ArrayToChar8(cThermoName)));
				DEBUG_PRINT("OW Name: ");
				DEBUG_PRINTLN(cThermoName);
				send(MsgOwName.setSensor(CHILD_OW_TEMP_NAME+i).set(cThermoName));
			}
		}
	}


	send(MsgDebugLevel.set(debugLevel));
	
	send(MsgDebugOWConList.set(bThermoKnown));
	// send(MsgDebugOWConList.set(ArrayToChar8(bThermoKnown)));
	

	

	BatteryVRef();
	
	// smartSleep((uint32_t)SLEEP_TIME);
	wait((uint32_t)SLEEP_TIME);

}


void checkResolution()//schreibt die Temperatur Auflösung ins EEPROM des DS18B20. Die Schreibzyklen sind begrenzt. Es reicht, wenn diese Routine im Setup aufgerufen wird.
{
	DEBUG_PRINTLN("checkResolution");
	for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)
	{
		if (bThermoKnown[i] == 'J')//weitermachen, wenn 1
		{
			for (uint8_t j=0;j<EEPROM_DEVICE_ID_LENGTH;j++)
			{
				tempDeviceAddress[j]=loadState(EEPROM_DEVICE_TEMP_ID_START+(i*EEPROM_DEVICE_CNT_STEP)+j);
			}
			// String strAddr=DevAddressToHex(tempDeviceAddress);
			// debugMessage("Checking Resolution for OwID: ",strAddr);
			uint8_t resolution = sensors.getResolution(tempDeviceAddress);
			if (resolution != OW_RESOLUTION) {
				sensors.setResolution(tempDeviceAddress,OW_RESOLUTION,false);
				DEBUG_PRINT("Changing Resolution to: ");
				DEBUG_PRINTLN(String(OW_RESOLUTION,DEC));
				wait(100);
			}
			else
			{
				// debugMessage("Resolution was OK: ",String(resolution,DEC));
			}
	
		}	
	}
}


void sortOwAdresses()
{
	oneWire.reset_search();
	uint8_t Index=0;
	uint8_t DeviceCounter=0;
	uint8_t nextFreeAddress=0;

	DEBUG_PRINTLN("Looking for OW Devices");
	while (oneWire.search(tempDeviceAddress))
	{
		DEBUG_PRINT("Found OwID: ");
		DEBUG_PRINTLN(DevAddressToHex(tempDeviceAddress));
		// Serial.println(String(*tempDeviceAddress));
		// DEBUG_PRINT("DeviceCounter: ");
		// DEBUG_PRINTLN(DeviceCounter);	
		if (DeviceCounter < MAX_ATTACHED_DS18B20)
		{
			Index=getTempDevAddressIndex(tempDeviceAddress);
			if (Index < MAX_ATTACHED_DS18B20)
			{
				
				bThermoKnown[Index]='J';
			}
			else//owID wurde im EEPROM nicht gefunden und sollte deshalb gespeichert werden
			{
				nextFreeAddress=getNextFreeEepromAddress();
				if (nextFreeAddress < MAX_ATTACHED_DS18B20)
				{
					send(MsgDebugReturnString.set(F("Neues OW Device")));
					SaveOwID(nextFreeAddress,tempDeviceAddress);
					bThermoKnown[nextFreeAddress]='J';
				}
				else
				{
					bThermoKnown[nextFreeAddress]='N';
					send(MsgDebugReturnString.set("F(Speicher EEPROM ist voll"));
				}
			}
		}
		else
		{
			DEBUG_PRINTLN("OneWire Bus ist ueberfuellt");
		}
		DeviceCounter++;
	}
	send(MsgDebugOWDevCount.set(DeviceCounter)); //Update controller with number of found devices
}


void SaveOwID(uint8_t nextFreeAddress, DeviceAddress tempDeviceAddress)
{
	send(MsgDebugReturnString.set("SaveOwID"));
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
		
		if (counter == 8)//alle 8 Blöcke waren gliech
		{

			return IndexCnt;
		}
		IndexCnt++;
	}
	return IndexCnt;//counter ist 1 größer als MAX_ATTACHED_DS18B20 und damit wird signalisiert, dass kein Speicher mehr frei ist. 
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
	// debugMessage("ClearEeprom", "");
	send(MsgDebugReturnString.set("EEPROM cleared"));
	for (uint8_t i = EEPROM_DEVICE_TEMP_ID_START; i < EEPROM_DEVICE_TEMP_ID_START+MAX_ATTACHED_DS18B20*EEPROM_DEVICE_CNT_STEP; i++)
	{
		Serial.print("Clearing Pos: "); 
		Serial.println(i);
		saveState(i,0xFF);
	}
}

void ClearDebug()
{
	// debugMessage("ClearEepromDebug", "");
	for (int i = 0; i <= 7; i++)
	{
		Serial.print("Clearing Pos: "); 
		Serial.println(i);
		saveState(i,0x00);
	}
}

void receive(const MyMessage &message)
{
	// debugMessage("receive: ", String(message.sensor));
	for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)//Speichern der Namen für die DS18B20 Devices
	{
		if (bThermoKnown[i] == 'J')//weitermachen, wenn J // "J" funktioniert nicht 'J' schon
		{
			if (message.sensor == CHILD_OW_TEMP_NAME+i)
			{
				switch (message.type) 
				{
					case V_TEXT: 
					{	
						// charAddr8 = message.getString();
						// debugMessage("got V_VAR1: ", String(charAddr8));
						// SaveName(i,String(charAddr8));
						SaveName(i,message.getString());
						send(MsgDebugLevel.set(debugLevel));
					}
				}
			}
		}
	}
	
	


	if (message.sensor == CHILD_DEBUG_LEVEL)
	{ 
		switch (message.type) 
		{
			case V_TEXT: 
			{
				// debugLevel = message.getULong();
				debugLevel = message.getByte();
				if (debugLevel == 4)
				{
					ClearEeprom();
					debugLevel=0;
				}
				saveState(EEPROM_DEVICE_DEBUG_Level, debugLevel);//8 Bit
				#ifdef SER_DEBUG
				showEEpromHex();
				#endif
			}
			break;
			// case V_VAR2: 
			// {

			// }
			// break;
			// case V_VAR3: 
			// {

			// }
			// break;
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
		if (cThermoName[i] < (char)MIN_ASCII_CHAR || cThermoName[i] > (char)MAX_ASCII_CHAR)//nur druckbare ASCII Zeichen zulassen
		{
			// DEBUG_PRINT("cThermoName was: ");
			// DEBUG_PRINTLN((char)cThermoName[i]);
			cThermoName[i]='-'; // 45="-"
			// DEBUG_PRINT("cThermoName replace: ");
			// DEBUG_PRINTLN((char)cThermoName[i]);
		}
		
	}
	// DEBUG_PRINTLN(String(cThermoName));
}


void SaveName(uint8_t deviceIndex, String sName)
{
	send(MsgDebugReturnString.set("SaveName"));
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


