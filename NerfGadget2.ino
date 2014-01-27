#include <Wire.h>  // Handles I2C
#include <EEPROM.h>  // Brightness, Baud rate, and I2C address are stored in EEPROM
#include "settings.h"  // Defines command bytes, EEPROM addresses, display data
#include "SevSeg.h" //Library to control generic seven segment displays
#include<stdlib.h>

#define FRAMEPERIOD 200; 

const int MODE_MAGAZINE = 0;
const int MODE_SPEEDOMETER = 1;

const int NUM_DIGITS = 4;
const int THRESHOLD = 800;

const int modeButton = 10;
const int actionButton = 11;
const int resetButton = 12;

boolean modeButtonLastState;
boolean actionButtonLastState;
boolean resetButtonLastState;

int mode = 1;

const int numMagazines = 5;
int magazine = 0;
int magazines[5] = {6,12,18,25,35};


SevSeg myDisplay;

double speed = 0.0;
double nerfDartLength = 0.071;

const int maxDuration = (0ul)-1;


struct display
{
  char digits[4];
  unsigned char decimals;
  unsigned char cursor;
  
  
} 
display;  // displays be displays

unsigned long startTime = 0;
unsigned long endTime = 0;

void setup(){
  initDisplay();

  display.digits[0] = 8;
  display.digits[1] = 8;
  display.digits[2] = 8;
  display.digits[3] = 8;  
  display.decimals = 0x00;  
  
  pinMode(modeButton, INPUT);    
  pinMode(actionButton, INPUT);    
  pinMode(resetButton, INPUT);
  //digitalWrite(modeButton, LOW);
  //digitalWrite(actionButton, LOW);
  //digitalWrite(resetButton, LOW);  
  
}

void loop(){

  while( true ){
    if( analogRead(A6) < THRESHOLD ){
      startTime = micros();  
      while( analogRead(A6) < THRESHOLD ){

      }
      endTime = micros();
      
      switch(mode){
      case MODE_MAGAZINE:
        magazine--;
        MagazineMode();
        break;
      case MODE_SPEEDOMETER:
        SpeedometerMode();
        break;
    }

    }
    

    CheckButtons();
    switch(mode){
      case MODE_MAGAZINE:
        displayInt( magazine );
        break;
      case MODE_SPEEDOMETER:
        displayDouble( speed );
        break;
    }
    /*
    int analogReading = analogRead(A6);
    double voltage = analogReading * 0.0049;
    double duration = 0.001 * ( endTime - startTime ) ;
    //displayDouble( voltage );
    displayInt(analogReading);
    */
  }
    
   
  

}

  inline void CheckButtons(){
  boolean modeButtonState = digitalRead( modeButton ) == HIGH;
  boolean actionButtonState = digitalRead( actionButton ) == HIGH;  
  boolean resetButtonState = digitalRead( resetButton ) == HIGH;
  
  if( modeButtonState != modeButtonLastState && !modeButtonState ){
    mode = ( mode + 1 ) % 2;
    displayInt( 0 );
    Serial.print("mode pressed ");
    Serial.println( mode );
  }

  if( actionButtonState != actionButtonLastState && !actionButtonState ){
    ActionPressed();
    Serial.println("action pressed");
  }

  if( resetButtonState != resetButtonLastState && !resetButtonState ){
    ResetPressed();
    Serial.println("reset pressed");

  }
  
  modeButtonLastState = modeButtonState;
  actionButtonLastState = actionButtonState;
  resetButtonLastState = resetButtonState;
}

inline void ActionPressed(){
  if( mode == MODE_MAGAZINE ){
    magazine--;
    MagazineMode();
  }
  else {
  }
}

inline void ResetPressed(){
  if( mode == MODE_MAGAZINE ){
    int i=0;
    while( magazines[i] <= magazine && i<numMagazines ) i++;
    if( i==numMagazines ) i=0;
    Serial.println( i );
    magazine = magazines[i];
    MagazineMode();
  }
  else {
    
  }
}

