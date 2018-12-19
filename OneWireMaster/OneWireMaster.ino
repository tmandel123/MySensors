//	20180205	1.5.06	Zusammenführen von MYSOneWireMaster_V1.4 und MYSOneWireMaster_V1.5
//						Steuern ob Sensor mit oder ohne Batterie betrieben wird
//	20180216	1.6.01	Scannen mit OneWire Library und bekannte Adressen ins EEPROM schreiben
//						bei before() auslesen der Resolution von DS18B20 und dauerhaft auf neuen Wert setzen, als dieser Wert von OW_RESOLUTION abweicht

// ToDo für Version 1.6: per #defines einstellen ob Strom oder Batteriebetrieb (Strom immer 5V, Batterie immer 3,3V (zumindest bei onewire)

/*

C:\_Lokale_Daten_ungesichert\Arduino\MySensors\OneWireMaster\OneWireMaster.ino:133:18: warning: ISO C++ forbids converting a string constant to 'char*' [-Wwrite-strings]

 char* charAddr = "012345678901234567890123";

                  ^

C:\_Lokale_Daten_ungesichert\Arduino\MySensors\OneWireMaster\OneWireMaster.ino:134:19: warning: ISO C++ forbids converting a string constant to 'char*' [-Wwrite-strings]

 char* charAddr8 = "01234567";

                   ^

C:\_Lokale_Daten_ungesichert\Arduino\MySensors\OneWireMaster\OneWireMaster.ino: In function 'void loop()':

C:\_Lokale_Daten_ungesichert\Arduino\MySensors\OneWireMaster\OneWireMaster.ino:263:75: warning: invalid conversion from 'volatile char*' to 'uint8_t* {aka unsigned char*}' [-fpermissive]

     send(msgOwName.setSensor(CHILD_ID_Temp+i).set(ArrayToChar8(cThermoName)));

                                                                           ^

C:\_Lokale_Daten_ungesichert\Arduino\MySensors\OneWireMaster\OneWireMaster.ino:440:8: note:   initializing argument 1 of 'char* ArrayToChar8(uint8_t*)'

 char* ArrayToChar8(uint8_t* data)

        ^

C:\_Lokale_Daten_ungesichert\Arduino\MySensors\OneWireMaster\OneWireMaster.ino:271:56: warning: invalid conversion from 'volatile char*' to 'uint8_t* {aka unsigned char*}' [-fpermissive]

  send(msgDebugThermoKnown.set(ArrayToChar8(bThermoKnown)));

                                                        ^

C:\_Lokale_Daten_ungesichert\Arduino\MySensors\OneWireMaster\OneWireMaster.ino:440:8: note:   initializing argument 1 of 'char* ArrayToChar8(uint8_t*)'

 char* ArrayToChar8(uint8_t* data)

        ^

C:\_Lokale_Daten_ungesichert\Arduino\MySensors\OneWireMaster\OneWireMaster.ino:273:64: warning: invalid conversion from 'volatile char*' to 'uint8_t* {aka unsigned char*}' [-fpermissive]

  debugMessage("bThermoKnown: ",String(ArrayToChar8(bThermoKnown)));

                                                                ^

C:\_Lokale_Daten_ungesichert\Arduino\MySensors\OneWireMaster\OneWireMaster.ino:440:8: note:   initializing argument 1 of 'char* ArrayToChar8(uint8_t*)'

 char* ArrayToChar8(uint8_t* data)

        ^


*/

// Enable debug prints to serial monitor
#define MY_DEBUG
#define SER_DEBUG

#define MY_RF24_CHANNEL 96									// Für Testphase deaktivieren, damit Kanal 76 aktiv wird (Prod=96 Test=76)
#define MY_RADIO_RF24
//#define MY_SMART_SLEEP_WAIT_DURATION (1000ul)
#define MY_NODE_ID 150
#define MY_PARENT_NODE_ID 51
// #define MY_PARENT_NODE_ID 0
// #define MY_PARENT_NODE_IS_STATIC
// #define MY_SPLASH_SCREEN_DISABLED
#define MY_TRANSPORT_WAIT_READY_MS (5000ul)

