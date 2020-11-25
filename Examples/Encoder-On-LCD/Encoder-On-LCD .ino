
#include <Wire.h>
#include "Adafruit_MCP23017.h"  // https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library
#include <LiquidCrystal_I2C.h>

// setup the port expander
Adafruit_MCP23017 MCP;
LiquidCrystal_I2C lcd(0x27,20,4);    // set the LCD address to 0x27 for a 16 chars and 2 line display

const int CW  = 1;
const int CCW = 2;
const int EncNo = 0;
const int Dir = 1;

boolean change=false;        // goes true when a change in the encoder state is detected
int butPress = 150;          // stores which button has been pressed
int encSelect[2] = {101, 0}; // stores the last encoder used and direction {EncID, 1=CW or 2=CCW}

const int EncNum = 5;  // number of rotary PrevEncoders

// encoder pin connections to MCP23017
const int EncPins[EncNum][2] = {// DT pin, CLK pin
  {6,5},    // Enc 1 --> GPA6, GPA5
  {3,2},    // Enc 2 --> GPA3, GPA2
  {10,9},   // Enc 3 --> GPB2, GPB1
  {12,13},  // Enc 3 --> GPB4, GPB5
  {15,1},   // Enc 4 --> GPB7, GPA1
 
};  

const int ButPins[EncNum] = {// Switch pin
   7,   // Enc 1 --> GPA7
   4,   // Enc 2 --> GPA4
   8,   // Enc 3 --> GPB0
   11,  // Enc 4 --> GPB5
   14}; // Enc 5 --> GPB6

int Counters[EncNum];  // keep aech encoder position
char list[] = {'A','B','C','D','E','F','G','H','I','J','K'};
String menu[]= {"X Axis",
               "Y Axis",
               "Z Axis",
               "Feed Rate",
               "Spindle Speed"};


// arrays to store the previous value of the PrevEncoders and buttons
unsigned char PrevEncoders[EncNum];
unsigned char PrevButton[EncNum];

// read the rotary encoder on pins X and Y, output saved in encSelect[EncID, direct]
unsigned char readEnc(const int *pin, unsigned char prev, int EncID) {

  unsigned char encA = MCP.digitalRead(pin[0]);    // Read encoder pins
  unsigned char encB = MCP.digitalRead(pin[1]);

  if((!encA) && (prev)) { 
    encSelect[EncNo] = EncID;
    if(encB) {
      encSelect[Dir] = CW;  // clockwise
      Counters[EncID]++;
    }
    else {
      encSelect[Dir] = CCW;  // Counter-clockwise
      Counters[EncID]--;
    }
    change=true;
  }
  
  return encA;
}

// read the button on pin N. Change saved in butPress
unsigned char readBut(const int pin, unsigned char prev, int EncID) {

  unsigned char butA = MCP.digitalRead(pin);    // Read encoder pins
  if (butA != prev) {
    if (butA == HIGH) butPress = 101; 
                 else butPress = EncID ;  // button pressed
  }
  return butA;  
}

// setup the PrevPrevPrevEncoders as inputs. 
unsigned char encPinsSetup(const int *pin) {

  MCP.pinMode(pin[0], INPUT);  // Encoder A pin  CLK pin
  MCP.pullUp(pin[0], HIGH);    //
  
  MCP.pinMode(pin[1], INPUT);  // Encoder B pin  DT pin
  MCP.pullUp(pin[1], HIGH); 

 
}

// setup the push buttons
void butPinsSetup(const int pin) {
  MCP.pinMode(pin, INPUT);
  MCP.pullUp(pin, HIGH); 
}

void setup() {  

  MCP.begin(0);    // 0 = i2c address 0x20  

  lcd.begin(); 
  lcd.backlight(); 
  lcd.clear();
  
  // setup the pins using loops, saves coding when you have a lot of PrevPrevPrevEncoders and buttons
  for (int n = 0; n < EncNum; n++) {
    
    encPinsSetup(EncPins[n]);
    PrevEncoders[n] = CW;  // default state 1
    
    butPinsSetup(ButPins[n]);
    PrevButton[n]  = 101;
    
    Counters[n] = 0; // position each encoder
  }

  lcd.setCursor(0,0);lcd.print("Enc0: ");
  lcd.setCursor(0,1);lcd.print("Enc1: ");
  lcd.setCursor(0,2);lcd.print("Enc2: ");
  lcd.setCursor(10,0);lcd.print("Enc3: ");
  lcd.setCursor(10,1);lcd.print("Enc4: ");
  lcd.setCursor(0,3); lcd.print("Button: ");
  
}

void loop() {

    // Read encoders and buttons
    for (int n = 0; n < EncNum; n++) {
      PrevEncoders[n] = readEnc(EncPins[n], PrevEncoders[n],n);
      PrevButton[n]   = readBut(ButPins[n], PrevButton[n],  n);
   }

       // do things when a when a button has been pressed
    if (butPress < 100) {
      lcd.setCursor(8,3);lcd.print("  ");
      lcd.setCursor(8,3);lcd.print(butPress);
      butPress = 101; 
    }

  // when an encoder has (been rotated
    if ((change == true) & (encSelect[EncNo] < 100 )) {

      // Display the direction of rotation
      lcd.setCursor(17,3);lcd.print("   ");
      if (encSelect[Dir] == CW ) { lcd.setCursor(18,3);lcd.print("CW"); }
                            else { lcd.setCursor(17,3);lcd.print("CWW"); }
     
      switch (encSelect[EncNo]) {
      case 0 :  lcd.setCursor(5,0);lcd.print("     "); // just shows the counter number
                lcd.setCursor(5,0);lcd.print(Counters[encSelect[EncNo]]);
                break;

      case 1 :  lcd.setCursor(5,1);lcd.print("     "); // shows the item from the list
                lcd.setCursor(5,1);lcd.print(list[Counters[encSelect[EncNo]] % sizeof(list)]);
                break;

      case 2 :  lcd.setCursor(5,2);lcd.print("             ");   // this is the best way to get the number of element in array
                lcd.setCursor(5,2);lcd.print(menu[Counters[encSelect[EncNo]] % (sizeof(menu)/sizeof(String))]);
                break;

      case 3 :  lcd.setCursor(15,0);lcd.print("    "); // multiples the counter by 10
                lcd.setCursor(15,0);lcd.print(Counters[encSelect[EncNo]] * 10);
                break;

      case 4 :  lcd.setCursor(15,1);lcd.print("     ");  // just show on/off according to remainder
                lcd.setCursor(15,1);lcd.print( (Counters[encSelect[EncNo]]%2) == 1 ? "ON" : "OFF");
                break;
      default : break;
    }

    encSelect[EncNo] = 101;
    // ready for the next change
    change = false; 

  }
}