inline void MagazineMode(){
  if( magazine < 0 ) magazine = 0;
}


void SpeedometerMode(){
  unsigned long duration = 0;
  if( endTime < startTime ) {
    duration = ( maxDuration - startTime ) + endTime;
  }
  else duration = endTime - startTime;
   
  speed = nerfDartLength * 1000000.0 / duration;
  
  if( Serial ){
    Serial.println( duration );
    Serial.print( "speed: " );
    Serial.println( speed );      
  }
  
  
}

void initDisplay(){
  int digit1 = 16; // DIG1 = A2/16 (PC2)
  int digit2 = 17; // DIG2 = A3/17 (PC3)
  int digit3 = 3;  // DIG3 = D3 (PD3)
  int digit4 = 4;  // DIG4 = D4 (PD4)

  //Declare what pins are connected to the segments
  int segA = 8;  // A = D8 (PB0)
  int segB = 14; // B = A0 (PC0)
  int segC = 6;  // C = D6 (PD6), shares a pin with colon cathode
  int segD = A1; // D = A1 (PC1)
  int segE = 23; // E = PB7 (not a standard Arduino pin: Must add PB7 as digital pin 23 to pins_arduino.h)
  int segF = 7;  // F = D7 (PD6), shares a pin with apostrophe cathode
  int segG = 5;  // G = D5 (PD5)
  int segDP= 22; //DP = PB6 (not a standard Arduino pin: Must add PB6 as digital pin 22 to pins_arduino.h)

  int digitColon = 2; // COL-A = D2 (PD2) (anode of colon)
  int segmentColon = 6; // COL-C = D6 (PD6) (cathode of colon), shares a pin with C
  int digitApostrophe = 9; // APOS-A = D9 (PB1) (anode of apostrophe)
  int segmentApostrophe = 7; // APOS-C = D7 (PD7) (cathode of apostrophe), shares a pin with F

  int numberOfDigits = 4; //Do you have a 2 or 4 digit display?

  int displayType = COMMON_ANODE; //SparkFun 10mm height displays are common anode

  //Initialize the SevSeg library with all the pins needed for this type of display
  myDisplay.Begin(displayType, numberOfDigits, 
  digit1, digit2, digit3, digit4, 
  digitColon, digitApostrophe, 
  segA, segB, segC, segD, segE, segF, segG, 
  segDP,
  segmentColon, segmentApostrophe);
  
  myDisplay.SetBrightness(100);
}

void displayInt(int value){
  byte thousands = ( value / 1000 ) % 10;
  byte hundreds = ( value / 100 ) % 10;
  byte tens = ( value / 10 ) % 10;
  byte ones = value % 10;
  
  display.digits[0] = thousands;
  display.digits[1] = hundreds;
  display.digits[2] = tens;
  display.digits[3] = ones;
  display.decimals = 0x0;
  
  myDisplay.DisplayString(display.digits, display.decimals);

}

void displayDouble(double value){
  char buffer[NUM_DIGITS + 1];
  unsigned char minStringWidthIncDecimalPoint = NUM_DIGITS + 1;
  char numVarsAfterDecimal = NUM_DIGITS + 1;
  dtostrf( value, minStringWidthIncDecimalPoint, numVarsAfterDecimal, buffer );

  int j=0;
  boolean isLeadingZero = true;
  for(int i=0;i<(NUM_DIGITS+1);i++){
    boolean isDotNext = i<NUM_DIGITS && buffer[i+1] == '.';
    
    if( buffer[i] == '.' || ( buffer[i] == '0' && isLeadingZero && !isDotNext ) ) continue;
    isLeadingZero = false;
    if( buffer[i] < 48 ) continue;
    byte number = buffer[i] - 48;
    if( j<NUM_DIGITS ) {
      display.digits[j] = number;
      if( isDotNext ) display.decimals = j+1;
    }
    j++;
  }  
  
  myDisplay.DisplayString(display.digits, display.decimals);

}

