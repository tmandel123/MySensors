#include <EEPROM.h>

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include "clockdisplay.h"



//config

#define buttonDown 8 // Button SET MENU'
#define buttonUp 7 // Button +
#define buttonSet 6 // Button -
#define secondIn 5 // Monitor 1Hz from the DS3231
#define statusOut 13 // Status LED
#define maxTime 240 // maximum time in seconds before timout during setting and no button pressed, max 255 seconds
#define autoIntervalAddr 0x00 // eeprom autoInterval save location

// The displays
clockDisplay destinationTime(0x70,0x10); // i2c address, eeprom save location, 8 bytes are needed
clockDisplay presentTime(0x71,0x18);
clockDisplay departedTime(0x72,0x20);




// Automatic times


//                                  YEAR, MONTH, DAY, HOUR, MIN
dateStruct destinationTimes[8] = { {1985, 10, 26, 1, 21},
                                    {1985, 10, 26, 1,24},
                                    {1955, 11,  5, 6, 0},  
                                    {1985, 10, 27,11, 0},
                                    {2015, 10, 21,16,29},
                                    {1955, 11, 12, 6, 0},
                                    {1885,  1,  1, 0, 0},
                                    {1885,  9,  2,12, 0} };


dateStruct departedTimes[8] = { {1985, 10, 26, 1, 20},
                                {1955, 11, 12, 22, 4},
                                {1985, 10, 26,  1,34},
                                {1885,  9,  7,9,10},
                                {1985, 10,26,11,35},
                                {1985,10,27,2,42},
                                {1955, 11,12,21,44},
                                {1955, 11, 13, 12, 0} };



                               


int8_t autoTime = 0; // selects the above array time


uint8_t autoInterval = 1; // array element of autoTimeIntervals[], set's time between automatically displayed times

const uint8_t autoTimeIntervals[5] = {0, 5, 15, 30, 60}; // time options, first must be 0, this is the off option.


// dest sept 2 1885 8:00
// 10/21/2015 16:29
//
// Some globals
clockDisplay* displaySet; // the display being updated during setting
RTC_DS3231 rtc;



///////////////////////////////////////

//const uint8_t monthDays [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };
const uint8_t monthDays [] = { 31,28,31,30,31,30,31,31,30,31,30,31 };


bool x; // for tracking second change
bool y;// for tracking second change
uint8_t timeout = 0; // for tracking idle time in menus, reset to 0 on each button press


//////////////////////////////functions /////////////////

bool loadAutoInterval(){
// loads the autoInterval
  autoInterval = EEPROM.read(autoIntervalAddr) ; // first location has autoInterval
  if( autoInterval > 5 ){ 
    autoInterval = 1; // default to first valid option if invalid read
    //Serial.println("BAD AUTO INT");
    return false;
  }

  return true;



}
void saveAutoInterval(){
// saves autoInterval
  EEPROM.update(autoIntervalAddr, autoInterval);
}

void displayHighlight(int& number){
// Turns on the currently selected display during setting
// Also sets the displaySet pointer to the display being updated 

  
  switch(number){
    case 0: // Destination Time
      destinationTime.on();
      destinationTime.setColon(false);  
      destinationTime.show();              
      presentTime.off();
      departedTime.off();
      displaySet = &destinationTime;
      break;
    case 1: // Presetnt Time
      destinationTime.off();                
      presentTime.on();
      presentTime.setColon(false);
      presentTime.show();
      departedTime.off();                
      displaySet = &presentTime;
      break;
    case 2: // Last Time Departed
      destinationTime.off();                
      presentTime.off();
      departedTime.on();
      departedTime.setColon(false);
      departedTime.show();
      displaySet = &departedTime;
      break;

    case 3: // auto enable
      destinationTime.showOnlySettingVal("PRE", -1, true); // display end, no numbers, clear rest of screen
      presentTime.showOnlySettingVal("SET", -1, true); // display end, no numbers, clear rest of screen
      destinationTime.on();
      presentTime.on();
     // destinationTime.setColon(false);  
      //destinationTime.show();              
      //presentTime.off();
      departedTime.off();
      displaySet = NULL;
      break;
    case 4: //Brt
      destinationTime.showOnlySettingVal("BRI", -1, true); // display end, no numbers, clear rest of screen
      presentTime.showOnlySettingVal("GHT", -1, true); // display end, no numbers, clear rest of screen
      destinationTime.on();
      presentTime.on();
     // destinationTime.setColon(false);  
      //destinationTime.show();              
      //presentTime.off();
      departedTime.off();
      displaySet = NULL;
      break;              
    case 5: //end
      destinationTime.showOnlySettingVal("END", -1, true); // display end, no numbers, clear rest of screen
      destinationTime.on();
      
      destinationTime.setColon(false);  
      //destinationTime.show();              
      presentTime.off();
      departedTime.off();
      displaySet = NULL;
      break;      
  }
}