#include <SPI.h>
#include <MySensors.h>
#include <DallasTemperature.h>	// https://github.com/milesburton/Arduino-Temperature-Control-Library Version 3.8.0
#include <OneWire.h>			// https://www.pjrc.com/teensy/td_libs_OneWire.html Version 2.3.4
#include <VoltageReference.h>	// https://github.com/rlogiacco/VoltageReference Version 1.2.2


#define ONE_WIRE_BUS 4 // Pin where dallas sensor is connected 

#define BATTERY_SENSE_PIN A0

// Node and sketch information
#define SKETCH_VER            			"1.6-002"        			// Sketch version
#define SKETCH_NAME           			"OneWireMasterBat"   		// Optional child sensor name
#define CHILD_ID_BAT_ANLG       		0							//ID für Batterie Werte an A0 --> Wird angewendet, wenn 3,3V StepUp Regler am Arduino angeschlossen ist. 
																	//Batterie ohne Spannungsteiler direkt an A0 anschließen und analogReference(DEFAULT);(VREF bringt hier immer den gleichen Wert)
#define CHILD_ID_BAT_VREF       		1         					//ID für Batterie Werte Intern per VRef --> Bei Betrieb mit 2-3 Batterien an Arduino mit abgelöteten Spannungsregler
#define CHILD_ID_DEBUG          		2							// Debug setzen und lesen
#define CHILD_ID_Temp         			3         					// erste ID für Temperatur Werte
#define OW_RESOLUTION					10
#define SLEEP_TIME						60000
#define MAX_ATTACHED_DS18B20      		8							//mehr als 8 funktioniert nicht mit den vorhandenen Methoden

#define EEPROM_DEVICE_NAME_LENGTH   	8
#define EEPROM_DEVICE_ID_LENGTH     	8
#define EEPROM_DEVICE_CNT_STEP			(EEPROM_DEVICE_NAME_LENGTH+EEPROM_DEVICE_ID_LENGTH) //Klammer könnte evtl. weg
//Werte für 
#define EEPROM_DEVICE_DEBUG_Level   	0     // DebugLevel, die nächsten 7 uint8_ts sind für Debug reserviert
#define EEPROM_DEVICE_DEBUG_7 			7

#define EEPROM_DEVICE_TEMP_ID_START		EEPROM_DEVICE_DEBUG_7+1
#define EEPROM_DEVICE_TEMP_NAME_START	EEPROM_DEVICE_TEMP_ID_START+EEPROM_DEVICE_ID_LENGTH

#define MAX_DEBUG_LEVEL         		4

#define	WITH_BATTERIE
// #define	BAT_VREF_MAX_VOLTATE			3000
#define	BAT_VREF_MAX_VOLTATE			5000
// #define	BAT_VREF_MIN_VOLTATE			1800
#define	BAT_VREF_MIN_VOLTATE			2600

#define BAT_MESSURED					4980
#define BAT_VREF_CORRECTION_VALUE		(float)BAT_VREF_MAX_VOLTATE/(float)BAT_MESSURED


#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif



OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
DeviceAddress tempDeviceAddress; 		//typedef uint8_t DeviceAddress[8];
VoltageReference vRef;

// char cThermoName[EEPROM_DEVICE_NAME_LENGTH]={'-','8','N','a','m','e','8','-'};
char cThermoName[EEPROM_DEVICE_NAME_LENGTH];
// volatile char bThermoKnown[MAX_ATTACHED_DS18B20]={'N','N','N','N','N','N','N','N'};
char bThermoKnown[MAX_ATTACHED_DS18B20];

uint16_t batteryBatAnalogValue = 0;
const uint16_t BAT_ANALOG_MIN=600;
const uint16_t BAT_ANALOG_MAX=1023;


uint8_t numberOfDevices=0; // Number of temperature devices found
// unsigned long lastHeartBeat = 0;
// uint32_t SLEEP_TIME = 20000;  // sleep time between reads (seconds * 1000 milliseconds) : default: 600000
// char* charAddr = "012345678901234567890123";
// char* charAddr8 = "01234567";




uint8_t debugLevel = 0; 
// debugLevel switches
// 0 	off
// 1
// 2
// 3	scanne nach aktuell angeschlossenen OwDevices und gebe für die restlichen den Speicher im EEPROM frei
// 4 	clear all EEprom, switsche danach automatisch zurück auf debugLevel=0


