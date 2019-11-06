/* MeteoStation
 *  ver. 0.0.5
 *  
 *  ATTENTION!!!
 *  Need reworking to solve overflowing timer problem.
 *  Current version won't work after overflow (49 days 17 hours).
 */

#define _VERSION_ "0.0.5"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args) write(args);
#else
#define printByte(args) print(args, BYTE);
#endif

/*
uint8_t bell[8] = {0x4, 0xe, 0xe, 0xe, 0x1f, 0x0, 0x4};
uint8_t note[8] = {0x2, 0x3, 0x2, 0xe, 0x1e, 0xc, 0x0};
uint8_t clock[8] = {0x0, 0xe, 0x15, 0x17, 0x11, 0xe, 0x0};
uint8_t heart[8] = {0x0, 0xa, 0x1f, 0x1f, 0xe, 0x4, 0x0};
uint8_t duck[8] = {0x0, 0xc, 0x1d, 0xf, 0xf, 0x6, 0x0};
uint8_t check[8] = {0x0, 0x1, 0x3, 0x16, 0x1c, 0x8, 0x0};
uint8_t cross[8] = {0x0, 0x1b, 0xe, 0x4, 0xe, 0x1b, 0x0};
uint8_t retarrow[8] = {0x1, 0x1, 0x5, 0x9, 0x1f, 0x8, 0x4};
*/

LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint8_t DHTPIN = 2;
float humi;
float temp;

DHT dht(DHTPIN, DHT11);

const uint32_t measurmentTime = 600000;
const uint32_t backlightTime = 20000;
//const uint32_t betweenMeasurmentsTime = 2000;
uint32_t timer0;  // Current time
uint32_t timer1 = 0;  // Measurment time
uint32_t timer2;  // Back-light time
uint32_t timer3;  // Between-measuremnts time

void setup() {
  dht.begin();
  
  lcd.init();
  lcd.backlight();
  /*
  lcd.createChar(0, bell);
  lcd.createChar(1, note);
  lcd.createChar(2, clock);
  lcd.createChar(3, heart);
  lcd.createChar(4, duck);
  lcd.createChar(5, check);
  lcd.createChar(6, cross);
  lcd.createChar(7, retarrow);
  */
  lcd.home();
  //lcd.print("     Hello!");
  lcd.print("  MeteoStation");
  lcd.setCursor(0, 1);
  lcd.print("   ver.");
  lcd.print(_VERSION_);
  //delay(1000);
  //displayKeyCodes();
}

void displayKeyCodes(void) {
  uint8_t i = 0;
  while(1) {
    lcd.clear();
    lcd.print("Codes 0x");
    lcd.print(i, HEX);
    lcd.print("-0x");
    lcd.print(i + 16, HEX);
    lcd.setCursor(0, 1);
    for(int j = 0; j < 16; j++) {
      lcd.printByte(i + j);
    }
    i += 16;
    delay(4000);
  }
}

void loop() {
  timer0 = millis();
  if(timer1 <= timer0) {
    timer1 = timer0 + measurmentTime;
    Measure();
  }

  if(timer2 <= timer0){
    lcd.noBacklight();
    timer2 = 4294967295;
  }
}

void Measure() {
  float tempCur, tempMin, tempMax;
  float humiCur, humiMin, humiMax;
  humi = temp = 0;
  
  for(int i = 0; i < 12; i++) {
    tempCur = dht.readTemperature();
    humiCur = dht.readHumidity();
    
    timer3 = millis() + 2000;

    if(i == 0 || tempMin > tempCur) {
      tempMin = tempCur;
    }
    if(tempMax < tempCur) {
      tempMax = tempCur;
    }
    if(i == 0 || humiMin > humiCur) {
      humiMin = humiCur;
    }
    if(humiMax < humiCur) {
      humiMax = humiCur;
    }
    
    temp += tempCur;
    humi += humiCur;
    
    while(millis() < timer3);
  }

  temp = (temp - tempMin - tempMax) / 10;
  humi = (humi - humiMin - humiMax) / 10 - 8;

  lcd.backlight();
  timer2 = millis() + backlightTime;
  
  if(isnan(humi) || isnan(temp)) {
    lcd.clear();
    lcd.print("   Measurment");
    lcd.setCursor(0, 1);
    lcd.print("     error!");
    return;
  }
  
  lcd.clear();
  lcd.print("TEMP: ");
  lcd.print(temp);
  lcd.print(" ");
  lcd.printByte(0xdf);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("HUMI: ");
  lcd.print(humi);
  lcd.print(" %");
}

