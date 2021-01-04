/* *********************************************************
      Display  LCD mneue
* *********************************************************
*/
void updateMenu() {
  switch (menueLevel) {
    case 0:
      menueLevel = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print(F("Start "));

      lcd.write((uint8_t)0);
      lcd.write((uint8_t)1);
      lcd.write((uint8_t)0);
      lcd.write((uint8_t)1);
      lcd.setCursor(0, 1);
      lcd.print(F("D:"));
      lcd.print((float)dovetailWidth / 10);
      lcd.print(F(" B:"));
      lcd.print((float)bladeThickness / 10);

      break;
    case 2:
      lcd.clear();
      lcd.print(F("Start "));
      lcd.write((uint8_t)1);

      lcd.write((uint8_t)0);
      lcd.write((uint8_t)1);
      lcd.write((uint8_t)0);

      lcd.setCursor(0, 1);
      lcd.print(F("D:"));
      lcd.print((float)dovetailWidth / 10);
      lcd.print(F(" B:"));
      lcd.print((float)bladeThickness / 10);

      break;

    case 3:
      lcd.clear();
      lcd.print(F("Move to Start"));
      lcd.setCursor(0, 1);
      //lcd.print(" MenuItem2");
      break;
    case 4:
      lcd.clear();
      lcd.print(F("skip dido"));
      break;
    case 5:
      lcd.clear();
      lcd.print(F("dovetail width"));
      lcd.setCursor(0, 1);
      lcd.print((float)dovetailWidth / 10);
      lcd.print("mm");
      break;
    case 6:
      lcd.clear();
      lcd.print(F("Blade Width"));
      lcd.setCursor(0, 1);
      lcd.print((float)bladeThickness / 10);
      lcd.print("mm");
      break;
    case 7:
      lcd.clear();
      lcd.print(F("- R E S E T -"));
      lcd.setCursor(0, 1);

      break;
    case 8:
      menueLevel = 7; //limit Mainmenue
      break;

    case 9:
      break;
    case 10:
      menueLevel = 9;
      break;
    case 11:
      lcd.clear();
      lcd.print(F("running <- ->"));
      lcd.setCursor(0, 1);
      lcd.print("cut ");

      lcd.print(executeCutPattern);
      lcd.print("/");
      lcd.print(cutPatternNumbers + 1);
      lcd.print(" ");
      break;
    case 30:
      menueLevel = 31;
      break;
    case 31:
      lcd.clear();
      lcd.print(F("dovetail "));
      lcd.print((float)dovetailWidth / 10);
      // lcd.setCursor(0, 1);
      // lcd.print(F("new: 5mm"));
      displayNewValue(5);
      break;
    case 32:
      displayNewValue(10);
      //lcd.setCursor(0, 1);
      // lcd.print(F("new: 10mm"));
      break;
    case 33:
      displayNewValue(15);
      //lcd.setCursor(0, 1);
      //lcd.print(F("new: 15mm"));
      break;
    case 34:
      displayNewValue(20);
      //lcd.setCursor(0, 1);
      //      lcd.print(F("new: 20mm"));
      break;
    case 35:
      displayNewValue(25);
      //lcd.setCursor(0, 1);
      //lcd.print(F("new: 25mm"));
      break;
    case 36:
      displayNewValue(30);
      // lcd.setCursor(0, 1);
      //lcd.print(F("new: 30mm"));
      break;
    case 37:
      displayNewValue(40);
      //lcd.setCursor(0, 1);
      //lcd.print("new: 40mm");
      break;
    case 38:
      displayNewValue(50);
      // lcd.setCursor(0, 1);
      //lcd.print(F("new: 50mm"));
      break;
    case 39:
      menueLevel = 38;
      break;
    case 40:
      menueLevel = 41;
      break;
    case 41: //40-49 reserved for ssetup blade
      lcd.clear();
      lcd.print(F("Blade "));
      lcd.print((float)bladeThickness / 10);
      displayNewValue(3);


      break;
    case 42:
      displayNewValue(4);

      break;
    case 43:
      displayNewValue(5);

      break;
    case 44:
      menueLevel = 43;
      break;
  }
}

