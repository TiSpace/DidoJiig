/* ************************************************************************
    Schrittmotorsteuerung für Zinkenfräsvorrichtung

    History:
    V02:    fully working, old MC3479 driver
    V03: revised hardware and TB6600

* ************************************************************************
*/

/*
   Vorschlag für LCD Menü  https://www.youtube.com/watch?v=DuAG98P9Seo
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://create.arduino.cc/projecthub/Oniichan_is_ded/lcd-i2c-tutorial-664e5a
#include <OneButton.h> //https://github.com/mathertel/OneButton
#include <Encoder.h>
#include "setting.h"
#include <EEPROM.h>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__) //to get Arduino Filename extracted from __FILE__



// variables
char inString[INLENGTH + 2];
int inCount;
unsigned int Frequenz = 2; //Hz (Maximum: 65535 Hz)
byte motorPosition = 0; // store Motor infonrmation
unsigned long lastPuls;
unsigned long numberTurns;
int goTurns; // number of turns shall be taken
int goTurnsCount;

byte statusMenue = 0;   // store Menue setting
int menueLevel = 1;

int bladeThickness = EEPROM.read(ADDR_blade);    //blade thickness
//int bladeThickness = 30;    //blade thickness
int dovetailWidth =  EEPROM.read(ADDR_dovetail);   // Zinnkenbreiet
//int dovetailWidth =  100;   // Zinnkenbreiet
int overlapWidth = 1;       // overlap for cut

int cutPattern = 0;         // pattern sequenz
int cutPatternNumbers = 0;  // how many cutss to be done
int cutPatternPosition[30]; // calcualated cut positions
int cutPatternPositionAbs;  //abssolute position
int executeCutPattern;    // running in the loop
byte cuttingInProgress;

enum motorStatus {startMotor, stopMotor, rightTurn, leftTurn, fullStep, halfStep};
//enum buttonAction {right, left};
enum LED_colour {red, green, blue, white, black};

//LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

// define buttons
OneButton btnRight = OneButton(Pin_goRight, true, true);
OneButton btnLeft = OneButton(Pin_goLeft, true, true);
OneButton btnEncoder = OneButton(Pin_EncoderSwitch, true, true);

Encoder myEnc(Pin_EncoderB, Pin_EncoderA);
long oldPosition  = 0;


/* ****************************************************
       SETUP
 *   ****************************************************
*/
void setup() {

  //definiere Pins
  pinMode(Pin_Drehrichtung, OUTPUT);
  pinMode( Pin_Enable, OUTPUT);
  //pinMode( Pin_HalfFull, OUTPUT);

  pinMode(Pin_Endschalter1, INPUT_PULLUP);
  pinMode(Pin_Endschalter2, INPUT_PULLUP);

  pinMode(LED_blue, OUTPUT);
  pinMode(LED_red, OUTPUT);
  pinMode(LED_green, OUTPUT);
  controlLED(black);

  btnRight.attachClick(handleButtonRight);
  btnRight.attachLongPressStart(handleButtonRightLongStart); //move als long as button is pressed
  btnRight.attachLongPressStop(handleButtonRightLongStop); //move als long as button is pressed

  btnLeft.attachClick(handleButtonLeft);

  btnLeft.attachLongPressStart(handleButtonLeftLongStart); //move als long as button is pressed
  btnLeft.attachLongPressStop(handleButtonLeftLongStop); //move als long as button is pressed


  btnEncoder.attachClick(handleButtonSwitch);
  btnEncoder.attachLongPressStart(handleButtonEncoderLongStart);
  Serial.begin(myBaudRate);
  versionsInfo();   //print File information

  menuCom(); // Display on SerialPort the communication menue

  Serial.flush(); //clear Serial buffer

  // defaults for motor
  digitalWrite(Pin_Enable, HIGH); //turn off motor
  actionMotor(fullStep);
  actionMotor(rightTurn);

  // initialize the lcd
  lcd.init();
  lcd.init();
  lcd.createChar(0, Char0);
  lcd.createChar(1, Char1);
  lcd.createChar(2, Charup);
  lcd.createChar(3, Chardown);

  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Zinkenfraese");
  lcd.setCursor(1, 1);
  lcd.print(__FILENAME__);

  //  lcd.print(__DATE__);
  //  lcd.print(" at ");
  //  lcd.print(__TIME__);
  delay(1000);

  updateMenu();   //display LCD menue

  //checkI2C();
}
/* ****************************************************
       LOOP
 *   ****************************************************
*/
void loop() {

  // keep watching the push button:
  btnRight.tick();
  btnLeft.tick();
  btnEncoder.tick();

  Eingabe(); // read ComPort for any input
  userMenue();  //read Encoder input

  // run triggered by Button the cutting pattern
  if (bitRead(motorPosition, Bit_ButtonPressed) == 1 && cuttingInProgress > 0 && bitRead( motorPosition, Bit_MotorRunning) == 0) {

    if (bitRead(cuttingInProgress, 1) == 0) {
      goTurnsCount = turnsFromDistance(cutPatternPosition[executeCutPattern]);
    }
    else {
      goTurnsCount = -turnsFromDistance(cutPatternPosition[executeCutPattern]);
    }
    Serial.print(executeCutPattern);
    Serial.print("/");
    Serial.print(cutPatternNumbers);
    Serial.print("  ");
    Serial.print(cutPatternPosition[executeCutPattern]);
    Serial.print("  ");
    Serial.println(goTurnsCount);

    if (goTurnsCount < 0) {  //neg numbers indicate turn to right
      actionMotor(rightTurn);
      //goTurnsCount = -turnsFromDistance(cutPatternPosition[executeCutPattern]);
      goTurnsCount = abs(goTurnsCount);
      Serial.print("##");
      Serial.println(goTurnsCount);
    }
    else {
      actionMotor(leftTurn);
    }
    if (bitRead(cuttingInProgress, 1) == 0) { // if not in modus to repeat last step
      executeCutPattern++;
    }
    lcd.setCursor(0, 0);
    lcd.print(F("Position "));
    lcd.print(executeCutPattern);
    lcd.print("/");
    lcd.print(cutPatternNumbers + 1);


    bitClear(motorPosition, Bit_ButtonPressed); //resset Button Event
  }


  //// führt Bladebreite Schritt aus
  //  if (bitRead(motorPosition, Bit_ButtonPressed) == 1) {
  //    goTurnsCount = turnsFromDistance(bladeThickness);
  //    Serial.print("Left/Right ");
  //    Serial.println(goTurnsCount);
  //    actionMotor(startMotor);
  //    bitClear(motorPosition, Bit_ButtonPressed);
  //  }

  checkEndposition(); // check if end positions are reached and stop motor


  /*
   * ***************************************************
        this actually moves the stepper motor
   * ***************************************************
  */
  // special movements -> user defined right/left

  if ((bitRead(motorPosition, Bit_LongPressRight) == 1) ||  //user defined right/left
      (bitRead(motorPosition, Bit_LongPressLeft) == 1 ) ||  //user defined right/left
      (bitRead(motorPosition, Bit_MoveToRightStart) == 1)  //running home until end position
     ) {
    goTurnsCount++;
  }

  // move before cutting
  if (bitRead(motorPosition, Bit_MoveUserStart) == 1) {
    if (goTurnsCount == 0) {
      cuttingInProgress = 1; //set flag for enabling cut movements
      bitClear(motorPosition, Bit_MoveUserStart);
    }
  }

  //Puls
  if (goTurnsCount > 0) {

    actionMotor(startMotor);
    goTurnsCount--;
    digitalWrite(Pin_Frequenz, HIGH);
    delayMicroseconds(350);
    //delay(Frequenz / 2);
    digitalWrite(Pin_Frequenz, LOW);
    //delay(Frequenz / 2);
    delayMicroseconds(350);
    //numberTurns++;
    //if (goTurnsCount % 250 == 0 & executeCutPattern) {
    if (goTurnsCount % 250 == 0 & executeCutPattern) {
      lcd.setCursor(0, 1);
      lcd.print("move ");
      lcd.print((float)distanceFromTurns(goTurnsCount) / 10);
      lcd.print("mm ");
    }
    if (goTurnsCount == 0 && executeCutPattern > cutPatternNumbers) {
      cuttingInProgress = 0;
      executeCutPattern = 0;
      lcd.clear();
      lcd.print(F("finish"));
      cuttingInProgress = 1;
      executeCutPattern = 0;
      menueLevel = 1;
      updateMenu();

    }
    //

  }
  else {
    actionMotor(stopMotor); //turn of energizing coils!

  }
}

