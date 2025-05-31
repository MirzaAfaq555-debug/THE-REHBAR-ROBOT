#include <LiquidCrystal_74HC595.h>

// Define Arduino pins connected to 74HC165
#define DATA_165 2  // Connected to Q7 (Serial out) PD2
#define CLOCK_165 4 // Connected to CLK (SRCLK)     PD4
#define LATCH_165 3 // Connected to SH/PL (LATCH)   PD3

// Define Arduino pins connected to 74HC595
#define DATA_595 9  // Connected to DS (SER)        PB1
#define CLOCK_595 8 // Connected to SHCP (SRCLK)    PB0
#define LATCH_595 7 // Connected to STCP (RCLK)     PD7

// Define shift register outputs assigned to LCD pins
#define RS 0  // Q0 → LCD RS
#define EN 2  // Q1 → LCD EN
#define D4 3  // Q2 → LCD D4
#define D5 4  // Q3 → LCD D5
#define D6 5  // Q4 → LCD D6
#define D7 6  // Q5 → LCD D7
#define Bklit 7 // Q6 → LCD Backlight transistor
#define volSense A0
#define L_speed(l) analogWrite(6 , l)
#define R_speed(r) analogWrite(5 , r)
int fullSpeed = 200;

float raw_vol = 0 ;
float voltage = 0;
uint8_t dataIn = 0;
bool sensorValues[8];
uint8_t IR_Val;
uint8_t regOut = 0;

// Initialize the LCD with the shift register
LiquidCrystal_74HC595 lcd(DATA_595, CLOCK_595, LATCH_595, RS, EN, D4, D5, D6, D7, Bklit , 1);

void setup() {
    digitalWrite(LATCH_165, 1);
    DDRB |= 0b00000011;
    DDRD |= 0b10011000;
    DDRD &= 0b11111011;
    //pinMode(A2, INPUT_PULLUP);
    //pinMode(A3, INPUT_PULLUP);
    /*pinMode(DATA_PIN ,OUTPUT);
    pinMode(CLOCK_PIN ,OUTPUT);
    pinMode(LATCH_PIN ,OUTPUT);
    //pinMode(volSense ,INPUT);
*/
    //lcd.setExtraBits(0b00001111);
    lcd.backlitEnable(1);
    delay(1000);
    lcd.begin(16, 2);  // Initialize a 16x2 LCD
    lcd.print("Hello, World!");
    lcd.setCursor(0,1);
    lcd.print("Bye, World!");
    delay(1000);
    lcd.backlitEnable(0);
    //Serial.begin(9600);
    delay(2000);
    lcd.backlitEnable(1);
    lcd.clear();
    lcd.print("Voltage = ");
}

void loop() {


    dataIn = shiftIn165(DATA_165, CLOCK_165, LATCH_165);
    for (int i = 0; i < 8; i++) {
    sensorValues[i] = (dataIn >> i) & 1;
    }
    
    //lcd.setExtraBits(dataIn);
    raw_vol = analogRead(volSense);
    voltage = ((raw_vol/1024)*15);//-0.10;
    //lcd.clear();
    lcd.setCursor(10,0);
    
    //setCursor(11,1);
    lcd.print(voltage);
    //lcd.setCursor(0,1);
    //for(int i = 0; i < 8; i++)
    //{
    //  lcd.print(sensorValues[i]);
    //}
    //delay(100);

    bool S1 = sensorValues[0]; // far left
    bool S2 = sensorValues[1];
    bool S3 = sensorValues[2]; // center
    bool S4 = sensorValues[3];
    bool S5 = sensorValues[4]; // far right

  // Movement logic
  if (S2 && !S3 && S4 && S1 && S5) {       //s1 left most  &  s5 right most
    moveForward(fullSpeed, fullSpeed); // Straight
  }
  else if ((!S2 && !S3) || (!S2 && S1)) {
    moveForward(0, fullSpeed); // Slight Left
  }
  else if (!S1) {
    turnLeftSharp(); // Sharp Left
  }
  else if ((!S3 && !S4) || (!S4 && S5)) {
    moveForward(fullSpeed, 0); // Slight Right
  }
  else if (!S5) {
    turnRightSharp(); // Sharp Right
  }
  else {
    stopMotors(); // Line lost
  }
  fullSpeed = 100;
}


// Motor control functions
void moveForward(int leftSpeed, int rightSpeed) {
  motorD(0b0110);
  L_speed(leftSpeed);
  R_speed(rightSpeed);
}

void turnLeftSharp() {
  //1010
  motorD(0b1010);
  L_speed(fullSpeed);
  R_speed(fullSpeed);
}

void turnRightSharp() {
  //0101
  motorD(0b0101);
  L_speed(fullSpeed);
  R_speed(fullSpeed);
}

void stopMotors() {
  motorD(0b0000);
  L_speed(0);
  R_speed(0);
}

void motorD(uint8_t val)
{
  regOut &= 0xF0;
  regOut |= val;
  lcd.setExtraBits(regOut);
}

uint8_t shiftIn165(int _data, int _clock, int _latch)
{
  uint8_t value = 0;
  //PORTD &= 0B11100111; 
  digitalWrite(_clock, 0);
  digitalWrite(_latch, 0);
  digitalWrite(_latch, 1);
  for (int i = 0; i < 8; i++) {
    uint8_t bit = digitalRead(_data);
    if (bit == 1) {
      value |= (1 << 7-i);
    }
    digitalWrite(_clock, 1); // Shift out the next bit
    digitalWrite(_clock, 0);
  }
  return value;
}