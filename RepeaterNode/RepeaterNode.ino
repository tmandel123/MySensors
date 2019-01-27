// #define MY_DEBUG								//nur mit Debug aktiviert können Sends im Abstand von 50ms weitergeleitet werden. Sonst gibt es zu viele NACKs

// Enable and select radio type attached
#define MY_RADIO_RF24
#define MY_RF24_CHANNEL 96
#define MY_NODE_ID 50
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC

#define MY_RF24_PA_LEVEL RF24_PA_MAX	//NodeID 50, seit MySensors 2.3.1 scheint auch PA_MAX zu funktionieren (Shielded Modul)
// #define MY_RF24_PA_LEVEL RF24_PA_HIGH
// #define MY_RF24_PA_LEVEL RF24_PA_LOW //läuft ohne Fehler, aber keine große Reichweite

/*
RF24_PA_MIN = -18dBm 
RF24_PA_LOW = -12dBm 
RF24_PA_HIGH = -6dBm 
RF24_PA_MAX = 0dBm
*/
#define MY_TRANSPORT_WAIT_READY_MS (5000ul)


#define SKETCH_VER            			"1.2-004"        			// Sketch version
#define SKETCH_NAME           			"Repeater Node"   		// Optional child sensor name

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE
#define HEARTBEAT_INTERVAL        		60000        //später alle 5 Minuten, zum Test alle 30 Sekunden
#define CHILD_ID_TEXT					0


#include <MySensors.h>

#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif

MyMessage hwTime		(CHILD_ID_TEXT, V_TEXT);
unsigned long lastHeartBeat = HEARTBEAT_INTERVAL - 8000;


void preHwInit() 
{
	DEBUG_SERIAL(MY_BAUD_RATE);
	DEBUG_PRINTLN("preHwInit...");
}

void before()
{
	DEBUG_PRINTLN("before...");
	DEBUG_PRINT("Version: ");
	DEBUG_PRINTLN(SKETCH_VER);
}

void setup()
{

}

void presentation()
{
	DEBUG_PRINTLN("presentation...");
	char CompileDate[8];
	getCompileDateTime(__DATE__, CompileDate);
	DEBUG_PRINTLN(CompileDate);
	
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

	present(CHILD_ID_TEXT, S_INFO, "Info Child");
}

void loop()
{
	unsigned long currentTime = millis();
	if (currentTime - lastHeartBeat > (unsigned long)HEARTBEAT_INTERVAL)
	{
		// sendHeartbeat();  
		send(hwTime.set(currentTime),true);
		lastHeartBeat = currentTime;
		// Serial.println("HEARTBEAT_INTERVAL erreicht");

	}
}

void getCompileDateTime(char const *date, char *buff) {
    int month, day, year;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", buff, &day, &year);
    month = (strstr(month_names, buff)-month_names)/3+1;
    sprintf(buff, "%d%02d%02d", year, month, day);
}