MyMessage BatAnalogValue		(CHILD_ID_BAT_ANLG,		V_VOLTAGE);
MyMessage BatvRefValue			(CHILD_ID_BAT_VREF,		V_VOLTAGE);

MyMessage msgDebugLevel     	(CHILD_ID_DEBUG, 		V_VAR1);
MyMessage msgDebugDevCount    	(CHILD_ID_DEBUG, 		V_VAR2);
MyMessage msgDebugThermoKnown   (CHILD_ID_DEBUG, 		V_VAR3);
//MyMessage msgDebugThermoKnown   (CHILD_ID_DEBUG, 		V_VAR4);
MyMessage msgDebugReturnString  (CHILD_ID_DEBUG, 		V_VAR5);

MyMessage msgOwTemp				(CHILD_ID_Temp,  		V_TEMP);
MyMessage msgOwID				(CHILD_ID_Temp,  		V_ID);
MyMessage msgOwName				(CHILD_ID_Temp,  		V_VAR1);


void preHwInit() 
{
	DEBUG_SERIAL(115200);
	DEBUG_PRINTLN("preHwInit: ");
}

void before()
{
	DEBUG_PRINTLN("before....");
	DEBUG_PRINT("Version: ");
	DEBUG_PRINTLN(SKETCH_VER);
	analogReference(INTERNAL);

	wait(100);//Warten bis sich alles eingeschwungen hat
	batteryBatAnalogValue = analogRead(BATTERY_SENSE_PIN);
	wait(100);
	batteryBatAnalogValue = analogRead(BATTERY_SENSE_PIN);
	
	// Startup up the OneWire library
	sensors.begin();
	vRef.begin();
	
	showEEprom();
	// ClearDebug();
	// ClearEeprom();
	sortOwAdresses();//prüft das EEPROM, welche TempAddress für OW Geräte bekannt sind und waren
	checkResolution();
	// saveState(24, 65);// OwID an Index 1 erste Stelle ungültig machen

}


void setup()
{
	DEBUG_PRINTLN("Setup...");
	sensors.setWaitForConversion(false);//damit mysensors nicht blockiert
	debugLevel = loadState(EEPROM_DEVICE_DEBUG_Level); //8 Bit
	if (debugLevel>MAX_DEBUG_LEVEL)
	{
		debugLevel=0;
		DEBUG_PRINTLN(F("Save: debugLevel to 0"));
		saveState(EEPROM_DEVICE_DEBUG_Level, debugLevel);//8 Bit
	}
	DEBUG_PRINT(F("Debug level fetched from EEPROM "));
	DEBUG_PRINTLN(debugLevel);
}

void presentation()
{
	DEBUG_PRINTLN("presentation...");
	sendSketchInfo(SKETCH_NAME, SKETCH_VER);  
	present(CHILD_ID_BAT_ANLG, S_MULTIMETER);
	present(CHILD_ID_BAT_VREF, S_MULTIMETER);
	present(CHILD_ID_DEBUG, S_CUSTOM);
	for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)
	{
		if (bThermoKnown[i] == 'J')//weitermachen, wenn 1
		{
			present(CHILD_ID_Temp+i, S_TEMP);
		}
	}
}

