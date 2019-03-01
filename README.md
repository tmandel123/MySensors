# MySensors
MySensors Nodes 


Diese MySensors Quellen sind optimiert, um mit dem Controller FHEM zusammenzuarbeiten.
Es gibt Einstellungen, mit denen im laufenden Betrieb Debugging beim Node eingeschaltet werden kann.

Des weiteren liefert jeder Node folgende Informationen:

text_HwTime
text_PA_Level
text_ParentNode
text_PassiveNode
text_RF_Channel
text_TX_RSSI

Das soll helfen das Chaos beim Testen optimaler Einstellungen (insbesondere beim Senden/Empfangen) besser in den Griff zu bekommen.
	
Wiederkehrende Aktionen jedes einzelnen Node wurden in die Datei CommonFunctions.h ausgelagert

mySendSketchInfo();
myPresentation();
myHeartBeatLoop();

SKETCH_VERSION -> wird immer aus Programmversion und Datum/Uhrzeit beim Kompilieren generiert.
	
1.0-001 20190301 13:54

Vorgesehen sind 2 Umgebungen: Produktion und Test

Produktion = Kanal 96
Test = Kanal 76

Updates der Nodes sollen über FOTA erfolgen



|  **Category** | **ID** | **SensorType** | **DataType** | **#define** | **MessageObject** | **TEXT** | **Usage** |
| --- | --- | --- | --- | --- | --- | --- | --- |
|   | 0 |  |  |  |  |  |  |
|   | 1 | S_CUSTOM | V_VAR1 | CHILD_METER_VALUE_GET | msgMeterValueGet |  | for compatibility with old value11 from energy meter |
|   | 2 |  |  |  |  |  |  |
|   | 3 |  |  |  |  |  |  |
|   | 4 |  |  |  |  |  |  |
|   | 5 |  |  |  |  |  |  |
|   | 6 |  |  |  |  |  |  |
|   | 7 |  |  |  |  |  |  |
|   | 8 |  |  |  |  |  |  |
|   | 9 |  |  |  |  |  |  |
|  temperature | 10 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP | msgOwTemp | OW_Temp_01 |  |
|  temperature | 11 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | msgOwTemp | OW_Temp_02 |  |
|  temperature | 12 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | msgOwTemp | OW_Temp_03 |  |
|  temperature | 13 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | msgOwTemp | OW_Temp_04 |  |
|  temperature | 14 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | msgOwTemp | OW_Temp_05 |  |
|  temperature | 15 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | msgOwTemp | OW_Temp_06 |  |
|  temperature | 16 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | msgOwTemp | OW_Temp_07 |  |
|  temperature | 17 | S_TEMP | V_TEMP, V_ID | CHILD_OW_TEMP+i | msgOwTemp | OW_Temp_08 |  |
|  temp-debug | 18 | S_INFO | V_TEXT | CHILD_OW_CONNECTED | msgDebugOWConList | OW_Con_Lst | OneWire Connection List |
|  temp-debug | 19 | S_INFO | V_TEXT | CHILD_OW_DEV_COUNT | msgDebugOWDevCount | OW_Dev_Cnt | number of OneWire Devices |
|  temp-name | 20 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME | msgOwName | OW_Name |  |
|  temp-name | 21 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | msgOwName | OW_Name |  |
|  temp-name | 22 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | msgOwName | OW_Name |  |
|  temp-name | 23 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | msgOwName | OW_Name |  |
|  temp-name | 24 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | msgOwName | OW_Name |  |
|  temp-name | 25 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | msgOwName | OW_Name |  |
|  temp-name | 26 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | msgOwName | OW_Name |  |
|  temp-name | 27 | S_INFO | V_TEXT | CHILD_OW_TEMP_NAME+i | msgOwName | OW_Name |  |
|  temp-debug | 28 | S_INFO | V_TEXT | CHILD_OW_RESOLUTION | msgOwResolution | OW_Resolution |  |
|   | 29 |  |  |  |  |  |  |
|  MultiButton | 30 | S_INFO | V_TEXT | CHILD_MULTI_BUTTON | msgMultiButton | Mlt_Button | Button sends multiple states depending on click (doubleclick, hold etc.) |
|  Button | 31 |  |  |  |  |  |  |
|   | 32 |  |  |  |  |  |  |
|   | 33 |  |  |  |  |  |  |
|   | 34 |  |  |  |  |  |  |
|  EnergyMeter | 35 | S_POWER | V_WATT, V_KWH, V_VAR1 | CHILD_POWER_METER | msgPowerMeter | PowerMeter |  |
|   | 36 |  |  |  |  |  |  |
|   | 37 |  |  |  |  |  |  |
|   | 38 |  |  |  |  |  |  |
|   | 39 |  |  |  |  |  |  |
|  Servo | 40 | S_INFO | V_TEXT | CHILD_SERVO_STATE | msgServoState | Servo_State | feedback of actual Servo Position |
|   | 41 |  |  |  |  |  |  |
|   | 42 |  |  |  |  |  |  |
|   | 43 |  |  |  |  |  |  |
|   | 44 |  |  |  |  |  |  |
|   | 45 |  |  |  |  |  |  |
|   | 46 |  |  |  |  |  |  |
|   | 47 |  |  |  |  |  |  |
|   | 48 |  |  |  |  |  |  |
|   | 49 |  |  |  |  |  |  |
|  Licht | 50 | S_DIMMER | V_DIMMER | CHILD_SINGLE_LED_DIMMER | msgDimmerState | Dimmer |  |
|  Licht | 51 | S_BINARY | V_STATUS | CHILD_SINGLE_LED_SWITCH | msgSwitchState | Switch |  |
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
|  Debug SET | 70 | S_INFO | V_TEXT | CHILD_DEBUG_LEVEL | msgDebugLevel | Dbg_Level | Number of debug level (0=off) |
|  Meter SET | 71 | S_INFO | V_TEXT | CHILD_NEW_METER_VALUE | msgNewMeterValue | SetMeterValue | set new value for gas meter, electric meter, water meter |
|   | 72 | S_INFO | V_TEXT | CHILD_ANLG_TH_MIN | msgThresholdMin | TH_Min | threshold min |
|   | 73 | S_INFO | V_TEXT | CHILD_ANLG_TH_MAX | msgThresholdMax | TH_Max | threshold max |
|   | 74 | S_INFO | V_TEXT | CHILD_KEY_VALUE_SETTER | msgKeyValue | Key_Val | key=value (child has to care about what to do) Name of Thermometer etc. |
|   | 75 | S_INFO | V_TEXT |  |  |  | EchoWaitTime |
|   | 76 |  |  |  |  |  |  |
|   | 77 |  |  |  |  |  |  |
|   | 78 |  |  |  |  |  |  |
|   | 79 |  |  |  |  |  |  |
|  Debug Infos | 80 | S_INFO | V_TEXT | CHILD_PASSIVE_NODE | msgPassiveNode | PassiveNode | MY_PASSIVE_NODE |
|   | 81 | S_INFO | V_TEXT | CHILD_PARENT_NODE | msgParentNode | ParentNode | MY_PARENT_NODE_ID |
|   | 82 |  |  |  |  |  |  |
|   | 83 |  |  |  |  |  |  |
|   | 84 |  |  |  |  |  |  |
|   | 85 |  |  |  |  |  |  |
|   | 86 |  |  |  |  |  |  |
|   | 87 |  |  |  |  |  |  |
|   | 88 |  |  |  |  |  |  |
|  PING ? | 89 |  |  |  |  |  | Node2Node Ping |
|  Debug Infos | 90 | S_INFO | V_TEXT | CHILD_HWTIME | msgHwTime | HwTime | millis() calculated to days since startup |
|  Debug Infos | 91 | S_INFO | V_TEXT | CHILD_TX_RSSI | msgSendingRSSI | TX_RSSI | RF24_getSendingRSSI() |
|  Debug Infos | 92 | S_INFO | V_TEXT | CHILD_RF24_PA_LEVEL | msgPaLevel | PA_Level | RF24_PA_MAX |
|  Debug Infos | 93 | S_INFO | V_TEXT | CHILD_RF24_CHANNEL | msgRFChannel | RF_Channel | 96, 76 |
|  Debug Infos | 94 | S_INFO | V_TEXT | CHILD_ECHO_TIMESTAMP | msgEchoTimeStamp | Echo_TS | TimeStamp |
|  Debug Infos | 95 | S_INFO | V_TEXT | CHILD_ECHO_RUNTIME | msgEchoRunTime | Echo_RT | RunTime |
|  Debug Infos | 96 | S_INFO | V_TEXT | CHILD_DEBUG_RETURN | msgDebugReturnString | Dbg_Return | "Speicher EEPROM ist voll", SaveOwID |
|  Debug Infos | 97 | S_INFO | V_TEXT | CHILD_PACKET_RATIO | msgPacketRatio | Pkt_Ratio | Paket Ratio good/(good+bad)*100 |
|  Battery | 98 | S_MULTIMETER | V_VOLTAGE | CHILD_BAT_ANLG | msgBatAnalogValue | - | sendBatteryLevel() |
|  Battery | 99 | S_MULTIMETER | V_VOLTAGE | CHILD_BAT_VREF | msgBatvRefValue | - | sendBatteryLevel() |