void displaySelect(int& number){
// Selects which display to update using Up/Down buttons after inital Set button press

        if(digitalRead(buttonUp)){
            number++;
            timeout = 0; //button pressed, reset timeout
            if(number == (6))
              number = 0;
              displayHighlight(number); // Show only the selected display and set pointer to it
             while(digitalRead(buttonUp)){ // still pressed? hold until release
              timeout = 0;
              delay(10);
              }          
            }
        
         if(digitalRead(buttonDown)){
            number--;
            timeout = 0; //button pressed, reset timeout            
            if(number == - 1)
              number = 5;
              displayHighlight(number); // Show only the selected display and set pointer to it
             while(digitalRead(buttonDown)){ // still pressed? hold until release
              timeout = 0;
              delay(10);
              }
          } 
  } //displaySelect

void setUpdate(uint16_t& number, int field){
// Shows only the field being updated
  
      switch(field){
      case 0:
        displaySet->showOnlyMonth(number);
      break;
      case 1:
        displaySet->showOnlyDay(number);
      break;
      case 2:
        displaySet->showOnlyYear(number);
      break;
      case 3:
        displaySet->showOnlyHour(number);
      break;
      case 4:
        displaySet->showOnlyMinute(number);
      break;            
      
    }
  
  }

void brtButtonUpDown(clockDisplay* displaySet){

int8_t number = displaySet->getBrightness();

// Store actual brightness as 0 to 15, but only show 5 levels, starting at 3
// Update the clockDisplay and show the value
        
        if(digitalRead(buttonUp)){
          
            number = number + 3;
            timeout = 0; //button pressed, reset timeout
            if(number > 15)
              number = 3;
              displaySet->setBrightness(number);
              displaySet->showOnlySettingVal("LVL", number / 3, false);
             while(digitalRead(buttonUp)){ // still pressed? hold until release
              timeout = 0;
              delay(10);
              }          
            }
        
         if(digitalRead(buttonDown)){
            number = number - 3;
            timeout = 0; //button pressed, reset timeout            
            if(number <= 0)
              number = 15;
              displaySet->setBrightness(number);
              displaySet->showOnlySettingVal("LVL", number / 3, false);
             while(digitalRead(buttonDown)){ // still pressed? hold until release
              timeout = 0;
              delay(10);
              }
          } 
  
  }

  
