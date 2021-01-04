//=======================================================================
//                    Kommunikation Menue
//=======================================================================

void menuCom(){
    Serial.println("Schrittmotoransteuerung");
  Serial.println("Fuer Eingabe mit CR abschliessen");
  Serial.println("Befehle: ");
  Serial.println("<Zahl>  -   Vorschub in 0.1mm");
  Serial.println("g   - Go, Motor starten");
  Serial.println("s   - Stop, Motor stoppen");
  Serial.println("r   - rechtslauf");
  Serial.println("l  -  linkslauf");
  //Serial.println("h  -  HalfStep");
 // Serial.println("f  -  FullStep");

}
//=======================================================================
//                    Kommunikation
//=======================================================================
void Eingabe()
{
  // werte Eingabe aus
  while (Serial.available() != 0) { // prüfe ob Daten anstehen
    //Serial.println(inCount);
    inString[inCount] = Serial.read();  //einlessen
    if (inString[inCount] == INTERMINATOR) { // Ende der NAchricht erkannt
      switch (inString[inCount - 1]) {
        case 'g':
          Serial.println("Starte Motor");
          actionMotor(startMotor);
          break;
        case 's':
          Serial.println("Stoppe Motor");
          actionMotor(stopMotor);

          break;
        case 'r':
          Serial.println("rechtslauf");
          actionMotor(rightTurn);

          break;
        case 'l':
          Serial.println("linkslauf");
          actionMotor(leftTurn);
          break;
//        case 'h':
//          Serial.println("HalfStep");
//          actionMotor(halfStep);
//          break;
//        case 'f':
//          Serial.println("Fullstep");
//          actionMotor(fullStep);
//          break;
        case 'w':
          Serial.print("repeate distance ");
          //Serial.println(goTurns / 50);
          Serial.println(distanceFromTurns(goTurns));
          goTurnsCount = goTurns;
          break;

        default:  //ess musss die Distanz sein
          Serial.println("new distance");

          //goTurns = atol(inString) * 50;
          goTurns = turnsFromDistance(atol(inString));
          goTurnsCount = goTurns;
          lcd.clear();

          lcd.print("Distance ");
          lcd.print((float)goTurns / 500);
          lcd.print("mm ");
          // lcd.setCursor(0,1);
          //lcd.print(goTurns);
      }
      inCount = 0;
      break;
    }
    inCount++;
  }
}
