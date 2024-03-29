/*

*/


#include <Wire.h>
#include "RTClib.h"
#include "clockdisplay.h"
#include <EEPROM.h>


clockDisplay::clockDisplay(uint8_t address, int saveAddress){
// Gets the i2c address and eeprom save location, default saveAddress is -1 if not provided to disable saving
// call begin() to start
  _address = address;
  _saveAddress = saveAddress;
  
  }

//private functions
uint8_t clockDisplay::getLED7SegChar(uint8_t value){
// returns bit pattern for provided number 0-9 or number provided as a char 0-9 for display on 7 segment display
  
  if(value >= '0' && value <='9'){ // it was provided as a char
      return numDigs [value - 48];
    } else if( value <=9){
      return numDigs [value];
      }
    return 0x0; // blank on invalid
}


uint16_t clockDisplay::getLEDAlphaChar(char value){
// returns bit pattern for provided character for display on alphanumeric display

  if(value == ' '){
    return alphaChars[0];
  } else if(value >= 'A' && value <= 'Z') {
      return alphaChars[value - 64];    
  } else if(value >= 'a' && value <= 'z') {
    // Everthing will be uppercase
      return alphaChars[value - 96];
  } else if(value >= '0' && value <= '9') {
      return alphaChars[value - 21]; 
  }  else {
       return alphaChars[0]; // blank if not found
  }
}




uint16_t clockDisplay::makeNum(uint8_t num){
// Make a number from the array and place it in the buffer at pos
// Each position holds two digits, high byte is 1's, low byte is 10's
      uint16_t segments;
       segments = getLED7SegChar(num%10) << 8; // Place 1's in upper byte
      segments = segments | getLED7SegChar(num/10); // 10's in lower byte
        return segments;
      
      
}

uint16_t clockDisplay::makeAlpha(uint8_t value){
//positions are 0 to 2
  return getLEDAlphaChar(value);

}



void clockDisplay::clearDisplay(){
  // directly clear the display RAM

  Wire.beginTransmission(_address);
  Wire.write(0x00); // start at address 0x0

  for (int i = 0; i < 16; i++) {
    Wire.write(0x0);      
  }
  Wire.endTransmission();

  
  }


void clockDisplay::lampTest(){ // turn on all LEDs
  Wire.beginTransmission(_address);
  Wire.write(0x00); // start at address 0x0

  for (int i = 0; i < 16; i++) {
    Wire.write(0xFF);       
  }
  Wire.endTransmission(); 

}  

void clockDisplay::begin(){
// Start the display
  
  Wire.beginTransmission(_address);
  Wire.write(0x20 | 1); // turn on oscillator
  Wire.endTransmission();
  
  clear(); //clear buffer
  setBrightness(15); // be sure in case coming up for mc reset
  clearDisplay(); //clear display RAM, comes up random.
  on(); // turn it on
  
  }
void clockDisplay::on(){
// Turn on the display
  Wire.beginTransmission(_address);
  Wire.write(0x80 | 1); // turn on the display
  Wire.endTransmission();
  
  }


void clockDisplay::off(){
// Turn off the display
  Wire.beginTransmission(_address);
  Wire.write(0x80); // turn off the display
  Wire.endTransmission();
  
  }
void clockDisplay::clear(){
  // clears the buffer
  // must call show to clear display
    
  // Holds the LED segment status
  // Month 0,1,2
  // Day 3
  // Year 4 and 5
  // Hour 6
  // Min 7
//  Serial.println("Clear Buffer");
  for(int i = 0; i < 8; i++){
    _displayBuffer[i] = 0;
  }
}


uint8_t clockDisplay::setBrightness(uint8_t level){
// Valid brighness levesl are 0 to 15. Default is 15.
  
  if(level > 15)
    return _brightness;

  Wire.beginTransmission(_address);
  Wire.write(0xE0 | level); // Dimming command
  Wire.endTransmission();

  
  //Serial.println("Setting brightness");
  //Serial.println(level,DEC);
  //Serial.println(0xE0 | level, BIN);

  _brightness = level;
  return _brightness;
}
uint8_t clockDisplay::getBrightness(){
  
  return _brightness;
  
  }