void buttonUpDown(uint16_t& number,  uint8_t field, int year = 0, int month = 0){

// number - a value we're updating
// displayNum - the display number being modified
// field - field being modified, this will be displayed on displayNum as it is updated

// displayNum:
//  0 Destination Time
//  1 Present Time
//  2 Last Time Departed

// field: 0 month, 1 day, 2 year, 3 hour, 4 minute

    int upperLimit;
    int lowerLimit;



      switch(field){
        case 0: //month
          upperLimit = 12;
          lowerLimit = 1;
          break;
        case 1: //day
          upperLimit = daysInMonth(month, year);
          lowerLimit = 1;
          break;
        case 2: //year
          if(displaySet->isRTC() == 1){ //year is limited for RTC, RTC is on display 1
            upperLimit = 2099;
            lowerLimit = 2019;
          } else {
            upperLimit = 9999;
            lowerLimit = 0;
          }
          break;
        case 3: //hour
          upperLimit = 23;
          lowerLimit = 0;
          break;          
        case 4: //minute
          upperLimit = 59;
          lowerLimit = 0;
          break;
          
        
        }

   
      uint16_t bCount = 0;

      unsigned long start = millis();

      if(digitalRead(buttonUp) || digitalRead(buttonDown) ){

        while(digitalRead(buttonUp)){

           timeout = 0;

           if(bCount == 0){ //millis() - start < 500 && 
            // Instantly respond to first press
              start = millis();
              bCount++;


              number++;

              if(number == (upperLimit + 1))
                number = lowerLimit;
              
              setUpdate(number, field);
              delay(10); //debounce
               
            }



            if(bCount < 7 && abs(millis() - start) > 500) {
              // Go slow if held
              start = millis();
              bCount++;
      
              number++;
              
              if(number == (upperLimit + 1))
                number = lowerLimit;
                              
              setUpdate(number, field);
            }


            if(bCount >= 7 && bCount < 60 && abs(millis() - start) > 100){
              //Go faster if held longer
              bCount++;
              start = millis();
              
              number++;

              if(number == (upperLimit + 1))
                number = lowerLimit;
              
              setUpdate(number, field);         
              
              }

             if(bCount >= 60 && abs(millis() - start) > 10){
              // Go very fast 
              //bCount++; // no need to count any more
              start = millis();
              
              number++;
              
              if(number == (upperLimit + 1))
                number = lowerLimit;
                
              setUpdate(number, field); 
             }
           }



        
         while(digitalRead(buttonDown)){
         
           if(bCount == 0){ //millis() - start < 500 && 
            // Instantly respond to first press
              start = millis();
              bCount++;


             number--;

              if(number == lowerLimit - 1)
                number = upperLimit;
              
              setUpdate(number, field);
              delay(10); //debounce
               
            }

            if(bCount < 7 && abs(millis() - start) > 500) {
              // Go slow if held
              start = millis();
              bCount++;


             number--;

              if(number == lowerLimit - 1)
                number = upperLimit;
              
              setUpdate(number, field);
            }


            if(bCount >= 7 && bCount < 60 && abs(millis() - start) > 100){
              //Go faster if held longer
              start = millis();
              bCount++;

              number--;

              if(number == lowerLimit - 1)
                number = upperLimit;
              
              setUpdate(number, field);    
              
              }

             if(bCount >= 60 && abs(millis() - start) > 10){
              // Go very fast 

              start = millis();

              number--;

              if(number == lowerLimit - 1)
                number = upperLimit;
              
              setUpdate(number, field);
             }

          
          } //while digitalread        
        
        }
} //updown

bool isLeapYear(int year){
// Determine if provided year is a leap year.
  if(year%4 == 0){
     if(year%100 == 0 ){
       if(year%400 == 0 ){
        return true;
      }else{
        return false;
        }
    }else{
      return true; 
      }
   }
    else{
      return false;
    }
}//isLeapYear

int daysInMonth(int month, int year){
  // Find number of days in a month

  if(month == 2 && isLeapYear(year)){
    return 29;
  }

  return monthDays[month - 1];
}//daysInMonth


void RTCClockOutEnable(){
  // enable the 1Hz RTC output
  uint8_t readValue =0;
  Wire.beginTransmission(0x68);
  Wire.write((byte)0x0E); // select control register
  Wire.endTransmission();
  
  Wire.requestFrom(0x68, 1);
  readValue = Wire.read();
  
  readValue = readValue & B11100011; // enable squarewave and set to 1Hz,
  // Bit 2 INTCN - 0 enables OSC
  // Bit 3 and 4 - 0 0 sets 1Hz
   
  
  Wire.beginTransmission(0x68);
  Wire.write((byte)0x0E); // select control register
  Wire.write(readValue);
  Wire.endTransmission();
 
  
  }

