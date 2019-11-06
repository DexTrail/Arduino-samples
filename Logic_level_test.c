// Определение границ логических уровней реле

const byte pinOut = 3;  // Управляющий пин
const byte pinIn = 11;  // Обратная связь
const byte pinAnalog = A0;  // Аналоговый вход (измерение напряжения)

int incomingByte;     // Считываемый байт
double measuredVoltage;   // Введеное напряжение
//int analogIn;         // Значение на аналоговом входе

bool isDataGot;         // Флаг ввода
bool isDecimal;       // Флаг ввода десятичного числа
bool isWrong;         // Флаг неправильного ввода

void setup() {
  pinMode(pinOut, OUTPUT);
  pinMode(pinIn, INPUT);
  pinMode(pinAnalog, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  unsigned long timePassed; // Общее время
  unsigned long timePassedCheckInput; // Время на проверку ввода
  
  while(1) {
    measuredVoltage = 0;
    isDataGot = false;
    
    isDecimal = false;
    isWrong = false;
    
    digitalWrite(LED_BUILTIN, LOW);
    
    Serial.println("*****************************************************");
    Serial.println(analogRead(pinIn));
    Serial.println("Input measured voltage at 0 Ohms:");
    digitalWrite(pinOut, HIGH);
    do {
      timePassed = millis();
      timePassedCheckInput = millis();
      while(Serial.available()) {
        incomingByte = Serial.read();
  
        if(!isWrong) {
          CheckInput(incomingByte);
        }
  
        isDataGot = true;
        digitalWrite(LED_BUILTIN, HIGH);
        delay(10);
      }
    } while(!isDataGot);  // Ждем ввода
  
    digitalWrite(LED_BUILTIN, LOW);
    
    timePassedCheckInput = millis() - timePassedCheckInput;
    Serial.print("Time for check input: ");
    Serial.print(timePassedCheckInput);
    Serial.println(" ms");
    Serial.println();
    
    if(isWrong) {
      Serial.println("Wrong input!");
      Serial.println();
      break;
    }
    
    Serial.print("Measured voltage: ");
    Serial.print(measuredVoltage, 4);
    Serial.println(" V");
    Serial.println();

    CheckLevel();
    Serial.println();

    timePassed = millis() - timePassed;
    Serial.print("Time for full loop: ");
    Serial.print(timePassed);
    Serial.println(" ms");
    Serial.println();
  
    //delay(0);
  }
}

// Проверяем ввод и формируем число
void CheckInput(int newByte) {
  static float decimalPlace;      // Место в десятичной части
  static bool isStopCounting;     // Округлено до 4-го десятичного знака
  
  if(newByte >= 48 && newByte <= 57) {     // "0" = 48 ... "9" = 57
    if(!isStopCounting) {
      if(!isDecimal) {
        measuredVoltage = measuredVoltage * 10 + (newByte - 48);
        if(measuredVoltage >= 10000) {      // Введенное число должно быть меньше 10000
          isWrong = true;
        }
      } else {
        measuredVoltage = measuredVoltage + (newByte - 48) / pow(10, decimalPlace);
  
        if(decimalPlace == 5.0f) {        // Округляем до 4-го знака и останавливаем формирование числа
          measuredVoltage = round(measuredVoltage * 1e+4) * 1e-4;
          isStopCounting = true;
        }
    
        decimalPlace++;
      }
    }
  } else if(((newByte == 44) || (newByte == 46)) && !isDecimal) {     // "," = 44, "." = 46, не должно быть несколько
    isDecimal = true;
    decimalPlace = 1.0f;
    isStopCounting = false;
  } else {
    isWrong = true;
  }
}

// Проверяем логические уровни
void CheckLevel() {
  int analogIn;
  
  unsigned long timePassedCheckLevel;  // Время на проверку

  Serial.print("Make HIGH level signal... ");
  do {
     delay(500);
    if(digitalRead(pinIn)) {
      digitalWrite(LED_BUILTIN, HIGH);
      analogIn = CheckAnalog();
      Serial.print("OK at ");
      Serial.print(measuredVoltage / 1024 * analogIn);
      Serial.print(" V (");
      Serial.print(5.0 / 1024 * analogIn);
      Serial.print(" V )\t\tanalogIn: ");
      Serial.println(analogIn);
      break;
    }
  } while(1);

  Serial.println("Begin check LOW level...");
  Serial.println();
  
  timePassedCheckLevel = millis();

  while(digitalRead(pinIn)) {}
  
  digitalWrite(LED_BUILTIN, LOW);
  analogIn = CheckAnalog();
  Serial.print("LOW at ");
  Serial.print(measuredVoltage / 1024 * analogIn);
  Serial.print(" V (");
  Serial.print(5.0 / 1024 * analogIn);
  Serial.print(" V )\t\tanalogIn: ");
  Serial.println(analogIn);
  Serial.println();

  timePassedCheckLevel = millis() - timePassedCheckLevel;
  Serial.print("Time for check LOW: ");
  Serial.print(timePassedCheckLevel);
  Serial.println(" ms");
  Serial.println();

  // ***************************************
  
  Serial.print("Make LOW level signal... ");
  digitalWrite(LED_BUILTIN, HIGH);
  do {
    delay(500);
    if(!digitalRead(pinIn)) {
      digitalWrite(LED_BUILTIN, LOW);
      analogIn = CheckAnalog();
      Serial.print("OK at ");
      Serial.print(measuredVoltage / 1024 * analogIn);
      Serial.print(" V (");
      Serial.print(5.0 / 1024 * analogIn);
      Serial.print(" V )\t\tanalogIn: ");
      Serial.println(analogIn);
      break;
    }
  } while(1);

  Serial.println("Begin check HIGH level...");
  Serial.println();
  
  timePassedCheckLevel = millis();

  while(!digitalRead(pinIn)) {}
  
  digitalWrite(LED_BUILTIN, HIGH);
  analogIn = CheckAnalog();
  Serial.print("HIGH at ");
  Serial.print(measuredVoltage / 1024 * analogIn);
  Serial.print(" V (");
  Serial.print(5.0 / 1024 * analogIn);
  Serial.print(" V )\t\tanalogIn: ");
  Serial.println(analogIn);
  Serial.println();

  timePassedCheckLevel = millis() - timePassedCheckLevel;
  Serial.print("Time for check HIGH: ");
  Serial.print(timePassedCheckLevel);
  Serial.println(" ms");
  Serial.println();
}

// Считываем усредненное значение на аналоговом входе
int CheckAnalog() {
  int analogIn = 0;
  int iterations;

  
  //Serial.print("analogIn / iterations:");
  for(iterations = 1; iterations <= 3; iterations++) {
    delay(10);
    analogIn += analogRead(pinAnalog);
    //Serial.print("\t");
    //Serial.print(analogIn / iterations);
  }
  //Serial.println();

  return analogIn / (iterations - 1);
}
