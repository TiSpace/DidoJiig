

#define myBaudRate 38400
// Pin defintions
#define Pin_Frequenz      13
#define Pin_Endschalter1  11
#define Pin_Endschalter2  12
#define Pin_Drehrichtung  10
#define Pin_Enable        9
//#define Pin_HalfFull      8
#define Pin_goRight       3
#define Pin_goLeft        2
#define Pin_EncoderA      5
#define Pin_EncoderB      6
#define Pin_EncoderSwitch 4

#define INLENGTH 5       //maximale Größe der Zahl
#define INTERMINATOR '\r' //CR

//#define MinFreq 33
//#define MaxFreq 65535

#define Bit_MotorRunning 0      //  1 = Motor running     0 = Motor stopped
#define Bit_MotorDirection 1    //  1 = right turn        0 = left turn
#define Bit_MotorStep 2         //  1 = half step         0 =full step
#define Bit_ButtonPressed 7     //  1  =Button pressed
#define Bit_LongPressRight 6
#define Bit_LongPressLeft 5
#define Bit_MoveToRightStart 4    // running right until end position is reached
#define Bit_MoveUserStart     3

// for better readability
#define userMotorTurnRight 1
#define userMotorTurnLeft 0
#define userMotorRun 1
#define userMotorStop 0
#define userSwitchPressed LOW

#define Bit_MenueLvl1 0
#define Bit_MenueLvl2 1
#define Bit_MenueLvl3 2
#define Bit_Encoderredate 7
#define Bit_EncoderSwitch 6

//#define bladeThickness 30 // [0.1mm]

#define LED_blue      A0
#define LED_green     A1
#define LED_red       A2


// defintion EEPROM Data
const int ADDR_dovetail = 10;
const int ADDR_blade = 12;

// **************         Definition von eigenen Zeichen 

// Nut
byte Char0[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// Zinken
byte Char1[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

// Pfeil nach oben
byte Charup[8] = {
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};

// Pfeil nach unten
byte Chardown[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
};
