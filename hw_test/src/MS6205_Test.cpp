//
// MC6205 (MS6205) Hardware Test (c) J.Postert 5/2021 V1.0
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
//

#include <Arduino.h>

char SERDEBUG = 0;

//#define internalPCB   // which PCB to use, internal MC6205 housing or the connector one. The postert MC6205 PCB's are meant here. Define your own GPIO's if you using your own

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

const char Positions[] = {32,33,34,48,49,50,64,65,66,80,81,82,96,97,98,112,113,114,128,129,130,144,145,146};

const char frame[]  = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,31,47,63,79,95,111,127,143,159,158,157,
                       156,155,154,153,152,151,150,149,148,147,146,145,144,128,112,96,80,64,48,32,16
                      };



// Prototypes
void updateShiftRegister(uint8_t c);
void sendData();
void setPosition();
void DisplayClear();

void DisplayHelloWorld(void);
void TestDisplayNumbers(void);
void DisplayMessage(String Message, int startPos);
void ScreenTest(void);
void DisplayCharacterSet(void);
void displayNumber(char number, char shift);
void DisplayText(void);
void hideCursor(void);




void setup() {
  if(SERDEBUG==1)
    Serial.begin(115200);

  pinMode(clearScreenPin, OUTPUT);
  pinMode(cursorPin, OUTPUT);
  pinMode(updatePin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
}

void loop() {
  DisplayClear();
  ScreenTest();
  delay(3000);
  DisplayClear();
  DisplayCharacterSet();
  delay(3000);
  DisplayClear();
  DisplayHelloWorld();
  delay(3000);
  DisplayClear();
  TestDisplayNumbers();
  delay(3000);
  DisplayText();
  delay(3000);
}


// fill screen with blocks (good to see pixel errors)
void ScreenTest(void)
{
  for (unsigned char i=-1; i!=MAXCHAR; i++) {
    updateShiftRegister(i);
    setPosition();
    updateShiftRegister(95);
    sendData();
    delay(1);
  }
 hideCursor();
}


// test numbers
// "0" = Pos1, "4" = Pos2, "9" = Pos3, "13" = Pos4
// displayNumber(number, position)
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

void DisplayText()
{
  char tmpbuff[160];
  sprintf(tmpbuff, "The lipogrammatic novel Ella Minnow Pea by Mark Dunn is built entirely around the \"quick brown fox\" pangram and its inventor. It depicts a fictional country");
  DisplayMessage(tmpbuff, 0);
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
void displayNumber(char number, char position)
{
  for (int i=0; i< 21; i++) 
  {
    updateShiftRegister((Positions[i]+position));
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
