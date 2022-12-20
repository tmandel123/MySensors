// #include <Arduino.h>
#include "DummyData.h"

void setup() {
	Serial.begin(9600);
	Serial.println(" ");
	Serial.println("Simulator begin...");
	pinMode(LED_BUILTIN, OUTPUT);
}



void loop() {

	digitalWrite(LED_BUILTIN, HIGH); 
	delay(30); 
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	digitalWrite(LED_BUILTIN, HIGH); 
	delay(30); 
	digitalWrite(LED_BUILTIN, LOW);


	// #### Datensatz 1
	
	for (unsigned int i = 0; i < HLY_bin_len; ++i) 
	{
		Serial.write(pgm_read_byte(&HLY_bin1[i]));
	}
	
	digitalWrite(LED_BUILTIN, HIGH); 
	delay(30); 
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
	
	// #### Datensatz 2
	
	for (unsigned int i = 0; i < HLY_bin_len; ++i) 
	{
		Serial.write(pgm_read_byte(&HLY_bin2[i]));
	}
	
	digitalWrite(LED_BUILTIN, HIGH); 
	delay(30); 
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);

	// #### Datensatz 3
	
	for (unsigned int i = 0; i < HLY_bin_len; ++i) 
	{
		Serial.write(pgm_read_byte(&HLY_bin3[i]));
	}
	digitalWrite(LED_BUILTIN, HIGH); 
	delay(30); 
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
	
	

//	#### Einspeisung
	
	for (unsigned int i = 0; i < HLY_bin_len; ++i) 
	{
		Serial.write(pgm_read_byte(&HLY_Einspeisung[i]));
	}
	digitalWrite(LED_BUILTIN, HIGH); 
	delay(30); 
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
	
	
	
	
}
