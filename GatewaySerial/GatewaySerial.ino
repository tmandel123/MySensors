/* ToDo:

Debug Child, welcher z.B. Mitteilt, mit welchem PA-Level kompiliert wurde

*/
// Enable debug prints to serial monitor
#define MY_DEBUG
#define MY_RADIO_RF24
// #define MY_RF24_CHANNEL 96									// Für Testphase deaktivieren, damit Kanal 76 aktiv wird (Prod=96 Test=76)

// #define MY_RF24_PA_LEVEL RF24_PA_MAX //liefert fast nur NACK
// #define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_RF24_PA_LEVEL RF24_PA_LOW //läuft ohne Fehler, aber keine große Reichweite

/*
RF24_PA_MIN = -18dBm 
RF24_PA_LOW = -12dBm 
RF24_PA_HIGH = -6dBm 
RF24_PA_MAX = 0dBm
*/

// Enable serial gateway
#define MY_GATEWAY_SERIAL

// Define a lower baud rate for Arduinos running on 8 MHz (Arduino Pro Mini 3.3V & SenseBender)
#if F_CPU == 8000000L
#define MY_BAUD_RATE 38400
#endif

// Enable inclusion mode
// #define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
#define MY_INCLUSION_BUTTON_FEATURE

// Inverses behavior of inclusion button (if using external pullup)
//#define MY_INCLUSION_BUTTON_EXTERNAL_PULLUP

// Set inclusion mode duration (in seconds)
// #define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
#define MY_INCLUSION_MODE_BUTTON_PIN  3

// Set blinking period
// #define MY_DEFAULT_LED_BLINK_PERIOD 300

// Inverses the behavior of leds
//#define MY_WITH_LEDS_BLINKING_INVERSE

// Flash leds on rx/tx/err
// Uncomment to override default HW configurations
//#define MY_DEFAULT_ERR_LED_PIN 4  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  6  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  5  // the PCB, on board LED

#include <MySensors.h>

void setup()
{
	// Setup locally attached sensors
}

void presentation()
{
	// Present locally attached sensors
}

void loop()
{
	// delay(25);
	// Send locally attached sensor data here
}