//=======================================================================
//                    Soft-Reset
//=======================================================================
void(* resetFunc) (void) = 0;//declare reset function at address 0




void handleButtonRight() {
  lcd.clear();
  Serial.println(F("Go right"));
  actionMotor(rightTurn);
  bitSet(motorPosition, Bit_ButtonPressed);
  bitClear(cuttingInProgress, 1);
}

void handleButtonRightLongStart() {
  Serial.println(F("Long Start"));
  actionMotor(leftTurn);
  // ToDo: check for endposition switch
  if (!digitalRead(Pin_Endschalter1) == userSwitchPressed ) {
    lcd.clear();
    lcd.print(F("move right"));
    goTurnsCount++;
  }
  bitSet(motorPosition, Bit_LongPressRight);
}
void handleButtonRightLongStop() {
  Serial.println(F("Long Stop"));
  bitClear(motorPosition, Bit_LongPressRight);
}

void handleButtonLeftLongStart() {
  Serial.println(F("Long Start"));
  actionMotor(rightTurn);
  if (!digitalRead(Pin_Endschalter2) == userSwitchPressed ) {
    lcd.clear();
    lcd.print(F("move left"));
    goTurnsCount++;
  }
  bitSet(motorPosition, Bit_LongPressLeft);
}
void handleButtonLeftLongStop() {
  Serial.println(F("Long Stop"));
  bitClear(motorPosition, Bit_LongPressLeft);
}