bool clockDisplay::save(){
// save date/time to eeprom
 // Serial.println("in save");
  uint16_t  sum = 0; //add them up for simple checksum

  if(!isRTC() && _saveAddress >= 0){ // rtc can't save, save address was not set and can't save if negative
    //Serial.println("in save not rtc");
    EEPROM.update(_saveAddress, _year & 0x00FF);
    sum = sum + _year & 0x00FF;

    EEPROM.update(_saveAddress + 1, (_year >> 8) & 0x00FF);
    sum = sum + (_year >> 8) & 0x00FF;

    EEPROM.update(_saveAddress + 2, _month);
    sum = sum + _month;

    EEPROM.update(_saveAddress + 3, _day);
    sum = sum + _day;

    EEPROM.update(_saveAddress + 4, _hour);
    sum = sum + _hour;

    EEPROM.update(_saveAddress + 5, _minute);
    sum = sum + _minute;


    EEPROM.update(_saveAddress + 6, _brightness);
    sum = sum + _brightness;
    
    sum = sum & 0x00FF; // 8 bit checksum
    EEPROM.update(_saveAddress + 7, sum);
  } else if(isRTC() && _saveAddress >= 0){
    //Serial.println("in save IS rtc");
    for(uint8_t c = 0; c < 6; c++){
       // Serial.println(c);
        EEPROM.update(_saveAddress + c, 0x00); // rtc has it's time in the RTC chip
      } 

    EEPROM.update(_saveAddress + 6, _brightness);
    sum = sum + _brightness;
    
    sum = sum & 0x00FF; // 8 bit checksum
    EEPROM.update(_saveAddress + 7, sum);  
  
  } else {
    return false;
  }

  return true;
}

bool clockDisplay::load(){
// Load saved date/time from eeprom
  
  uint16_t  sum = 0;
  
  if(_saveAddress >= 0){

    for(int c = 0; c <= 6; c++){
      sum = sum + EEPROM.read(_saveAddress + c);
      }

      sum = sum & 0x00FF; // 8 bit checksum


      if(sum == EEPROM.read(_saveAddress + 7)){ // saved checksum matches

        if(!isRTC()){ // not a rtc, load saved values
          //Serial.println(">>>>>>>>>>>>>>>>>>> LOADING <<<<<<<<<<<<<<<<<<<");
          setYear(EEPROM.read(_saveAddress + 1) << 8  | EEPROM.read(_saveAddress));
          setMonth(EEPROM.read(_saveAddress + 2));
          setDay(EEPROM.read(_saveAddress + 3));
          setHour(EEPROM.read(_saveAddress + 4));
          setMinute(EEPROM.read(_saveAddress + 5));
          setBrightness(EEPROM.read(_saveAddress + 6));
          return true;
        } else if(isRTC()){
          // rtc doesnt save any time
          setBrightness(EEPROM.read(_saveAddress + 6));
          }

      } else{
      //  Serial.println("Invalid!");
        return false;
      }

  } else {
      return false; // a valid eeprom address is not set, can't load anything or is RTC
    }

  return true;
}

void clockDisplay::show(){
// Show the buffer

  if(_hour < 12 ){
    AM();    
  } else {
    PM();
  }
	  
	  
  if(_colon){
	  colonOn();
  } else {
	  colonOff();
	}

  Wire.beginTransmission(_address);
  Wire.write(0x00); // start at address 0x0

  for (int i = 0; i < 8; i++) {
    Wire.write(_displayBuffer[i] & 0xFF);    
    Wire.write(_displayBuffer[i] >> 8);    
  }
  Wire.endTransmission();  
}

void clockDisplay::showAnimate1(){
  // Show all but month
  
  off();

  if(_hour < 12 ){
    AM();    
    } else {
      PM();
    }
    
    
  if(_colon){
    colonOn();
  } else {
    colonOff();
  }

  Wire.beginTransmission(_address);
  Wire.write(0x00); // start at address 0x0

  for (int i = 0; i < 8; i++) {
    if(i>2){
      Wire.write(_displayBuffer[i] & 0xFF);    
      Wire.write(_displayBuffer[i] >> 8);
    } else {
      Wire.write(0x00); //blank month, first 3 16 bit locations    
      Wire.write(0x00);     
      
      }   
  }
  Wire.endTransmission();  

  on();

  
}

