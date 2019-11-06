const byte pinLED_Power = 3;  // Светодиод Power
const byte pinLED_Aux1 = 5;   // Светодиод Дополнительный1 
const byte pinVmeter = 9;     // Вольметр

byte Vmeter_val = 0;          // Значение вольтметра
const byte Vmeter_Low = 62;   // Минимальное значение вольтметра
const byte Vmeter_High = 242; // Максимальное значение вольтметра
boolean LED_Power_State = HIGH;
boolean LED_Aux1_State = HIGH;

unsigned long curTime = 0;    // DEBUG
unsigned long newTime = 0;    // DEBUG

unsigned long timer0 = 0;
unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long timer3 = 0;

const int Vmeter_Time = 250;
const int LED_Power_Time = 200;
const int LED_Aux1_Time = 100;

void setup() {
  pinMode(pinLED_Power, OUTPUT);
  pinMode(pinLED_Aux1, OUTPUT);
  pinMode(pinVmeter, OUTPUT);

  Serial.begin(9600);          // DEBUG
  randomSeed(analogRead(0));
}

void loop() {
  curTime = micros();          // DEBUG

  timer0 = millis();

  // Если таймаут вольтметра прошел, меняем целевое значение вольтметра
  if(timer1 <= timer0) {
    timer1 = timer0 + Vmeter_Time;
    Vmeter_val = byte((random(Vmeter_Low, Vmeter_High) + random(Vmeter_Low, Vmeter_High) + random(Vmeter_Low, Vmeter_High)) / 3);
  }
  analogWrite(pinVmeter, Vmeter_val);  // TODO: Сделать плавный ход стрелки

  // При выполнении условий включаем режим мигания, выключаем диод (0 - 43 мА = 6,0 ед. на 1 мА)
  if(timer2 == 0 && Vmeter_val >= 186) {
    timer2 = timer0 + LED_Power_Time;
    LED_Power_State = LOW;
  }
  if(timer3 == 0 && Vmeter_val <= 116) {
    timer3 = timer0 + LED_Aux1_Time;
    LED_Aux1_State = LOW;
  }

  // Включаем диод, если {режим мигания} && {прошло время полутакта} && {диод выключен - нужно, чтобы цикл прошел хоть раз}
  if(timer2 != 0 && timer2 <= timer0 && LED_Power_State == LOW) {
    timer2 = timer0 + LED_Power_Time;
    LED_Power_State = !LED_Power_State;
  }
  if(timer3 != 0 && timer3 <= timer0 && LED_Aux1_State == LOW) {
    timer3 = timer0 + LED_Aux1_Time;
    LED_Aux1_State = !LED_Aux1_State;
  }

  // Сбрасываем режим мигания, если {режим мигания} && {прошло время полутакта} && {диод включен - нужно, чтобы цикл прошел хоть раз}
  if(timer2 != 0 && timer2 <= timer0 && LED_Power_State == HIGH) timer2 = 0;
  if(timer3 != 0 && timer3 <= timer0 && LED_Aux1_State == HIGH) timer3 = 0;

  // Устанавливаем диоды
  digitalWrite(pinLED_Power, LED_Power_State);
  digitalWrite(pinLED_Aux1, LED_Aux1_State);
  
  newTime = micros();         // DEBUG
  
  Serial.print(curTime);      // DEBUG...
  Serial.print("\t\t");
  Serial.print(newTime);
  Serial.print("\t\t");
  Serial.println(newTime-curTime);  // ...DEBUG
}
