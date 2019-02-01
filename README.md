# MySensors
MySensors Nodes 


Diese MySensors Quellen sind optimiert, um mit dem Controller FHEM zusammenzuarbeiten.
Es gibt Einstellungen, mit denen im laufenden Betrieb Debugging beim Node eingeschaltet werden kann.

Vorgesehen sind 2 Umgebungen: Produktion und Test

Produktion = Kanal 96
Test = Kanal 76

Updates der Nodes erfolgen mit FOTA über Kanal 76 (der Reboot wird im Kanal 96 initiiert und der Bootloader startet im Standard mit Kanal 76)


|  **Pin imprint** | **Pin IDE Digital** | **Pin IDE Analog** | **use in MySensors** | **#define** |
| --- | --- | --- | --- | --- |
|  D2 | 2 |  | IRQ or Button |  |
|  D3 | 3 |  | Button |  |
|  D4 | 4 |  |  |  |
|  D5 | 5 |  |  |  |
|  D6 | 6 |  |  |  |
|  D7 | 7 |  | RX LED |  |
|  D8 | 8 |  | TX LED |  |
|  D9 | 9 |  | RF24 |  |
|  D10 | 10 |  | RF24 |  |
|  D11 | 11 |  | RF24 |  |
|  D12 | 12 |  | RF24 |  |
|  D13 | 13 |  | RF24 |  |
|  A0 | 14 | A0 | Sensor | ANALOG_INPUT_SENSOR |
|  A1 | 15 | A1 |  |  |
|  A2 | 16 | A2 |  |  |
|  A3 | 17 | A3 |  |  |
|  A4 | 18 | A4 |  |  |
|  A5 | 19 | A5 |  |  |
|  A6 |  | A6 |  |  |
|  A7 |  | A7 |  |  |
