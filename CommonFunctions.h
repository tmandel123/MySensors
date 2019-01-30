#ifdef SER_DEBUG
#define DEBUG_SERIAL(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_SERIAL(x)
#define DEBUG_PRINT(x) 
#define DEBUG_PRINTLN(x) 
#endif

void getCompileDateTime(char const *date, char *buff) {
    int month, day, year;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", buff, &day, &year);
    month = (strstr(month_names, buff)-month_names)/3+1;
    sprintf(buff, "%d%02d%02d", year, month, day);
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