void handleButtonEncoderLongStart() {
  lcd.clear();
  lcd.print(F("- R E S E T -"));
  lcd.setCursor(0, 1);
  delay(500);
  resetFunc(); //call reset

}
void handleButtonLeft() {
  lcd.clear();
  Serial.println (F("Go left"));
  actionMotor(leftTurn);
  bitSet(motorPosition, Bit_ButtonPressed);
  if ( executeCutPattern > 0) {
    executeCutPattern--; // force to go to last step
    bitSet(cuttingInProgress, 1);
  }
}

// **********************************************************
//
//    activates and initialize the stepper motor
//
// **********************************************************
void actionMotor(enum motorStatus myMotor) {
  //startMotor, stopMotor,rightTurn,leftTurn,fullStep,halfStep
  switch (myMotor) {
    case startMotor:
      digitalWrite(Pin_Enable, LOW);
      bitSet( motorPosition, Bit_MotorRunning);
      numberTurns = 0; //reset turn counter
      controlLED(red);
      break;
    case stopMotor:
      digitalWrite(Pin_Enable, HIGH);
      bitClear( motorPosition, Bit_MotorRunning);
      goTurnsCount = 0;
      controlLED(green);
      break;
    case rightTurn:
      digitalWrite(Pin_Drehrichtung, LOW);
      bitSet( motorPosition, Bit_MotorDirection);
      break;
    case leftTurn:
      digitalWrite(Pin_Drehrichtung, HIGH);
      bitClear( motorPosition, Bit_MotorDirection);
      break;


  }

}

// **********************************************************
//    calculate distance (in 0.1mm) on given turns
// **********************************************************
int distanceFromTurns(int myTurns) { //distance in 0.1mm
  // calculate turn by given distance and considering half/full step
  // full step: 0.002mm/turn
  // half step:  0.001mm/turn
  // return myTurns / (bitRead(motorPosition, Bit_MotorStep) == 0 ? 50 : 100);

  //new hardware
  // full step: 0.010mm/turn
  // half step:  0.005mm/turn
  return myTurns / (bitRead(motorPosition, Bit_MotorStep) == 0 ? 10 : 20);
}

// **********************************************************
//    calculate turns on given distance (in 0.1mm)
// **********************************************************
int turnsFromDistance(int myDistance) { //distance in 0.1mm
  // calculate turn by given distance and considering half/full step
  // full step: 0.002mm/turn
  // half step:  0.001mm/turn
  //return myDistance * (bitRead(motorPosition, Bit_MotorStep) == 0 ? 50 : 100);

  //new hardware
  // full step: 0.010mm/turn
  // half step:  0.005mm/turn
  return myDistance * (bitRead(motorPosition, Bit_MotorStep) == 0 ? 10 : 20);

}

// **********************************************************
//    handler for encoder switch
// **********************************************************
void handleButtonSwitch() {
  Serial.println("Encoder Switch");
  executeAction();
}
void userMenue() {
  //react on redary Encodeer movements
  long newPosition = myEnc.read() / 4; //read  Encoder
  if (newPosition < oldPosition) {
    oldPosition = newPosition;
    menueLevel--;
    bitSet(statusMenue, Bit_Encoderredate); // Encder has been redated
    Serial.println("turn left");
    updateMenu();
  }
  if (newPosition > oldPosition) {
    oldPosition = newPosition;
    menueLevel++;
    bitSet(statusMenue, Bit_Encoderredate); // Encder has been redated
    Serial.print(newPosition);
    Serial.println(" turn right");
    updateMenu();
  }
  //menueLevel = constrain(menueLevel, 0, 255);


}



