#include "SevSeg.h" //Library to control generic seven segment displays

#define displayTimeout 5

const float nerfBulletLength = 0.07;
const unsigned long maxDuration = (0ul)-1;

/* Sensor variables */

volatile uint8_t readFlag;
volatile int analogVal;
volatile int lastAnalogVal;
volatile unsigned long lastAnalogReadTime;

int lowestRestVal;
int heighestRestVal;
int avgRestVal;
long analogTotal = 0;
long analogReads = 0;

boolean isBulletCrossing;
int threshold = 20;

/* Speed mode variables */

unsigned long startTime;
unsigned long endTime;
float speed ;


/* Interface variables */

const uint8_t MODE_MAGAZINE = 0;
const uint8_t MODE_SPEEDOMETER = 1;

const uint8_t NUM_DIGITS = 4;
const int THRESHOLD = 800;

const uint8_t modeButton = MISO;
const uint8_t actionButton = SCK;
const uint8_t resetButton = MOSI;

boolean modeButtonLastState;
boolean actionButtonLastState;
boolean resetButtonLastState;

int mode = 0;

unsigned long loopCounter = 0;

/* Magazine mode variables */

const int numMagazines = 5;
volatile int magazine = 0;
int magazines[5] = {6,12,18,25,35};


/* Display variables */

SevSeg myDisplay; //Create an instance of the object

struct display
{
  char digits[4];
  unsigned char decimals;
  unsigned char cursor;
} 
speedDisplay, magazineDisplay;  
boolean displayOn = true;

void setup(){
  //Serial.begin(9600);
  setupButtons();
  setupFreeRunningAnalog();
  setupDisplay();
  myDisplay.SetBrightness(100);
  displayFloat( speed );
  displayInt( magazine );

}


void loop(){
  loopCounter++;
  if(readFlag==1) checkAnalog();
  if( isBulletCrossing ) return;
  checkButtons();
  switch(mode){
    case MODE_MAGAZINE:
      displayMagazine();
      break;
    case MODE_SPEEDOMETER:
      displaySpeed();
      break;
  } 
}

inline void displayMagazine(){
  myDisplay.DisplayString(magazineDisplay.digits, magazineDisplay.decimals);
}

inline void displaySpeed(){    
    
    if( displayOn ){
      myDisplay.DisplayString(speedDisplay.digits, speedDisplay.decimals);
      unsigned long currentTime = micros();
      if( ( currentTime - endTime ) > 1000000 * displayTimeout ) displayOn = false;
    }
}

inline void checkButtons(){
  boolean modeButtonState = digitalRead( modeButton ) == HIGH;
  boolean actionButtonState = digitalRead( actionButton ) == HIGH;  
  boolean resetButtonState = digitalRead( resetButton ) == HIGH;
  
  if( modeButtonState != modeButtonLastState && !modeButtonState ){
    ModePressed();
  }

  if( actionButtonState != actionButtonLastState && !actionButtonState ){
    ActionPressed();
  }

  if( resetButtonState != resetButtonLastState && !resetButtonState ){
    ResetPressed();
    
  }
  
  modeButtonLastState = modeButtonState;
  actionButtonLastState = actionButtonState;
  resetButtonLastState = resetButtonState;
}


void onBulletEnter(){
  startTime = lastAnalogReadTime;
}

void onBulletExit(){
  
  switch(mode){
      case MODE_SPEEDOMETER:
        SpeedometerMode();
        
      case MODE_MAGAZINE:
          MagazineMode();
      break;
  }
  
}

inline void ModePressed(){
    mode = ( mode + 1 ) % 2;
    endTime=micros();
    displayOn = true;
    resetCalibration();
}

inline void ActionPressed(){
  if( mode == MODE_MAGAZINE ){
    MagazineMode();
  }
  else {
    endTime = micros();
    displayOn = true;
  }
}

inline void ResetPressed(){
  if( mode == MODE_MAGAZINE ){
    int i=0;
    while( magazines[i] <= magazine && i<numMagazines ) i++;
    if( i==numMagazines ) i=0;
    magazine = magazines[i];
    displayInt( magazine );
  }
  else {
    
  }
}

inline void MagazineMode(){
  magazine--;
  if( magazine < 0 ) magazine = 0;
  displayInt( magazine );
}

inline void SpeedometerMode(){
  endTime = lastAnalogReadTime;
  displayOn = true;
  unsigned long duration = ( endTime > startTime ) ? endTime - startTime : ( maxDuration - startTime ) + endTime ;
  speed = nerfBulletLength * 1000000 / duration;
  displayFloat( speed );

}

