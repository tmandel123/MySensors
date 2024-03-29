/***************************************************
 This example reads Capacitive Soil Moisture Sensor.

 Created 2015-10-21
 By berinie Chen <bernie.chen@dfrobot.com>

 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here: https://www.dfrobot.com/wiki/index.php?title=Capacitive_Soil_Moisture_Sensor_SKU:SEN0193
 2.This code is tested on Arduino Uno.
 3.Sensor is connect to Analog 0 port.
 ****************************************************/

const int AirValue = 652;   //you need to replace this value with Value_1
const int WaterValue = 251;  //you need to replace this value with Value_2
int intervals = (AirValue - WaterValue)/3;
int soilMoistureValue = 0;
uint32_t zwischenWert = 0;
void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
}
void loop() {
	
	for (int i=0; i<200; i++)
	{
		soilMoistureValue = analogRead(A0);  //put Sensor insert into soil
		zwischenWert = zwischenWert + soilMoistureValue;
		delay (10);
	}
	
	
	zwischenWert = zwischenWert / 200;
	
	soilMoistureValue = zwischenWert;
		
soilMoistureValue = analogRead(A0);  //put Sensor insert into soil
if(soilMoistureValue > WaterValue && soilMoistureValue < (WaterValue + intervals))
{
  Serial.println("Very Wet");
}
else if(soilMoistureValue > (WaterValue + intervals) && soilMoistureValue < (AirValue - intervals))
{
  Serial.println("Wet");
}
else if(soilMoistureValue < AirValue && soilMoistureValue > (AirValue - intervals))
{
  Serial.println("Dry");
}

// else
{
	Serial.print("Werte ");
	Serial.println(soilMoistureValue);
}
delay(100);
}