// **********************************************************
//            Check Endposition
// **********************************************************
void checkEndposition() {
  // final Position left
  if (digitalRead(Pin_Endschalter1) == userSwitchPressed &&
      bitRead(motorPosition, Bit_MotorRunning) == userMotorRun &&
      bitRead(motorPosition, Bit_MotorDirection) == userMotorTurnLeft) {
    Serial.println("linke Endanschlag");
    actionMotor(stopMotor);
    digitalWrite(Pin_Drehrichtung, LOW);//change direction
    bitSet( motorPosition, Bit_MotorDirection);
    goTurnsCount = 0; //Motor stop
    lcd.clear();
    lcd.print("Final Position");
    lcd.setCursor(0, 1);
    lcd.print("right");
    controlLED(blue);
    delay(2000);

  }

  // final Postion right
  if (digitalRead(Pin_Endschalter2) == userSwitchPressed &&
      bitRead(motorPosition, Bit_MotorRunning) == userMotorRun &&
      bitRead(motorPosition, Bit_MotorDirection) == userMotorTurnRight) {
    Serial.println("rechter Endanschlag");
    actionMotor(stopMotor);
    digitalWrite(Pin_Drehrichtung, HIGH);   //change direction
    bitClear( motorPosition, Bit_MotorDirection);
    goTurnsCount = 0; //Motor stop
    bitClear(motorPosition, Bit_MoveToRightStart); //stop running home

    lcd.clear();
    lcd.print("Final Position");
    lcd.setCursor(0, 1);
    lcd.print("left");
    controlLED(blue);
    delay(2000);
  }
}


// **********************************************************
//            Calculate Cut width
// **********************************************************
void calculateCut() {
  /*
     int bladeThickness = 30;    //blade thickness
    int dovetailWidth =  100;   // Zinnkenbreiet
    int overlapWidth = 1;       // overlap for cut

    int cutPattern = 0;         // pattern sequenz
    int cutPatternNumbers=0;    // how many cutss to be done
    int cutPatternPosition[30]; // calcualated cut positions
    cutPatternPositionAbs
  */
  Serial.println("BP0");

  cutPatternPosition[0] = 0;  //initial cut
  cutPatternPosition[1] = dovetailWidth - bladeThickness; //end of dovetail
  cutPatternPositionAbs = cutPatternPosition[1];

  cutPatternNumbers = 1;

  if ((cutPatternPosition[0] + bladeThickness) < cutPatternPosition[1]) { //Reststeg bleibt
    cutPatternNumbers = 2;
    cutPatternPosition[2] = -cutPatternPosition[1] + bladeThickness - overlapWidth;
    cutPatternPositionAbs += -cutPatternPosition[1] + bladeThickness - overlapWidth;
  }
  //  Serial.println("BP1");
  //
  //  for (int i = 0; i < 3; i++) {
  //
  //
  //    Serial.print(i);
  //    Serial.print(" ");
  //    Serial.print(    cutPatternPosition[i]);
  //    Serial.print(" ");
  //    Serial.println(  cutPatternPositionAbs);
  //  }

  //while ((cutPatternPosition[cutPatternNumbers] + bladeThickness) < dovetailWidth) {
  while ((cutPatternPositionAbs + 2 * bladeThickness ) < dovetailWidth) { //zwei mal da allererster Schnitt bereit erfolgte


    cutPatternNumbers++;
    cutPatternPosition[cutPatternNumbers] = bladeThickness - overlapWidth;
    cutPatternPositionAbs += cutPatternPosition[cutPatternNumbers];
    if (cutPatternPositionAbs > (dovetailWidth - bladeThickness)) { //Schnitt ungültig -> löschen
      //cutPatternNumbers--;
      //cutPatternPositionAbs -= bladeThickness - overlapWidth;;
    }
    //    Serial.print(cutPatternNumbers);
    //    Serial.print(" ");
    //    Serial.print(    cutPatternPosition[cutPatternNumbers]);
    //    Serial.print(" ");
    //    Serial.println(  cutPatternPositionAbs);
  }


  cutPatternNumbers++;
  cutPatternPosition[cutPatternNumbers] = 2 * dovetailWidth - cutPatternPositionAbs;

  Serial.println("Schnitte");
  // nur debug
  cutPatternPositionAbs = 0;
  for (int i = 0; i < cutPatternNumbers + 1; i++) {
    cutPatternPositionAbs += cutPatternPosition[i];
    Serial.print(i);
    Serial.print("  ");
    Serial.print(cutPatternPosition[i]);
    Serial.print("  ");
    Serial.println(cutPatternPositionAbs);
  }
  Serial.print("Absolut: ");
  Serial.println(cutPatternPositionAbs);
  // bei neg r/l berücksichtigen!!
}


// **********************************************************
//            Control LED
// **********************************************************
void controlLED(enum LED_colour myLED) {
  // LOW switches LED on
  digitalWrite(LED_red, HIGH);
  digitalWrite(LED_green, HIGH);
  digitalWrite(LED_blue, HIGH);
  switch (myLED) {
    case red:
      digitalWrite(LED_red, LOW);
      break;
    case green:
      digitalWrite(LED_green, LOW);
      break;
    case blue:
      digitalWrite(LED_blue, LOW);
      break;
    case white:
      digitalWrite(LED_red, LOW);
      digitalWrite(LED_green, LOW);
      digitalWrite(LED_blue, LOW);
      break;
    case black:
      //simply keep all LED off
      break;
  }
}
