# MySensors
MySensors Nodes 


Diese MySensors Quellen sind optimiert, um mit dem Controller FHEM zusammenzuarbeiten.
Es gibt Einstellungen, mit denen im laufenden Betrieb Debugging beim Node eingeschaltet werden kann.

Vorgesehen sind 2 Umgebungen: Produktion und Test

Produktion = Kanal 96
Test = Kanal 76

Updates der Nodes sollen über FOTA erfolgen





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
|  temperature | 10 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP | MsgOwTemp |  |  |
|  temperature | 11 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | MsgOwTemp |  |  |
|  temperature | 12 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | MsgOwTemp |  |  |
|  temperature | 13 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | MsgOwTemp |  |  |
|  temperature | 14 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | MsgOwTemp |  |  |
|  temperature | 15 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | MsgOwTemp |  |  |
|  temperature | 16 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | MsgOwTemp |  |  |
|  temperature | 17 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | MsgOwTemp |  |  |
|  temp-debug | 18 | S_INFO | V_TEXT | CHILD_OW_CONNECTED | MsgDebugOWConList | OW_CON_LST | OneWire Connection List |
|  temp-debug | 19 | S_INFO | V_TEXT | CHILD_OW_DEV_COUNT | MsgDebugOWDevCount | OW_DEV_CNT | number of OneWire Devices |
|  temp-name | 20 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME | MsgOwName | OW_NAME |  |
|  temp-name | 21 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | MsgOwName | OW_NAME |  |
|  temp-name | 22 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | MsgOwName | OW_NAME |  |
|  temp-name | 23 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | MsgOwName | OW_NAME |  |
|  temp-name | 24 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | MsgOwName | OW_NAME |  |
|  temp-name | 25 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | MsgOwName | OW_NAME |  |
|  temp-name | 26 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | MsgOwName | OW_NAME |  |
|  temp-name | 27 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | MsgOwName | OW_NAME |  |
|   | 28 |  |  |  |  |  |  |
|   | 29 |  |  |  |  |  |  |
|  MultiButton | 30 | S_INFO | V_TEXT | CHILD_MULTI_BUTTON | MsgMultiButton |  | Button sends multiple states depending on click (doubleclick, hold etc.) |
|  Button | 31 |  |  |  |  |  |  |
|   | 32 |  |  |  |  |  |  |
|   | 33 |  |  |  |  |  |  |
|   | 34 |  |  |  |  |  |  |
|   | 35 |  |  |  |  |  |  |
|   | 36 |  |  |  |  |  |  |
|   | 37 |  |  |  |  |  |  |
|   | 38 |  |  |  |  |  |  |
|   | 39 |  |  |  |  |  |  |
|  Servo | 40 | S_INFO | V_TEXT | CHILD_SERVO_STATE | MsgServoState | SERVO_STATE | feedback of actual Servo Position |
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
|  Debug SET | 70 | S_INFO | V_TEXT | CHILD_DEBUG_LEVEL | MsgDebugLevel | DBG_LEVEL | Number of debug level (0=off) |
|   | 71 | S_INFO | V_TEXT | CHILD_ANLG_TH_MIN | MsgThresholdMin | TH_MIN | threshold min |
|   | 72 | S_INFO | V_TEXT | CHILD_ANLG_TH_MAX | MsgThresholdMax | TH_MAX | threshold max |
|   | 73 | S_INFO | V_TEXT | CHILD_KEY_VALUE_SETTER | MsgKeyValue | KEY_VAL | key=value (child has to care about what to do) Name of Thermometer etc. |
|   | 74 | S_INFO | V_TEXT |  |  |  | EchoWaitTime |
|   | 75 |  |  |  |  |  |  |
|   | 76 |  |  |  |  |  |  |
|   | 77 |  |  |  |  |  |  |
|   | 78 |  |  |  |  |  |  |
|   | 79 |  |  |  |  |  |  |
|   | 80 |  |  |  |  |  |  |
|   | 81 |  |  |  |  |  |  |
|   | 82 |  |  |  |  |  |  |
|   | 83 |  |  |  |  |  |  |
|   | 84 |  |  |  |  |  |  |
|   | 85 |  |  |  |  |  |  |
|   | 86 |  |  |  |  |  |  |
|   | 87 |  |  |  |  |  |  |
|   | 88 |  |  |  |  |  |  |
|  PING ? | 89 |  |  |  |  |  | Node2Node Ping |
|  Debug Infos | 90 | S_INFO | V_TEXT | CHILD_HWTIME | MsgHwTime | HWTIME | millis() calculated to days since startup |
|  Debug Infos | 91 | S_INFO | V_TEXT | CHILD_TX_RSSI | MsgSendingRSSI | TX_RSSI | RF24_getSendingRSSI() |
|  Debug Infos | 92 | S_INFO | V_TEXT | CHILD_RF24_PA_LEVEL | MsgPaLevel | PA_LEVEL | RF24_PA_MAX |
|  Debug Infos | 93 | S_INFO | V_TEXT | CHILD_RF24_CHANNEL | MsgRFChannel | RF_CHANNEL | 96, 76 |
|  Debug Infos | 94 | S_INFO | V_TEXT | CHILD_ECHO_TIMESTAMP | MsgEchoTimeStamp | ECHO_TS | TimeStamp |
|  Debug Infos | 95 | S_INFO | V_TEXT | CHILD_ECHO_RUNTIME | MsgEchoRunTime | ECHO_RT | RunTime |
|  Debug Infos | 96 | S_INFO | V_TEXT | CHILD_DEBUG_RETURN | MsgDebugReturnString | DBG_RETURN | "Speicher EEPROM ist voll", SaveOwID |
|   | 97 |  |  |  |  |  |  |
|  Battery | 98 | S_MULTIMETER | V_VOLTAGE | CHILD_BAT_ANLG | BatAnalogValue | - | sendBatteryLevel() |
|  Battery | 99 | S_MULTIMETER | V_VOLTAGE | CHILD_BAT_VREF | MsgBatvRefValue | - | sendBatteryLevel() |










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