bool checkTimeOut(){
// Call frequently while whiting for button press, increments timeout each second, returns true on when maxtime reached.
      y = digitalRead(secondIn);
      if(x != y){
        x = y;
        digitalWrite(statusOut, !y); // update status LED
        if(y == 0){
          timeout++;
        }

      }

      if(timeout >= maxTime){
        return true; // timed out
        }

      return false;
}

void animate(){
// Show month after a short delay
  
  destinationTime.showAnimate1();
  presentTime.showAnimate1();
  departedTime.showAnimate1();
  delay(80);
  destinationTime.showAnimate2();
  presentTime.showAnimate2();
  departedTime.showAnimate2();
  
  
  }

void allLampTest(){
// Activate lamp test on all displays and turn on     
  destinationTime.lampTest();
  presentTime.lampTest();
  departedTime.lampTest();
  destinationTime.on();  
  presentTime.on();
  departedTime.on();
}

void allOff(){
  destinationTime.off();
  presentTime.off();
  departedTime.off();
  }


        
void waitForButtonSetRelease(){
  // Do nothing and wait for buttonSet to be released
  while(digitalRead(buttonSet)){
    timeout = 0; // a button was pressed
    delay(100); //wait for release
  }
  return;
}


void doGetBrightness(clockDisplay* displaySet){
// Adjust the brightness of the selected displaySet and save


        allLampTest(); //turn on all the segments        



        
        displaySet->showOnlySettingVal("LVL", displaySet->getBrightness() / 3, false);

        while(!checkTimeOut() && !digitalRead(buttonSet)){
          brtButtonUpDown(displaySet); 
          delay(100);
        } 

;



        if(!checkTimeOut()){ // only if there wasn't a timeout
          displaySet->save();
          displaySet->showOnlySave();
          delay(1000);
          waitForButtonSetRelease();        
          allLampTest(); //turn on all the segments
        }
}

void autoTimesButtonUpDown(){



        
        if(digitalRead(buttonUp)){
          
            autoInterval++;
  
            timeout = 0; //button pressed, reset timeout
            if(autoInterval > 4) 
              autoInterval = 0;

            destinationTime.showOnlySettingVal("INT", autoTimeIntervals[autoInterval], true);

            if(autoTimeIntervals[autoInterval] == 0){
              presentTime.showOnlySettingVal("CUS", -1, true);
              departedTime.showOnlySettingVal("TOM", -1, true);
              departedTime.on();
              } else {
                departedTime.off();
                presentTime.showOnlySettingVal("MIN", -1, true);
                
                
                }                    
        
             while(digitalRead(buttonUp)){ // still pressed? hold until release
              timeout = 0;
              delay(10);
              }          
            }
        
         if(digitalRead(buttonDown)){
            autoInterval--;

            timeout = 0; //button pressed, reset timeout            
            if(autoInterval == 255) // goes to 255, not 0, since it's unsigned
              autoInterval = 4;


            destinationTime.showOnlySettingVal("INT", autoTimeIntervals[autoInterval], true);

           // Serial.println(autoInterval,DEC);
            
            if(autoTimeIntervals[autoInterval] == 0){
              presentTime.showOnlySettingVal("CUS", -1, true);
              departedTime.showOnlySettingVal("TOM", -1, true);
              departedTime.on();
              } else {
                departedTime.off();
                presentTime.showOnlySettingVal("MIN", -1, true);
                
                }   
                         
             while(digitalRead(buttonDown)){ // still pressed? hold until release
              timeout = 0;
              delay(10);
              }
          } 
  
  }


