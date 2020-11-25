
#include <Wire.h>
#include "Adafruit_MCP23017.h"  // https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library


// setup the port expander
Adafruit_MCP23017 MCP;

const int CW  = 1;
const int CCW = 2;
const int EncNo = 0;
const int Dir = 1;

boolean change=false;        // goes true when a change in the encoder state is detected
int butPress = 150;          // stores which button has been pressed
int encSelect[2] = {101, 0}; // stores the last encoder used and direction {EncID, 1=CW or 2=CCW}

const int EncNum = 5;  // number of rotary 

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

int Counters[EncNum];  // keep each encoder position

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

  Serial.begin(9600);

  MCP.begin(0x27);    // 0 = i2c address 0x20  

  // setup the pins using loops, saves coding when you have a lot of PrevPrevPrevEncoders and buttons
  for (int n = 0; n < EncNum; n++) {
    
    encPinsSetup(EncPins[n]);
    PrevEncoders[n] = CW;  // default state 1
    
    butPinsSetup(ButPins[n]);
    PrevButton[n]  = 101;
    
    Counters[n] = 0; // position each encoder
  } 

  Serial.println("Rotary Enceoder Development Board Demo....");
  Serial.println("Turn or press one the 5 Rotary Encoder...."); 
}

void loop() {

    // Read encoders and buttons
    for (int n = 0; n < EncNum; n++) {
      PrevEncoders[n] = readEnc(EncPins[n], PrevEncoders[n],n);
      PrevButton[n]   = readBut(ButPins[n], PrevButton[n],  n);
   }

       // do things when a when a button has been pressed
    if (butPress < 100) {
      Serial.print(butPress+1); Serial.println(". Encoder button is pressed...");
      butPress = 101; 
    }

  // when an encoder has (been rotated
    if ((change == true) & (encSelect[EncNo] < 100 )) {

      // Display the direction of rotation
      Serial.print(encSelect[EncNo]+1);
      if (encSelect[Dir] == CW ) { Serial.print(". Encoder is turned CW  --> "); }
                            else { Serial.print(". Encoder is turned CWW --> "); }
     
      switch (encSelect[EncNo]) {
      case 0 :  Serial.println(Counters[encSelect[EncNo]]); // just shows the counter number
                break;

      case 1 :  Serial.println(list[Counters[encSelect[EncNo]] % sizeof(list)]); // shows the item from the list
                break;

      case 2 :  Serial.println(menu[Counters[encSelect[EncNo]] % (sizeof(menu)/sizeof(String))]);   // this is the best way to get the number of element in array
                break;

      case 3 :  Serial.println(Counters[encSelect[EncNo]] * 10); // multiples the counter by 10
                break;

      case 4 :  Serial.println((Counters[encSelect[EncNo]]%2) == 1 ? "ON" : "OFF");  // just show on/off according to remainder
                break;
      default : break;
    }

    encSelect[EncNo] = 101;
    // ready for the next change
    change = false; 

  }
}