void clockDisplay::showAnimate2(){
  // Show month, assumes showAnimate1() was already called

  Wire.beginTransmission(_address);
  Wire.write(0x00); // start at address 0x0
  for (int i = 0; i < 3; i++) {
    Wire.write(_displayBuffer[i] & 0xFF);    
    Wire.write(_displayBuffer[i] >> 8);    
  }
  Wire.endTransmission(); 
  
}

void clockDisplay::setMonth(int monthNum){
// Makes characters for 3 char month, valid months 1-12

  if(monthNum < 13){
    _month = monthNum; // keep track
    monthNum--; //array starts at 0 
    _displayBuffer[0] = makeAlpha(months[monthNum][0]);
    _displayBuffer[1] = makeAlpha(months[monthNum][1]);
    _displayBuffer[2] = makeAlpha(months[monthNum][2]);
  } // else invalid
}


void clockDisplay::setDay(int dayNum){
// Place LED pattern in day position in buffer, which is 3.
//  Serial.println("Setting day");

  _day = dayNum;
  _displayBuffer[3] = makeNum(dayNum);    
}
void clockDisplay::setYear(uint16_t yearNum){ //void clockDisplay::setYear(uint16_t yearNum){
// Place LED pattern in year position in buffer, which is 4 and 5.

  _year = yearNum;
  _displayBuffer[4] = makeNum(yearNum /100);
  _displayBuffer[5] = makeNum(yearNum %100);

      
}

void clockDisplay::setHour(uint16_t hourNum){
// Place LED pattern in hour position in buffer, which is 6.
//  Serial.println("Setting hour");
  _hour = hourNum;


// Show it as 12 hour time
// AM/PM will be set on show() to avoid being overwritten


  if(hourNum == 0){
  _displayBuffer[6] = makeNum(12);
  }
  else if(hourNum >12){
    // pm
    _displayBuffer[6] = makeNum(hourNum - 12);
    } else{
      
      // am
  _displayBuffer[6] = makeNum(hourNum);
}

      
}
    void clockDisplay::setMinute(int minNum){
// Place LED pattern in minute position in buffer, which is 7.
//  Serial.println("Setting min");

  _minute = minNum;
  _displayBuffer[7] = makeNum(minNum);    
}

void clockDisplay::AM(){

  _displayBuffer[3] = _displayBuffer[3] | 0x0080;
  _displayBuffer[3] = _displayBuffer[3] & 0x7FFF;  
  return;    
}

void clockDisplay::PM(){

  _displayBuffer[3] = _displayBuffer[3] | 0x8000; 
  _displayBuffer[3] = _displayBuffer[3] & 0xFF7F; 
  return; 
}

void clockDisplay::colonOn(){

      _displayBuffer[4] = _displayBuffer[4] | 0x8080; 
      return;
}

void clockDisplay::colonOff(){

      _displayBuffer[4] = _displayBuffer[4] & 0x7F7F; 
      return;
}




void clockDisplay::showOnlyMonth(int monthNum){
// clears the display RAM and only shows the provided month

    clearDisplay(); 


    directCol(0, makeAlpha(months[monthNum - 1][0]));
    directCol(1, makeAlpha(months[monthNum - 1][1]));
    directCol(2, makeAlpha(months[monthNum - 1][2]));
  
  }

void clockDisplay::showOnlySave(){
// clears the display RAM and only shows the word save

    clearDisplay(); 
/*    if(isRTC()){
      directCol(0, makeAlpha('S'));
      directCol(1, makeAlpha('E'));
      directCol(2, makeAlpha('T'));
    } else{
*/
      directCol(0, makeAlpha('S'));
      directCol(1, makeAlpha('A'));
      directCol(2, makeAlpha('V'));
      directCol(3, numDigs[10]); // 10 is an E
/*    }
*/

    
  
  }