void doGetAutoTimes(){
// Set the auto times setting  
        
        destinationTime.showOnlySettingVal("INT", autoTimeIntervals[autoInterval], true);
        
        presentTime.on(); 
        if(autoTimeIntervals[autoInterval] == 0){
              presentTime.showOnlySettingVal("CUS", -1, true);
              departedTime.showOnlySettingVal("TOM", -1, true);
              departedTime.on();

        } else {
          departedTime.off();
          presentTime.showOnlySettingVal("MIN", -1, true);
        }  

        while(!checkTimeOut() && !digitalRead(buttonSet)){
          autoTimesButtonUpDown(); 
          delay(100);
        } 

        if(!checkTimeOut()){ // only if there wasn't a timeout
          
          presentTime.off();
          departedTime.off(); 
          destinationTime.showOnlySave();
          saveAutoInterval();
          delay(1000);
          waitForButtonSetRelease();        
        }
}



void setup () {
  Serial.begin(9600);
  delay(1000); // wait for console opening


// set up i/o
  pinMode(buttonDown,INPUT);
  pinMode(buttonUp,INPUT);
  pinMode(buttonSet,INPUT);
  pinMode(secondIn,INPUT); // for monitoring seconds
  //pinMode(4,OUTPUT); //debug
  pinMode(statusOut, OUTPUT); // Second status LED


unsigned long resetTime = millis();;
while(digitalRead(buttonSet)){
  
  if( millis() - resetTime > 5000){
   // Serial.println("RESET");

//Clear the eeprom, clock can't be reset or cleared via software.
      for(int16_t c = 0; c < 1024; c++){ 
      //  Serial.print("Erasing: ");
      //  Serial.println(c,DEC);
        EEPROM.update(c, 0xFF);
      }


    while(digitalRead(buttonSet)){
      // wait for release
      }

  }
  
  
  }


  bool validLoad = true;

  





// RTC setup
  if (! rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    // Lost power and battery didn't keep time
    rtc.adjust(DateTime(2019, 1, 1, 0, 0, 0));
  }

  RTCClockOutEnable(); // Turn on the 1Hz output

//Wire.begin();

// Start the displays by calling begin()

  presentTime.begin();
  destinationTime.begin();
  departedTime.begin();

  presentTime.setRTC(true); // configure it as a display that will hold real time


// corrupt eeprom to test checksup/reset
//EEPROM.update(0x10,0x0f);
//EEPROM.update(0x18,0x0f);
//EEPROM.update(0x20,0x0f);
    if(!destinationTime.load()){
      validLoad = false;
      // Set valid time and set if invalid load
      // 10/26/1985 1:21
      destinationTime.setMonth(10);
      destinationTime.setDay(26);
      destinationTime.setYear(1985);
      destinationTime.setHour(1);
      destinationTime.setMinute(21);
      departedTime.setBrightness(15);
      destinationTime.save();
    }

  if(!departedTime.load()){
    validLoad = false;
    // Set valid time and set if invalid load
    // 10/26/1985 1:20
    departedTime.setMonth(10);
    departedTime.setDay(26);
    departedTime.setYear(1985);
    departedTime.setHour(1);
    departedTime.setMinute(20); 
    departedTime.setBrightness(15);
    departedTime.save();
  }

  if(!presentTime.load()){// Time isn't saved here, but other settings are
    validLoad = false;
    presentTime.setBrightness(15);
    presentTime.save();
  }

  
  if(!loadAutoInterval()){ // load saved settings
    validLoad = false;
   // Serial.println("BAD AUTO INT");
    EEPROM.update(autoIntervalAddr, 1); // default to first option
  
  }
  
  
  
  if(autoTimeIntervals[autoInterval]){ // non zero interval, use auto times
    destinationTime.setFromStruct(&destinationTimes[0]); //load the first one
    departedTime.setFromStruct(&departedTimes[0]);
    
    }


  if(!validLoad){
    // Show message
      destinationTime.showOnlySettingVal("RE", -1, true);
      presentTime.showOnlySettingVal("SET", -1, true);
      delay(5000);
      allOff();
      delay(1000);
    }


  
  presentTime.setDateTime(rtc.now()); // Load the time for initial animation show
  animate();
}


// variables to hold time during setting

int displayNum;  // selected display
// unsigned long then = millis();

bool autoTrack = false;

int8_t minPrev; // track previous minute