void loop()//ToDo: getResolution je Sensor-Adresse, wenn anders als festgeletzte Auflösung, dann setResolution (dies wird dauerhaft im EEPROM vom OneWire Gerät gespeichert)
{
	DEBUG_PRINTLN("");
	DEBUG_PRINTLN(" - NEW LOOP -");
	DEBUG_PRINTLN("");


	// String tempString;
	
	sendHeartbeat();  

	sortOwAdresses();

	sensors.requestTemperatures();
	//int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
	// smartSleep(750);//conversionTime liefert auch 750 ms zurück
	wait(750);//conversionTime liefert auch 750 ms zurück



	for (uint8_t i = 0; i < MAX_ATTACHED_DS18B20; i++)
	{
		// debugMessage("loop for bThermoKnown i: ",String(i,DEC));
		if (bThermoKnown[i] == 'J')//weitermachen, wenn 1
		{
			// debugMessage("loop for bThermoKnown Index OK: ",String(i,DEC));
			for (uint8_t j=0;j<EEPROM_DEVICE_ID_LENGTH;j++)
			{
				// debugMessage("building tempDeviceAddress",String(j,DEC));
				tempDeviceAddress[j]=loadState(EEPROM_DEVICE_TEMP_ID_START+(i*EEPROM_DEVICE_CNT_STEP)+j);
			}
			// String strAddr=addrToStr(tempDeviceAddress);
			// debugMessage("loop: getTempC: ",strAddr);
			// strAddr=addrToStr(cThermoName);
			// debugMessage("loop: Name: ",strAddr);
			float temperature = sensors.getTempC(tempDeviceAddress);
			// debugMessage("loop: gotTempC: ",String(temperature,DEC));
			if (temperature != -127.00 && temperature != 85.00) 
			{
				send(msgOwTemp.setSensor(CHILD_ID_Temp+i).set(temperature,1));//Temp mit einer Nachkommastelle senden
				send(msgOwID.setSensor(CHILD_ID_Temp+i).set(tempDeviceAddress,8));
				LoadName(i);//Load from EEPROM to volatile char cThermoName
				// send(msgOwName.setSensor(CHILD_ID_Temp+i).set(ArrayToChar8(cThermoName)));
				DEBUG_PRINT("Ow Name: ");
				DEBUG_PRINTLN(cThermoName);
				send(msgOwName.setSensor(CHILD_ID_Temp+i).set(cThermoName));
			}
		}	
	}


	send(msgDebugLevel.set(debugLevel));
	send(msgDebugDevCount.set(numberOfDevices));
	send(msgDebugThermoKnown.set(bThermoKnown));
	// send(msgDebugThermoKnown.set(ArrayToChar8(bThermoKnown)));
	
	// debugMessage("bThermoKnown: ",String(ArrayToChar8(bThermoKnown)));
	
	int vcc = vRef.readVcc(); //5000 oder 3000 mA
	// debugMessage("vRef: ",String(vcc,DEC));
	// Serial.print("vRef: ");
	// Serial.println(vcc);	
	
	float correctionValue=BAT_VREF_CORRECTION_VALUE; //Soll / Ist = 1,034 //5000 oder 3000 mA
	// debugMessage("correctionValue: ",String(correctionValue,DEC));
	// Serial.print("correctionValue: ");
	// Serial.println(correctionValue);	
	
	float vccCorrect = (vcc * correctionValue);
	// debugMessage("vcc korrigiert: ",String(vccCorrect,DEC));
	// Serial.print("vcc korrigiert: ");
	// Serial.println(vccCorrect);
	
	int batteryPcntVcc = 0;
	float batVoltage = 0;

	vccCorrect=constrain(vccCorrect, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE);
	// debugMessage("constrain: ",String(vccCorrect,DEC));
	// Serial.print("constrain: ");
	// Serial.println(vccCorrect);

	
	batteryPcntVcc = map(vccCorrect, BAT_VREF_MIN_VOLTATE, BAT_VREF_MAX_VOLTATE, 0, 100); 
	// debugMessage("batteryPcntVcc: ",String(batteryPcntVcc,DEC));
	// Serial.print("batteryPcntVcc: ");
	// Serial.println(batteryPcntVcc);

	batVoltage  = vccCorrect / 1000;
	// debugMessage("batVoltage: ",String(batVoltage,DEC));
	
	// Serial.print("batVoltage: ");
	// Serial.println(batVoltage);
		
	sendBatteryLevel(batteryPcntVcc);
	send(BatvRefValue.set(batVoltage,3));
	
	
	
	
	
	
	
	
	
	
	int batteryPcnt = 0;
	float batteryVoltage = 0;
	// get the battery Voltage
	batteryBatAnalogValue = analogRead(BATTERY_SENSE_PIN);
	batteryBatAnalogValue=constrain(batteryBatAnalogValue, BAT_ANALOG_MIN, BAT_ANALOG_MAX);
	batteryPcnt = map(batteryBatAnalogValue, BAT_ANALOG_MIN, BAT_ANALOG_MAX, 0, 100); //zwischen 100% und 60% bewegt sich die Nutzungsmöglichkeit der Batterie. Darunter bricht die Spannung zusammen
	batteryVoltage  = batteryBatAnalogValue * 0.004594330400782;
	sendBatteryLevel(batteryPcnt);
	send(BatAnalogValue.set(batteryVoltage,3));
	// smartSleep(SLEEP_TIME);
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
			// String strAddr=addrToStr(tempDeviceAddress);
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

	DEBUG_PRINTLN("Looking for Ow Devices");
	while (oneWire.search(tempDeviceAddress))
	{
		DEBUG_PRINT("Found OwID: ");
		DEBUG_PRINTLN(addrToStr(tempDeviceAddress));
		DEBUG_PRINT("DeviceCounter: ");
		DEBUG_PRINTLN(DeviceCounter);	
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
					send(msgDebugReturnString.set(F("Neues OW Device")));
					SaveOwID(nextFreeAddress,tempDeviceAddress);
					bThermoKnown[nextFreeAddress]='J';
				}
				else
				{
					bThermoKnown[nextFreeAddress]='N';
					send(msgDebugReturnString.set("F(Speicher EEPROM ist voll"));
					// debugMessage("Speicher EEPROM ist voll","");
					// debugMessage("nextFreeAddress: ",String(nextFreeAddress,HEX));
					// debugMessage("DeviceCounter: ",String(DeviceCounter,DEC));
					// debugMessage("tempDeviceAddress: ",addrToChar(tempDeviceAddress));
				}
			}
		}
		else
		{
			DEBUG_PRINTLN("OneWire Bus ist ueberfuellt");
		}
		DeviceCounter++;
	}
	
	// debugMessage("NumberOfDevices: ",String(numberOfDevices,DEC));
	// numberOfDevices = sensors.getDeviceCount();
	numberOfDevices = DeviceCounter;
}


