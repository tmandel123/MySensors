# MySensors
MySensors Nodes 


Diese MySensors Quellen sind optimiert, um mit dem Controller FHEM zusammenzuarbeiten.
Es gibt Einstellungen, mit denen im laufenden Betrieb Debugging beim Node eingeschaltet werden kann.

Vorgesehen sind 2 Umgebungen: Produktion und Test

Produktion = Kanal 96
Test = Kanal 76

Updates der Nodes erfolgen mit FOTA über Kanal 76 (der Reboot wird im Kanal 96 initiiert und der Bootloader startet im Standard mit Kanal 76)






|  **Category** | **ID** | **SensorType** | **DataType** | **#define** | **MessageObject** | **TEXT** | **Usage** |
| --- | --- | --- | --- | --- | --- | --- | --- |
|   | 0 |  |  |  |  |  |  |
|   | 1 |  |  |  |  |  |  |
|   | 2 |  |  |  |  |  |  |
|   | 3 |  |  |  |  |  |  |
|   | 4 |  |  |  |  |  |  |
|   | 5 |  |  |  |  |  |  |
|   | 6 |  |  |  |  |  |  |
|   | 7 |  |  |  |  |  |  |
|   | 8 |  |  |  |  |  |  |
|   | 9 |  |  |  |  |  |  |
|  Temperatur | 10 | S_TEMP |  | CHILD_ID_Temp |  |  |  |
|   | 11 | S_TEMP |  | CHILD_ID_Temp+i |  |  |  |
|   | 12 | S_TEMP |  | CHILD_ID_Temp+i |  |  |  |
|   | 13 | S_TEMP |  | CHILD_ID_Temp+i |  |  |  |
|   | 14 | S_TEMP |  | CHILD_ID_Temp+i |  |  |  |
|   | 15 | S_TEMP |  | CHILD_ID_Temp+i |  |  |  |
|   | 16 | S_TEMP |  | CHILD_ID_Temp+i |  |  |  |
|   | 17 | S_TEMP |  | CHILD_ID_Temp+i |  |  |  |
|   | 18 |  |  |  |  |  |  |
|   | 19 |  |  |  |  |  |  |
|  Servo | 20 | S_INFO |  | CHILD_SERVO_STATE | MsgServoState | SERVO_STATE | feedback of actual Servo Position |
|   | 21 |  |  |  |  |  |  |
|   | 22 |  |  |  |  |  |  |
|   | 23 |  |  |  |  |  |  |
|   | 24 |  |  |  |  |  |  |
|   | 25 |  |  |  |  |  |  |
|   | 26 |  |  |  |  |  |  |
|   | 27 |  |  |  |  |  |  |
|   | 28 |  |  |  |  |  |  |
|   | 29 |  |  |  |  |  |  |
|  MultiButton | 30 |  |  |  |  |  | Button sends multiple states depending on click (doubleclick, hold etc.) |
|  Button | 31 |  |  |  |  |  |  |
|   | 32 |  |  |  |  |  |  |
|   | 33 |  |  |  |  |  |  |
|   | 34 |  |  |  |  |  |  |
|   | 35 |  |  |  |  |  |  |
|   | 36 |  |  |  |  |  |  |
|   | 37 |  |  |  |  |  |  |
|   | 38 |  |  |  |  |  |  |
|   | 39 |  |  |  |  |  |  |
|   | 40 |  |  |  |  |  |  |
|   | 41 |  |  |  |  |  |  |
|   | 42 |  |  |  |  |  |  |
|   | 43 |  |  |  |  |  |  |
|   | 44 |  |  |  |  |  |  |
|   | 45 |  |  |  |  |  |  |
|   | 46 |  |  |  |  |  |  |
|   | 47 |  |  |  |  |  |  |
|   | 48 |  |  |  |  |  |  |
|   | 49 |  |  |  |  |  |  |
|   | 50 |  |  |  |  |  |  |
|   | 51 |  |  |  |  |  |  |
|   | 52 |  |  |  |  |  |  |
|   | 53 |  |  |  |  |  |  |
|   | 54 |  |  |  |  |  |  |
|   | 55 |  |  |  |  |  |  |
|   | 56 |  |  |  |  |  |  |
|   | 57 |  |  |  |  |  |  |
|   | 58 |  |  |  |  |  |  |
|   | 59 |  |  |  |  |  |  |
|   | 60 |  |  |  |  |  |  |
|   | 61 |  |  |  |  |  |  |
|   | 62 |  |  |  |  |  |  |
|   | 63 |  |  |  |  |  |  |
|   | 64 |  |  |  |  |  |  |
|   | 65 |  |  |  |  |  |  |
|   | 66 |  |  |  |  |  |  |
|   | 67 |  |  |  |  |  |  |
|   | 68 |  |  |  |  |  |  |
|   | 69 |  |  |  |  |  |  |
|   | 70 |  |  |  |  |  |  |
|   | 71 |  |  |  |  |  |  |
|   | 72 |  |  |  |  |  |  |
|   | 73 |  |  |  |  |  |  |
|   | 74 |  |  |  |  |  |  |
|   | 75 |  |  |  |  |  |  |
|   | 76 |  |  |  |  |  |  |
|   | 77 |  |  |  |  |  |  |
|   | 78 |  |  |  |  |  |  |
|   | 79 |  |  |  |  |  |  |
|  Debug Infos | 80 | S_MULTIMETER | V_VOLTAGE | CHILD_BAT_ANLG | BatAnalogValue |  | sendBatteryLevel() |
|   | 81 | S_MULTIMETER | V_VOLTAGE | CHILD_BAT_VREF | BatvRefValue | - | sendBatteryLevel() |
|   | 82 | S_INFO | V_TEXT | CHILD_DEBUG_LEVEL |  | DBG_LEVEL | Number of debug level (0=off) |
|   | 83 | S_INFO | V_TEXT | CHILD_ANLG_TH_MIN |  | TH_MIN | threshold min |
|   | 84 | S_INFO | V_TEXT | CHILD_ANLG_TH_MAX |  | TH_MAX | threshold max |
|   | 85 |  |  |  |  |  |  |
|   | 86 |  |  |  |  |  |  |
|   | 87 |  |  |  |  |  |  |
|   | 88 |  |  |  |  |  |  |
|   | 89 |  |  |  |  |  |  |
|   | 90 | S_INFO | V_TEXT | CHILD_HWTIME | MsgHwTime | HWTIME | millis() calculated to days since startup |
|   | 91 | S_INFO | V_TEXT | CHILD_TX_RSSI | MsgSendingRSSI | TX_RSSI | RF24_getSendingRSSI() |
|   | 92 | S_INFO | V_TEXT | CHILD_RF24_PA_LEVEL | MsgPaLevel | PA_LEVEL |  |
|   | 93 | S_INFO | V_TEXT | CHILD_RF24_CHANNEL | MsgRFChannel | RF_CHANNEL |  |
|   | 94 | S_INFO | V_TEXT | CHILD_ECHO_TIME | MsgEchoTime | ECHO |  |
|   | 95 | S_INFO | V_TEXT | CHILD_ECHO_RUNTIME | MsgEchoRunTime | ECHO_TIME |  |
|   | 96 |  |  |  |  |  |  |
|   | 97 |  |  |  |  |  |  |
|   | 98 |  |  |  |  |  |  |
|   | 99 |  |  |  |  |  |  |










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