void displayNewValue(int myNewValue) {
// simplify routine: shows value for new setup in LCD
  lcd.setCursor(0, 1);
  lcd.print(F("new: "));
  lcd.print(myNewValue);
  lcd.print(F("mm"));
}
/* *********************************************************
      Actions resulted out of LCD mneue
 * *********************************************************
*/
void executeAction() {
  Serial.print("Action: ");
  Serial.println(menueLevel);
  switch (menueLevel) {
    case 1:
      calculateCut();
      cuttingInProgress = 1; //set flag for enabling cut movements
      menueLevel = 11;
      lcd.clear();
      lcd.print(F("run Pattern"));
      break;
    case 2:
      goTurnsCount = turnsFromDistance(dovetailWidth);
      bitSet(motorPosition, Bit_MoveUserStart);   //first move a width of the dido
      actionMotor(leftTurn);
      calculateCut();
      //cuttingInProgress = 1; //set flag for enabling cut movements
      menueLevel = 11;
      lcd.clear();
      lcd.print(F("run Pattern"));
      break;
      break;
    case 3://move to start
      actionMotor(rightTurn);
      actionMotor(startMotor);
      menueLevel = 0;
      Serial.print("move to start ");
      Serial.println(goTurnsCount);
      bitSet(motorPosition, Bit_MoveToRightStart); // set flag to allow run
      goTurnsCount++;



      break;
    case 4:
      //move simply the width of 2 didos
      goTurnsCount = turnsFromDistance(dovetailWidth) * 2;
      actionMotor(leftTurn);
      actionMotor(startMotor);
      break;
    case 5:
      menueLevel = 31;
      updateMenu();
      break;
    case 6: // ssetup blade width
      menueLevel = 41;
      updateMenu();
      break;
    case 7:
      resetFunc(); //call reset
      //menueLevel = 51;
      //updateMenu();
      break;
    case 31:
      dovetailWidth = 50;
      menueLevel = 3;
      EEPROM.write(ADDR_dovetail, dovetailWidth);
      updateMenu();
      break;
    case 32:
      dovetailWidth = 100;
      menueLevel = 3;
      EEPROM.write(ADDR_dovetail, dovetailWidth);
      updateMenu();
      break;
    case 33:
      dovetailWidth = 150;
      menueLevel = 3;
      EEPROM.write(ADDR_dovetail, dovetailWidth);
      updateMenu();
      break;
    case 34:
      dovetailWidth = 200;
      menueLevel = 3;
      EEPROM.write(ADDR_dovetail, dovetailWidth);
      updateMenu();
      break;
    case 35:
      dovetailWidth = 250;
      menueLevel = 3;
      EEPROM.write(ADDR_dovetail, dovetailWidth);
      updateMenu();
      break;
    case 36:
      dovetailWidth = 3000;
      menueLevel = 3;
      EEPROM.write(ADDR_dovetail, dovetailWidth);
      updateMenu();
      break;
    case 37:
      dovetailWidth = 400;
      menueLevel = 3;
      EEPROM.write(ADDR_dovetail, dovetailWidth);
      updateMenu();
      break;
    case 38:
      dovetailWidth = 500;
      menueLevel = 3;
      EEPROM.write(ADDR_dovetail, dovetailWidth);
      updateMenu();
      break;
    case 41:
      bladeThickness = 30;
      menueLevel = 4;
      EEPROM.write(ADDR_blade, bladeThickness);
      updateMenu();
      break;
    case 42:
      bladeThickness = 40;
      menueLevel = 4;
      EEPROM.write(ADDR_blade, bladeThickness);
      updateMenu();
      break;
    case 43:
      bladeThickness = 50;
      menueLevel = 4;
      EEPROM.write(ADDR_blade, bladeThickness);
      updateMenu();
      break;
    case 51:
      break;
  }
}
