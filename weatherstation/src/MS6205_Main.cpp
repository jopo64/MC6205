
//
// MC6205 (MS6205) (c) J.Postert 3/2021 V1.0
//
// Russian gas discharge display with 160 characters 10x16 organized
// the MC6205 has a very limited character-set of only 96 chars, most cyrillic (wasn't usefull for me)
// the code used here, needs a modified characterset-PROM (2716, 2732 or 2816) with US-ASCII + ISO-Latin-1 font enhancements
// MC6205 supports 96 characters, but it has an easter egg... a hidden charset block... 
// for using 128 chars, the hardware has to be changed a bit (one trace to cut and one wire to solder), then! BINgo 8 bit data instead of 7
//
// the charcter set is organized in 3 x 32 chars == 96, plus the hidden one in the EPROM which is in total 4 x 32 chars == 128
//
// some similar projects:
// https://dekatronpc.com/index.php/%D0%9C%D0%A16205
// https://hackaday.io/project/172342-mc6205-ms6205-display
// https://github.com/holzachr/MS6205-arduino-library
// https://radiokot.ru/forum/viewtopic.php?p=402815#p402815
// https://www.youtube.com/watch?v=_xHFLyv6XAk
// https://www.youtube.com/watch?v=5NktqEwgCs4
//
// ESP32: WLAN 2,4GHz only
//

#include <WiFi.h>



#define internalPCB   // which PCB to use, internal MC6205 housing or the connector one. The postert MC6205 PCB's are meant here. Define your own GPIO's if you using your own

#ifdef internalPCB
  // MC6205 hardware pins to Arduino (internal PCB)
  const char updatePin      = 19;   // 16B
  const char clearScreenPin = 5;    // 18A
  const char cursorPin      = 18;   // 16A

  // shift register pins
  const char latchPin       = 12;   // Latch  | Pin 12 of 74LS595 (ST_CP)
  const char clockPin       = 14;   // CLK    | Pin 11 of 74LS595 (SH_CP)
  const char dataPin        = 13;   // MOSI   | Pin 14 of 74LS595 (DS)
#else
  // MC6205 hardware pins to Arduino (connector PCB)
  const char updatePin      = 3;    // 16B
  const char clearScreenPin = 19;   // 18A
  const char cursorPin      = 21;   // 16A

  // shift register pins
  const char latchPin       = 5;    // Latch  | Pin 12 of 74LS595 (ST_CP)
  const char clockPin       = 18;   // CLK    | Pin 11 of 74LS595 (SH_CP)
  const char dataPin        = 23;   // MOSI   | Pin 14 of 74LS595 (DS)
#endif


const char MAXCHAR  = 160;
const char ROWSIZE  = 16;

String MessageBuffer;

bool SwitchClockFlg = 0;
char BigClockCntr = 0;

unsigned long tMillis;
unsigned long tMillis_saved[10];  // array for saved milliseconds

char SERDEBUG = 0;                // debug infos to serial

// XXX   X   XXX  XXX
//   X  XX   X X  X X
//   X   X   X X  X X
// XXX   X   X X  XXX
// X     X   X X  X X
// X     X   X X  X X
// XXX  XXX  XXX  XXX

const char zero[]   = {95,95,95,95,0,95,95,0,95,95,0,95,95,0,95,95,0,95,95,95,95};
const char one[]    = {0,95,0,95,95,0,0,95,0,0,95,0,0,95,0,0,95,0,95,95,95};
const char two[]    = {95,95,95,0,0,95,0,0,95,95,95,95,95,0,0,95,0,0,95,95,95};
const char three[]  = {95,95,95,0,0,95,0,0,95,95,95,95,0,0,95,0,0,95,95,95,95};
const char four[]   = {95,0,95,95,0,95,95,0,95,95,95,95,0,0,95,0,0,95,0,0,95};
const char five[]   = {95,95,95,95,0,0,95,0,0,95,95,95,0,0,95,0,0,95,95,95,95};
const char six[]    = {95,95,95,95,0,0,95,0,0,95,95,95,95,0,95,95,0,95,95,95,95};
const char seven[]  = {95,95,95,0,0,95,0,0,95,0,0,95,0,0,95,0,0,95,0,0,95};
const char eight[]  = {95,95,95,95,0,95,95,0,95,95,95,95,95,0,95,95,0,95,95,95,95};
const char nine[]   = {95,95,95,95,0,95,95,0,95,95,95,95,0,0,95,0,0,95,95,95,95};

const char posNum[] = {32,33,34,48,49,50,64,65,66,80,81,82,96,97,98,112,113,114,128,129,130,144,145,146};

const char frame[]  = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,31,47,63,79,95,111,127,143,159,158,157,
                       156,155,154,153,152,151,150,149,148,147,146,145,144,128,112,96,80,64,48,32,16
                      };