inline void resetCalibration(){
  analogReads = 0;
}

/*
void inline checkAnalog(){
    if( analogReadCounter == 1 ) return;
    if( !isBulletCrossing && ( ( analogVal - lastAnalogVal ) > 10 ) ){
      isBulletCrossing = true;
      onBulletEnter();
    }
    else if( isBulletCrossing && ( ( analogVal - lastAnalogVal ) < 10 ) ){
      isBulletCrossing = false;
      onBulletExit();
    }
}*/


void checkAnalog(){ 
  if( analogReads <= 100 ){
      if( analogReads == 100 ){
        avgRestVal = analogTotal / analogReads;
        isBulletCrossing = false;
        analogReads++;          
        return;
      }
      analogTotal += analogVal;
      if( lowestRestVal > analogVal || analogReads == 0 ) lowestRestVal = analogVal;
      if( heighestRestVal < analogVal || analogReads == 0 ) heighestRestVal = analogVal;
      analogReads++;
  }else if( !isBulletCrossing && analogVal > avgRestVal + threshold ){
    isBulletCrossing = true;
    onBulletEnter();
    
  }else if( isBulletCrossing && analogVal <= heighestRestVal ){
      isBulletCrossing = false;
      onBulletExit();
  }else if( !isBulletCrossing ){
    // If the bullet hasn't been detected, adjust avgerage, heighest and lowest analog values 
    if( analogVal > avgRestVal ) avgRestVal++;
    if( analogVal < avgRestVal ) avgRestVal--;
    
    if( analogVal > heighestRestVal ) heighestRestVal = analogVal;
    else if( loopCounter % 10 == 0 ) heighestRestVal--;
    if( analogVal < lowestRestVal ) lowestRestVal = analogVal;
    else if( loopCounter % 10 == 0 ) lowestRestVal++;

  }
}

void displayInt(int value){
  
  sprintf( magazineDisplay.digits, "% 4d", value );
  magazineDisplay.decimals = 0;
  if( magazineDisplay.digits[2] == ' ' ) magazineDisplay.digits[2] = '0';
  

}

void displayFloat(float value){
  char analogChars[NUM_DIGITS+1];
  //char numChars[NUM_DIGITS];
  dtostrf( speed, 5, 1, analogChars );
  int j=0;
  for(int i=0;i<5;i++){
    if( analogChars[i] != '.' ) {
      speedDisplay.digits[j] = analogChars[i];
      j++;
    }
    else{
      speedDisplay.decimals = 1 << (i-1);
    }
  }
  
    
}

ISR(ADC_vect){
  readFlag = 1;
  // Must read low first
  analogVal = ADCL | (ADCH << 8);
  lastAnalogReadTime = micros();
  
}

void setupButtons(){
  pinMode(modeButton, INPUT);
  pinMode(actionButton, INPUT);
  pinMode(resetButton, INPUT);
}

void setupDisplay(){
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
}

void setupFreeRunningAnalog(){
  
  // clear ADLAR in ADMUX (0x7C) to right-adjust the result
  // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  ADMUX &= B11011111;
  
  // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
  // proper source (01)
  ADMUX &= B00111111;
  ADMUX |= B01000000;
  
  // Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog
  // input
  ADMUX &= B11110000;
  
  // Set MUX3..0 in ADMUX (0x7C) to read from AD8 (Internal temp)
  // Do not set above 15! You will overrun other parts of ADMUX. A full
  // list of possible inputs is available in Table 24-4 of the ATMega328
  // datasheet
    
  ADMUX |= 6;
  // ADMUX |= B00001000; // Binary equivalent
  
  // Set ADEN in ADCSRA (0x7A) to enable the ADC.
  // Note, this instruction takes 12 ADC clocks to execute
  ADCSRA |= B10000000;
  
  // Set ADATE in ADCSRA (0x7A) to enable auto-triggering.
  ADCSRA |= B00100000;
  
  // Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running.
  // This means that as soon as an ADC has finished, the next will be
  // immediately started.
  ADCSRB &= B11111000;
  
  // Set the Prescaler to 4 (16000KHz/64 = 250KHz)
  // Above 200KHz 10-bit results are not reliable.
  ADCSRA |= B00000110 ;
  // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
  // Without this, the internal interrupt will not trigger.
  ADCSRA |= B00001000;
  
  // Enable global interrupts
  // AVR macro included in <avr/interrupts.h>, which the Arduino IDE
  // supplies by default.
  sei();
  
  // Kick off the first ADC
  readFlag = 0;
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  ADCSRA |=B01000000;
}