void loop () {


  





  //uint16_t uint8_t 
  // time display update
  y = digitalRead(secondIn);
  
  if(y != x){ // different on half second
    
    DateTime dt = rtc.now();
    presentTime.setDateTime(dt); // Set the current time in the display
    //presentTime.setDateTime(rtc.now()); // Set the current time in the display
   
    if(y==0){ // flash colon on half seconds, lit on start of second

/////////////////////////////

    // 
    // auto display some times


//Serial.print(dt.minute());
//Serial.print(":");
//Serial.println(dt.second());   
// Do this on previous minute:59
    if(dt.minute() == 59){
      minPrev = 0;
    } else {
      
      minPrev = dt.minute() + 1;
      
      }

    
// only do this on second 59, check if it's time to do so
      if( dt.second() == 59 &&autoTimeIntervals[autoInterval] && ( minPrev % autoTimeIntervals[autoInterval] == 0 ) ){ 

    //Serial.println("DO IT");


    
     
        if( !autoTrack ){
          autoTrack = true; // Already did this, don't repeat
// do auto times
        autoTime++;
        if(autoTime > 4 ){ // currently have 5 times
          autoTime = 0;
        }

        // Show a preset dest and departed time
        // then = millis(); ///////////////// ????????????????????????????

        destinationTime.setFromStruct(&destinationTimes[autoTime]);
        departedTime.setFromStruct(&departedTimes[autoTime]);

        destinationTime.setColon(true);
        presentTime.setColon(true);
        departedTime.setColon(true);

        allOff();


// Blank on second 59, display when new minute begins

        while(digitalRead(secondIn) == 0){ // wait for the complete of this half second
          
         // Wait for this half second to end
          }
        while(digitalRead(secondIn) == 1){ // second on next low
          
         // Wait for the other half to end
          }


        dt = rtc.now(); // New time by now
        presentTime.setDateTime(dt); //will be at next minute

        animate(); // show all with month showing last

// end auto times
      
        }

      

      
      }else{
        autoTrack = false;
      }

  


//////////////////////////////

      destinationTime.setColon(true);
      presentTime.setColon(true);
      departedTime.setColon(true);

      
    } else { //colon

      
      destinationTime.setColon(false);
      presentTime.setColon(false);
      departedTime.setColon(false);


 
    } // colon

    presentTime.show(); // update display with object's time
    destinationTime.show();
   //destinationTime.showOnlySettingVal("SEC", dt.second(), true); // display end, no numbers, clear rest of screen
    departedTime.show();
    
    digitalWrite(statusOut, !y); // built-in LED shows system is alive, invert to light on start of new second
    
    x = y; // remember it 
  }
// Time update ends

//Check buttons for setting

  if(digitalRead(buttonSet)){

    // Set button pressed, get display

    displayNum = 0; // this is really setting function number

// Load the times
    destinationTime.load(); // this wipes out currently displayed times if on auto, brightness may do a save, so need stored times any
    departedTime.load();
    
    displayHighlight(displayNum);
      
    waitForButtonSetRelease();

    timeout = 0; // Reset on each press

    while(!checkTimeOut() && !digitalRead(buttonSet)){
      // Keep looking for up down button presses until set is pressed again
      displaySelect(displayNum);
      delay(50);
    }

    // Have a selected display pointed to by displaySet if displayNum 0-2

    if(displayNum <= 2){ // Doing display times

      uint16_t yearSet; // hold new times that will be set, all need to be same type since they're passed by ref
      uint16_t monthSet;
      uint16_t daySet;
      uint16_t minSet;
      uint16_t hourSet;
  
      if(displaySet->isRTC() == 1 ){ // this is the RTC, get the current time time.
        
        DateTime currentTime = rtc.now();
        
        yearSet = currentTime.year();
        monthSet = currentTime.month();
        daySet = currentTime.day();
        minSet = currentTime.minute();
        hourSet = currentTime.hour();
  
      } else {
        // non RTC, get the time info from the object
        yearSet = displaySet->getYear();
        monthSet = displaySet->getMonth();
        daySet = displaySet->getDay();
        minSet = displaySet->getMinute();
        hourSet = displaySet->getHour();
       
        }


// Get year
/*
Fields
0 - month
1 - day
2 - year
3 - hour
4 - min
*/
    setUpdate(yearSet, 2); // show only the year field - value,display field 
    waitForButtonSetRelease();
    while(!checkTimeOut() && !digitalRead(buttonSet)){
      buttonUpDown(yearSet, 2); // number to adjust, field
      delay(100);
    }  

// Get month

    setUpdate(monthSet,  0 );
    waitForButtonSetRelease();
    while(!checkTimeOut() && !digitalRead(buttonSet)){
      buttonUpDown(monthSet,  0); // setting, which display, whcih field, starting from left
      delay(100);
    }   


// Get day

    setUpdate(daySet,  1 );
    waitForButtonSetRelease();    
    while(!checkTimeOut() && !digitalRead(buttonSet)){
      buttonUpDown(daySet,  1, yearSet, monthSet); // this depends on the month and year
      delay(100);
    }   
 
   
// Get hour

    setUpdate(hourSet,  3 );
    waitForButtonSetRelease();      
    while(!checkTimeOut() && !digitalRead(buttonSet)){
      buttonUpDown(hourSet,   3);
      delay(100);
    }   

// Get minute
  
    setUpdate(minSet, 4 );
    waitForButtonSetRelease();   
    while(!checkTimeOut() && !digitalRead(buttonSet)){
      delay(100);
      buttonUpDown(minSet, 4);
    }

// Have new date & time

    if(!checkTimeOut()){ // Do nothing if there was a timeout waiting for button presses
      
      if(displaySet->isRTC() == 1){ // this is the RTC display, set the RTC as quickly as possible, as soon as set button pressed
        rtc.adjust(DateTime(yearSet, monthSet, daySet, hourSet, minSet, 0));
        
      } else { // Not rtc, setting a static display, turn off auto changing
        autoInterval = 0; // set to the 0 interval, disables auto
        saveAutoInterval(); // save it
        }
      ///////////////////////////////////}

      //displaySet->off(); // turn it off
      displaySet->showOnlySave(); // Show a save message

      waitForButtonSetRelease();// button still pressed? wait until release to prevent re-entering set mode

      // wait to do this, update rtc as fast as possible
      // update the object
      displaySet->setMonth(monthSet);
      displaySet->setDay(daySet);
      displaySet->setYear(yearSet);
      displaySet->setHour(hourSet);
      displaySet->setMinute(minSet);
      displaySet->save(); // save to eeprom
      delay(1000);
   
    } // checktimeout

    //displaySet->off(); // turn it off, prevents minute from showing for 1 second

    } else if(displayNum == 4){
        // Get brightness settings
        
        waitForButtonSetRelease();


        
        if(!checkTimeOut())
          doGetBrightness(&destinationTime);
        if(!checkTimeOut())
          doGetBrightness(&presentTime);
        if(!checkTimeOut())
          doGetBrightness(&departedTime);
        allOff();
    } else if(displayNum == 3){ // auto times
      waitForButtonSetRelease();
      int8_t number = 0;
      doGetAutoTimes();


    } else {// if(displayNum < 3)
      // END option, turn off displays, do nothing. 
      allOff();
    }
  



    // Return dest/dept displays to where they should be
    // Menuing system wipes out auto times
    if( autoTimeIntervals[autoInterval] == 0 ){
        destinationTime.load();
        departedTime.load();
      } else {
        destinationTime.setFromStruct(&destinationTimes[autoTime]);
        departedTime.setFromStruct(&departedTimes[autoTime]);
     }






    // Done, turn off displays, then turn on
    allOff();
    
    delay(1000);
    presentTime.setDateTime(rtc.now()); // Set the current time in the display, set times are 2+ seconds old
// all displays on and show 
    animate(); // show all with month showing last
    // then = millis(); // start count to prevent double animate if it's been too long
    }


////////Done setting


delay(10);



}