|  **Pin imprint** | **Pin IDE Digital** | **Pin IDE Analog** | **use in MySensors** | **#define** |
| --- | --- | --- | --- | --- |
|  D2 | 2 |  | Button | PRIMARY_BUTTON_PIN |
|  D3 | 3 |  | Button, Sensor, Switch | SECONDARY_BUTTON_PIN, DIGITAL_INPUT_SENSOR, |
|  D4 | 4 |  | OneWire | ONE_WIRE_BUS |
|  D5 | 5 |  | Switch, Dimmer | LED_PWM_PIN, LED_DIGITAL_PIN |
|  D6 | 6 |  |  |  |
|  D7 | 7 |  |  |  |
|  D8 | 8 |  | PulseLED | PULSE_LED |
|  D9 | 9 |  | RF24 |  |
|  D10 | 10 |  | RF24 |  |
|  D11 | 11 |  | RF24 |  |
|  D12 | 12 |  | RF24 |  |
|  D13 | 13 |  | RF24 |  |
|  A0 | 14 | A0 | Sensor | ANALOG_INPUT_SENSOR |
|  A1 | 15 | A1 |  |  |
|  A2 | 16 | A2 |  |  |
|  A3 | 17 | A3 | RX LED green | MY_DEFAULT_RX_LED_PIN |
|  A4 | 18 | A4 | TX LED yellow | MY_DEFAULT_TX_LED_PIN |
|  A5 | 19 | A5 | Err LED red | MY_DEFAULT_ERR_LED_PIN |
|  A6 |  | A6 |  |  |
|  A7 |  | A7 |  |  |