// Prototypes
void displayNumber(char number, char shift);
void updateShiftRegister(uint8_t c);
void DisplayHelloWorld(void);
void TestDisplayNumbers(void);
void DisplayOneChar(void);
void DisplayMessage(String Message, int startPos);
void ScreenTest(void);
void DisplayCharacterSet(void);
void hideCursor(void);
void sendData();
void setPosition();
void setColon(void);
void DisplayClear();
void DisplayDateTime(void);
void DisplayDate(void);
void DisplayBigClock(void);
String utf8toIso1(String utf8string);

// WiFi stuff
extern void WiFi_init();
extern void NTP_printLocalTime(int offset, int mode);
extern void WeatherReport(void);


void setup() {
  if(SERDEBUG==1)
    Serial.begin(115200);

  pinMode(clearScreenPin, OUTPUT);
  pinMode(cursorPin, OUTPUT);
  pinMode(updatePin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  digitalWrite(cursorPin,HIGH);
  digitalWrite(updatePin,HIGH);
  digitalWrite(clearScreenPin,LOW);
  delay(30);
  digitalWrite(clearScreenPin,HIGH);

  // initialize WiFi stack
  WiFi_init();

  // weather info from openweathermap
  WeatherReport();
  delay(4000);
  DisplayClear();
  tMillis_saved[2] = millis()+15000;
}

void lloop() {
  DisplayClear();
  DisplayCharacterSet();
  delay(2000);
  DisplayClear();
  DisplayHelloWorld();
  delay(2000);
  DisplayClear();
  ScreenTest();
  delay(2000);
  DisplayClear();
  WeatherReport();
  delay(2000);
}

void loop() {

  tMillis = millis();
  char tmpbuff[160];

  if(tMillis >= tMillis_saved[4]+88000) {
    DisplayClear();
    DisplayHelloWorld();
    delay(2000);
    DisplayClear();
    tMillis_saved[4] = millis();
  }

  // some text
  if(tMillis >= tMillis_saved[3]+120000) {
    //sprintf(tmpbuff, "The lipogrammatic novel Ella Minnow Pea by Mark Dunn is built entirely around the \"quick brown fox\" pangram and its inventor. It depicts a fictional country");
    sprintf(tmpbuff, "Die Umlaute \'ä\',\'ö\',\'ü\' und \'ß\' gehören zu einer Besonderheit der deutschen Sprache. Sie erweitern das Alphabet um vier weitere Buchstaben. Wie schön.");
    DisplayMessage(utf8toIso1(tmpbuff), 0);
    delay(8000);
    DisplayClear();
    tMillis_saved[3] = millis();
  }
  
  // display charcter-set
  if(tMillis >= tMillis_saved[1]+96000) 
  {
    tMillis_saved[1] = millis();
    DisplayClear();
    DisplayCharacterSet();
    delay(4000);
    DisplayClear();
  }

  // switch clocks
  if(tMillis >= tMillis_saved[2]+10000) {
    SwitchClockFlg = !SwitchClockFlg;
    DisplayClear();
    BigClockCntr = 0;
    tMillis_saved[2] = millis();
  }

  // seconds update of clock
  if(tMillis >= tMillis_saved[0]+999)
  {
    tMillis_saved[0] = millis();
    if(SwitchClockFlg == 1)
      DisplayDateTime();
    else {
      DisplayBigClock();
    }
  }

  // Weather report
  if(tMillis >= tMillis_saved[5]+20000)
  {
    DisplayClear();
    WeatherReport();
    tMillis_saved[5] = millis();
    delay(6000);
    DisplayClear();
  }
}


// showtime
void DisplayDateTime(void) {
  NTP_printLocalTime(49,0);
}

void DisplayDate(void) {
  NTP_printLocalTime(1,1);
}

// show our character set 
void DisplayCharacterSet(void)
{
  for (int i=0; i < MAXCHAR; i++) {
    updateShiftRegister(i);
    setPosition();
    updateShiftRegister(i);
    sendData();
    delay(5);
  }
 hideCursor();
}


// show our character set 
void ScreenTest(void)
{
  for (int i=0; i < MAXCHAR; i++) {
    updateShiftRegister(i);
    setPosition();
    updateShiftRegister(95);
    sendData();
    delay(1);
  }
 hideCursor();
}


void DisplayOneChar(void)
{
  updateShiftRegister(1);
  setPosition();
  updateShiftRegister(25);
  sendData();
  updateShiftRegister(2);
  setPosition();
  updateShiftRegister(4);
  setPosition();
  updateShiftRegister(24);
  sendData();
  updateShiftRegister(6);
  setPosition();
}

// test numbers
// "0" = Pos1, "4" = Pos2, "9" = Pos3, "13" = Pos4  Use this for Clocks
// displayNumber(Number to be displayed at Position 0 - 159)
void TestDisplayNumbers(void)
{
  int dly = 200;
  displayNumber(0,0);
  delay(dly);
  displayNumber(1,4);
  delay(dly);
  displayNumber(2,9);
  delay(dly);
  displayNumber(3,13);
  
  delay(dly*5);
  displayNumber(4,0);
  delay(dly);
  displayNumber(5,4);
  delay(dly);
  displayNumber(6,9);
  delay(dly);
  displayNumber(7,13);
  
  delay(dly*5);
  displayNumber(8,0);
  delay(dly);
  displayNumber(9,4);
  delay(dly);
  displayNumber(0,9);
  delay(dly);
  displayNumber(0,13);
}


void DisplayBigClock(void)
{
  struct tm timeinfo;
 
  getLocalTime(&timeinfo);
  
  if(BigClockCntr++ <= 4) {
    if(BigClockCntr == 1) { // date needs no update every second, so one shot
      DisplayDate();        // show date in first row
      displayNumber(timeinfo.tm_hour/10%10,0);
      delay(50);
      displayNumber(timeinfo.tm_hour%10,4);
      delay(50);
      displayNumber(timeinfo.tm_min/10%10,9);
      delay(50);
      displayNumber(timeinfo.tm_min%10,13);
    }
  } 
  else
  {
    NTP_printLocalTime(1,0);          // show date and time in first row
    displayNumber(timeinfo.tm_min/10%10,0);
    delay(5);
    displayNumber(timeinfo.tm_min%10,4);
    delay(5);
    displayNumber(timeinfo.tm_sec/10%10,9);
    delay(5);
    displayNumber(timeinfo.tm_sec%10,13);
  }
  if(BigClockCntr > 12)
    BigClockCntr = 0;
}


//
// display free Text (utf8 possible)
//
void DisplayMessage(String Message, int startPos)
{
  for (int i=0; i<Message.length(); ++i) {
    updateShiftRegister(startPos+i);
    setPosition();
    updateShiftRegister(Message[i]-32);
    sendData();
  }
  hideCursor();
}

//
// display greetings
//
void DisplayHelloWorld(void)
{   
  DisplayMessage("Hello", 18);
  delay(300);
  DisplayMessage("World", 38);
  delay(500);
  DisplayMessage("-- this is --", 66);
  delay(400);
  DisplayMessage("MC6205", 85);
  delay(500);
  DisplayMessage("Greets from", 114);
  delay(250);
  DisplayMessage("Germany", 132);
  delay(500);
  for (int i=0; i<48; i++) {
    updateShiftRegister(frame[i]);
    setPosition();
    updateShiftRegister(127); // heart symbol
    sendData();
    delay(20);
  }
  hideCursor();
  delay(1000);
}



// generate big number to be displayed 
void displayNumber(char number, char shift)
{
  for (int i=0; i< 21; i++) 
  {
    updateShiftRegister((posNum[i]+shift));
    setPosition();
    if(number==0) updateShiftRegister(zero[i]);
    else if(number==1) updateShiftRegister(one[i]);
    else if(number==2) updateShiftRegister(two[i]);
    else if(number==3) updateShiftRegister(three[i]);
    else if(number==4) updateShiftRegister(four[i]);
    else if(number==5) updateShiftRegister(five[i]);
    else if(number==6) updateShiftRegister(six[i]);
    else if(number==7) updateShiftRegister(seven[i]);
    else if(number==8) updateShiftRegister(eight[i]);
    else if(number==9) updateShiftRegister(nine[i]);
    sendData();
  }
}

//  Update shift register
void updateShiftRegister(unsigned char c)
{
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, c);
  digitalWrite(latchPin, HIGH);
}