void clockDisplay::showOnlySettingVal(const char* setting, int8_t val, bool clear){

  if(clear)
    clearDisplay();


  int8_t c = 0;
  while(c < 3 && setting[c]){
    directCol(c, makeAlpha(setting[c]));
    c++;
    }

  if(val >= 0 && val < 100)
    directCol(3, makeNum(val));
  else
    directCol(3, 0x00);
    
  
}
  
void clockDisplay::showOnlyDay(int dayNum){

  // clears the display RAM and only shows the provided day
  

    
    clearDisplay();
    
    directCol(3, makeNum(dayNum));
  
  }


void clockDisplay::showOnlyYear(int yearNum){

// clears the display RAM and only shows the provided year


    clearDisplay();
    
    directCol(4, makeNum(yearNum /100));
    directCol(5, makeNum(yearNum %100));
}

void clockDisplay::showOnlyHour(int hourNum){

  // clears the display RAM and only shows the provided hour
  

    
    clearDisplay();

    if(hourNum == 0 ){
      directCol(6, makeNum(12));
      directAM();        
      }

    else if(hourNum >12){
    // pm
     directCol(6, makeNum(hourNum - 12));
   
    } else {
      
      // am
      directCol(6, makeNum(hourNum));
      directAM();
    }


    if(hourNum > 11){
      directPM();
    } else {
      directAM();}

    
    
  
  }

void clockDisplay::showOnlyMinute(int minuteNum){

  // clears the display RAM and only shows the provided minute
  

    
    clearDisplay();
    
    directCol(7, makeNum(minuteNum));
  
  }


void clockDisplay::directCol(int col, int segments){
// directly write directly to a column with supplied segments
// Month/Alpha - first 3 cols  
// Day - column 4
// Year - column 5 & 6
// Hour - column 7
// Min - column 8

  
  Wire.beginTransmission(_address);
  Wire.write(col * 2); // 2 bytes per col * position, day is at pos  
  // leave buffer intact, direclty write to display
  Wire.write(segments & 0xFF);    
  Wire.write(segments >> 8);    

  Wire.endTransmission();  



}

void clockDisplay::directAM(){

   Wire.beginTransmission(_address);
  Wire.write(0x6); // 2 bytes per col * position, day is at pos  
  // leave buffer intact, direclty write to display
  Wire.write(0x80);    
  Wire.write(0x0);    

  Wire.endTransmission();  

  }
void clockDisplay::directPM(){

   Wire.beginTransmission(_address);
  Wire.write(0x6); // 2 bytes per col * position, day is at pos  
  // leave buffer intact, direclty write to display
  Wire.write(0x0);    
  Wire.write(0x80);    

  Wire.endTransmission();
  }

void clockDisplay::setDateTime(DateTime dt){
// Set the displayed time with supplied DateTime object
// 
// DateTime implemention does not work for years < 2000!

    setMonth(dt.month());
    setDay(dt.day());
    setYear(dt.year());
    setHour(dt.hour());
    setMinute(dt.minute());
  }
void clockDisplay::setFromStruct(dateStruct* s){
// Set time from array, YEAR, MONTH, DAY, HOUR, MIN
// Values not checked for correctness

  setYear(s->year);
  setMonth(s->month);
  setDay(s->day);
  setHour(s->hour);
  setMinute(s->minute);
  
  }
DateTime clockDisplay::getDateTime(){
  // returns a DateTime that we're set to
  // this could be broken, DateTime implementation doesn't work with years < 2000
  
  return DateTime(_year, _month, _day, _hour, _minute, 0);
  
  }





uint8_t clockDisplay::getMonth(){
	return _month;
}

uint8_t clockDisplay::getDay(){
	return _day;
}

uint16_t clockDisplay::getYear(){

  
	return _year;
	}
	
uint8_t clockDisplay::getHour(){
	return _hour;
	}
	
uint8_t clockDisplay::getMinute(){
return _minute;
}
void clockDisplay::setColon(bool col){
// set true to turn it on
	_colon = col; 
}



void clockDisplay::setRTC(bool rtc){
	// track if this is will be holding real time.
	
	_rtc = rtc;
}


bool clockDisplay::isRTC(){
	// is this an real time display?
	return _rtc;
	}
