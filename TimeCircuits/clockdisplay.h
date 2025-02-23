/*




*/


const uint16_t alphaChars [] = {0x0, 0x8CCB, 0x2E3B, 0xF3, 0x263B, 0x88F3, 0x88C3, 0x8FB, 0x8CC8, 0x2233, 0x478, 0x90C4, 0xF0, 0x5CC, 0x15C8, 0x4FB, 0x8CC3, 0x14FB, 0x9CC3, 0x88BB, 0x2203, 0x4F8, 0x40C4, 0x54C8, 0x5104, 0x2104, 0x4037, 0x4FB, 0x40C, 0x8C73, 0x8C3B, 0x8C88, 0x88BB, 0x88FB, 0x2007, 0x8CFB, 0x8CBB };
const uint8_t numDigs [] = {B00111111, B00000110, B01011011, B01001111, B01100110, B01101101, B01111101, B00000111, B01111111, B01101111, B01111001};
const char months[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};


struct dateStruct{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  };

class clockDisplay{


  public:
    clockDisplay(uint8_t address, int saveAddress = -1);
    void begin();
	  void lampTest();
    void on();
    void off();
    void clear();
    uint8_t setBrightness(uint8_t level);


    uint8_t getBrightness();

    void setMonth(int monthNum);
    void setDay(int dayNum);
    void setYear(uint16_t yearNum);
    void setHour(uint16_t hourNum);
    void setMinute(int minNum);
    void setColon(bool col);
    void colonOn();
    void colonOff();
	
    uint8_t getMonth();
    uint8_t getDay();
    uint16_t getYear();
    uint8_t getHour();
    uint8_t getMinute();
    
    
    void setRTC(bool rtc); // make this an RTC display
    bool isRTC();
    
    void AM(); 
    void PM();    


    void showOnlyMonth(int monthNum); // Show only the supplied month, do not modify object's month
    void showOnlyDay(int dayNum);
    void showOnlyHour(int hourNum);
    void showOnlyMinute(int minuteNum); 
    void showOnlyYear(int yearNum);
    void showOnlySave();
    void showOnlySettingVal(const char* setting, int8_t val = -1, bool clear = false);
       
    void setDateTime(DateTime dt); // Set object date & time using a DateTime
    void setFromStruct(dateStruct* s);
    
    DateTime getDateTime(); // Return object's date & time as a DateTime
    
    void show();
	  void showAnimate1();
    void showAnimate2();
    
    bool save();
    bool load();
  
    private:
    uint8_t _address;
	  int _saveAddress = -1;
    uint16_t _displayBuffer[8]; // Segments to make current time.

    uint16_t _year = 2019; // keep track of these
    uint8_t _month = 1;
    uint8_t _day = 1;
    uint8_t _hour = 0;
    uint8_t _minute = 0;
    bool _colon = false; // should colon be on?
    bool _rtc = false; // will this be displaying real time
    uint8_t _brightness = 15;
    
    uint8_t getLED7SegChar(uint8_t value);
    uint16_t getLEDAlphaChar(char value);
    
    uint16_t makeNum(uint8_t num);

    uint16_t makeAlpha(uint8_t value);

    void clearDisplay(); // clears display RAM
    void directCol(int col, int segments); // directly writes column RAM
    void directAM(); 
    void directPM();



    
  };