// update/strobe char
void sendData()
{ 
  digitalWrite(updatePin,LOW);
  digitalWrite(updatePin,HIGH);
}

// strobe cursor position
void setPosition()
{
  digitalWrite(cursorPin,LOW);
  digitalWrite(cursorPin,HIGH);
}

// clear screen 
void DisplayClear()
{
  digitalWrite(clearScreenPin,LOW);
  delay(30);
  digitalWrite(clearScreenPin,HIGH);
  hideCursor();
}

// kick cursor to outer space
void hideCursor(void)
{
  updateShiftRegister(160);
  setPosition();
}


//
// simple utf8 to iso latin-1 mapping and cleanup
//
String utf8toIso1(String utf8string) 
{
  for (int i=0; i < utf8string.length()-1; ++i) {
    char c   = utf8string[i];
    char cnv = utf8string[i+1];
    
    if (c == 0xC2) {
      utf8string.remove(i,1);
      if(cnv==0xa7) utf8string[i+1] = 148;         // utf8 §
      else if(cnv==0xb0) utf8string[i+1] = 149;    // utf8 °
      else if(cnv==0xb5) utf8string[i+1] = 150;    // utf8 µ
    }
    if (c == 0xC3) {
      if(SERDEBUG==1) {
        Serial.print("Content+1:");
        Serial.println(cnv,HEX);
      }

      if(cnv==0x80) utf8string[i+1] = 'A';       // utf8 À
      else if(cnv==0x81) utf8string[i+1] = 'A';  // utf8 À
      else if(cnv==0x82) utf8string[i+1] = 'A';  // utf8 À
      else if(cnv==0x83) utf8string[i+1] = 'A';  // utf8 Ã
      else if(cnv==0x84) utf8string[i+1] = 132;  // utf8 Ä
      else if(cnv==0x85) utf8string[i+1] = 'A';  // utf8 Å
      else if(cnv==0x86) utf8string[i+1] = 'A';  // utf8 Æ
      else if(cnv==0x87) utf8string[i+1] = 'C';  // utf8 Ç
      else if(cnv==0x88) utf8string[i+1] = 'E';  // utf8 È
      else if(cnv==0x89) utf8string[i+1] = 'E';  // utf8 É
      else if(cnv==0x8a) utf8string[i+1] = 'E';  // utf8 Ê
      else if(cnv==0x8b) utf8string[i+1] = 'E';  // utf8 Ë
      else if(cnv==0x8c) utf8string[i+1] = 'I';  // utf8 Ì
      else if(cnv==0x8d) utf8string[i+1] = 'I';  // utf8 Í
      else if(cnv==0x8e) utf8string[i+1] = 'I';  // utf8 Î
      else if(cnv==0x8f) utf8string[i+1] = 'I';  // utf8 Ï
      else if(cnv==0x92) utf8string[i+1] = 'O';  // utf8 Ò
      else if(cnv==0x93) utf8string[i+1] = 'O';  // utf8 Ó
      else if(cnv==0x94) utf8string[i+1] = 'O';  // utf8 Ô
      else if(cnv==0x95) utf8string[i+1] = 'O';  // utf8 Õ
      else if(cnv==0x96) utf8string[i+1] = 133;  // utf8 Ö
      else if(cnv==0x9c) utf8string[i+1] = 134;  // utf8 Ü
      else if(cnv==0x9f) utf8string[i+1] = 131;  // utf8 ß
      else if(cnv==0xa0) utf8string[i+1] = 135;  // utf8 à
      else if(cnv==0xa1) utf8string[i+1] = 136;  // utf8 á
      else if(cnv==0xa2) utf8string[i+1] = 137;  // utf8 â
      else if(cnv==0xa3) utf8string[i+1] = 'a';  // utf8 ã
      else if(cnv==0xa4) utf8string[i+1] = 128;  // utf8 ä
      else if(cnv==0xa5) utf8string[i+1] = 'a';  // utf8 å
      else if(cnv==0xa6) utf8string[i+1] = 'a';  // utf8 æ
      else if(cnv==0xa7) utf8string[i+1] = 144;  // utf8 ç
      else if(cnv==0xa8) utf8string[i+1] = 138;  // utf8 è
      else if(cnv==0xa9) utf8string[i+1] = 139;  // utf8 é
      else if(cnv==0xaa) utf8string[i+1] = 140;  // utf8 ê
      else if(cnv==0xab) utf8string[i+1] = 'e';  // utf8 ë
      else if(cnv==0xb9) utf8string[i+1] = 141;  // utf8 ù
      else if(cnv==0xba) utf8string[i+1] = 142;  // utf8 ú
      else if(cnv==0xbb) utf8string[i+1] = 143;  // utf8 û
      else if(cnv==0xbc) utf8string[i+1] = 130;  // utf8 ü
      else if(cnv==0xbd) utf8string[i+1] = 'y';  // utf8 ý
      else if(cnv==0xbf) utf8string[i+1] = 'y';  // utf8 ÿ
      else if(cnv==0xb2) utf8string[i+1] = 142;  // utf8 ò
      else if(cnv==0xb3) utf8string[i+1] = 143;  // utf8 ó
      else if(cnv==0xb4) utf8string[i+1] = 144;  // utf8 ô
      else if(cnv==0xb5) utf8string[i+1] = 'o';  // utf8 õ
      else if(cnv==0xb6) utf8string[i+1] = 129;  // utf8 ö

      if(SERDEBUG==1) {
        Serial.print("utf8: ");
        Serial.print(c,HEX);
        Serial.print(" : ");
        Serial.println(utf8string.charAt(i+1),HEX);
      }

      utf8string.remove(i,1);    // remove C3

    } // remove all other utf >= C4
    if (c==0xC4 || c==0xC5 || c==0xC6 || c==0xC7 || c==0xC8 || c==0xC9) {
      utf8string.remove(i,2);
    }
  }
  
  return utf8string;
}