void SaveOwID(uint8_t nextFreeAddress, DeviceAddress tempDeviceAddress)
{
	send(msgDebugReturnString.set("SaveOwID"));
	// debugMessage("SaveOwID","");
	uint8_t tempAddressIndex=EEPROM_DEVICE_TEMP_ID_START+nextFreeAddress*EEPROM_DEVICE_CNT_STEP;
	// debugMessage("Freie Adresse an :",String(tempAddressIndex,DEC));
	for (uint8_t j = 0; j<EEPROM_DEVICE_ID_LENGTH; j++)
	{
		// debugMessage("nextFreeAddress: ",String(tempAddressIndex+j));
		// Serial.print("Save ID: ");
		// Serial.print(String(tempDeviceAddress[j],HEX));
		// Serial.print(" Address: ");
		// Serial.println(String((tempAddressIndex+j),DEC));
		saveState((tempAddressIndex+j),tempDeviceAddress[j]);
	}

}

// char* ArrayToChar8(uint8_t* data)
// {
	// String strAddr;
	// for (uint8_t i = 0; i < 8; i++) {
		// strAddr = strAddr + char(data[i]);
	// }
	// for (int j = 0; j < 8; j++) {
	// charAddr8[j] = strAddr[j];
	// }
	// return charAddr8;
// }

// char* addrToChar(uint8_t* data) {
  // String strAddr = String(data[0], HEX); //Chip Version; should be higher than 16
  // strAddr = strAddr + "-";
  // for (uint8_t i = 1; i < 8; i++) {
    // if (data[i] < 16) strAddr = strAddr + 0;
    // strAddr = strAddr + String(data[i], HEX) + "-";
    // strAddr.toUpperCase();
  // }
  // for (int j = 0; j < 24; j++) {
    // charAddr[j] = strAddr[j];
  // }
  // return charAddr;
// }

String addrToStr(uint8_t* data) //Data ist 8 Zeichen Char Array
{
	String strAddr;
	for (uint8_t i = 0; i < 8; i++) 
	{
		if (data[i] < 16)
		{
			strAddr = strAddr + 0;
		}
		
		strAddr = strAddr + String(data[i], HEX);
		
		// if (i < 7)
		// {
			// strAddr = strAddr + "-";
		// }
	}
	strAddr.toUpperCase();
	return strAddr;
}

// String cThermoNameToStr(uint8_t* data) //Data ist 8 Zeichen Char Array
// {
	// String strAddr;
	// for (uint8_t i = 0; i < EEPROM_DEVICE_NAME_LENGTH; i++) 
	// {
		// strAddr = strAddr + (char)data[i];
	// }
	// return strAddr;
// }

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
				// Serial.println("");
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
	// debugMessage("getNextFreeEepromAddress","");
	uint8_t TempChar;
	uint8_t counter=0;
	for (uint8_t i = EEPROM_DEVICE_TEMP_ID_START; i < EEPROM_DEVICE_TEMP_ID_START+MAX_ATTACHED_DS18B20*EEPROM_DEVICE_CNT_STEP; i=i+EEPROM_DEVICE_CNT_STEP)
	// for (uint8_t i = EEPROM_DEVICE_TEMP_ID_START; i < EEPROM_DEVICE_TEMP_ID_START+MAX_ATTACHED_DS18B20*2*EEPROM_DEVICE_NAME_LENGTH; i=i+2*EEPROM_DEVICE_NAME_LENGTH)
	{
		TempChar=loadState(i);
		// debugMessage("TempChar: ",String(TempChar,HEX));
		// debugMessage("i: ",String(i,DEC));
		if (TempChar == 0xFF)
		{
			return counter;
		}
		counter++;
	}
	return counter;//counter ist 1 größer als MAX_ATTACHED_DS18B20 und damit wird signalisiert, dass kein Speicher mehr frei ist.
}

void showEEprom()
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


void ClearEeprom()
{
	// debugMessage("ClearEeprom", "");
	send(msgDebugReturnString.set("EEPROM cleared"));
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
			if (message.sensor == CHILD_ID_Temp+i)
			{
				switch (message.type) 
				{
					case V_VAR1: 
					{	
						// charAddr8 = message.getString();
						// debugMessage("got V_VAR1: ", String(charAddr8));
						// SaveName(i,String(charAddr8));
						SaveName(i,message.getString());
					}
				}
			}
		}
	}
	
	


	if (message.sensor == CHILD_ID_DEBUG)
	{ 
		switch (message.type) 
		{
			case V_VAR1: 
			{
				// debugLevel = message.getULong();
				debugLevel = message.getByte();
				if (debugLevel == 4)
				{
					ClearEeprom();
					debugLevel=0;
				}
				saveState(EEPROM_DEVICE_DEBUG_Level, debugLevel);//8 Bit
			}
			break;
			case V_VAR2: 
			{

			}
			break;
			case V_VAR3: 
			{

			}
			break;
		}
	}
}



void LoadName(uint8_t deviceIndex)//lädt in die globale Variable cThermoName von deviceIndex+(0 bis 7)
{
	
	// debugMessage("LoadName für deviceIndex:", String(deviceIndex,DEC));
	uint8_t EepromIndex=EEPROM_DEVICE_TEMP_NAME_START+deviceIndex*EEPROM_DEVICE_CNT_STEP;
	for (uint8_t i = 0; i<EEPROM_DEVICE_NAME_LENGTH; i++)
	{
		// debugMessage("EepromIndex: ",String(EepromIndex+i));
		cThermoName[i]=loadState(EepromIndex+i); // von i (deviceIndex+0 bis deviceIndex+7) nach 0-7 kopieren ; i-deviceIndex ergibt 0-7
		if (cThermoName[i] < 48 || cThermoName[i] > 125)//nur druckbare ASCII Zeichen zulassen
		{
			DEBUG_PRINT("cThermoName was: ");
			DEBUG_PRINTLN(String(cThermoName[i],HEX));

			cThermoName[i]='-'; // 45="-"
			
			DEBUG_PRINT("cThermoName replace: ");
			DEBUG_PRINTLN(String(cThermoName[i],HEX));

		}
		
	}
	DEBUG_PRINTLN(String(cThermoName));
}


void SaveName(uint8_t deviceIndex, String sName)
{
	send(msgDebugReturnString.set("SaveName"));
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

// void debugMessage(String header, String content)
// {
	// DEBUG code ------
	// Serial.print(header);
	// Serial.println(content);
	// DEBUG code ------   
// }

