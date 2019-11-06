// Определение границ логических уровней реле

const int pinOut = 5; // Управляющий пин
const int pinIn = A0;  // Обратная связь

int incomingByte;     // Считываемый байт
double measuredVoltage;   // Введеное напряжение
bool dataGot;         // Флаг ввода

bool isDecimal;       // Флаг ввода десятичного числа
bool isWrong;         // Флаг неправильного ввода

//void(* resetFunc) (void) = 0; //Программный Reset

void setup() {
  pinMode(pinOut, OUTPUT);
  pinMode(pinIn, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  unsigned long timePassed; // Общее время
  unsigned long timePassedCheckInput; // Время на проверку ввода
  
  while(1) {    
    measuredVoltage = 0;
    dataGot = false;
    
    isDecimal = false;
    isWrong = false;
    
    Serial.println("*****************************************************");
    Serial.println(analogRead(pinIn));
    Serial.println("Input measured voltage:");
    do {
      timePassed = millis();
      timePassedCheckInput = millis();
      while(Serial.available()) {
        incomingByte = Serial.read();
  
        if(!isWrong) {
          CheckInput(incomingByte);
        }
  
        dataGot = true;
        digitalWrite(LED_BUILTIN, HIGH);
        delay(10);
      }
    } while(!dataGot);  // Ждем ввода
  
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
    //resetFunc();      // Reset
  }
}

// Проверяем логические уровни
void CheckLevel() {
  const byte precision = 5;
  byte A, B;
  int C;
  byte C_in;
  int count;

  unsigned long timePassedCheckLevel = millis();  // Время на проверку
  
  Serial.println("Begin check HIGH level...");
  Serial.println();
  
  Serial.println("Check by mean...");
  A = 255;
  B = 0;
  count = 0;
  do {
    C = (A + B) / 2.0;
    C_in = CheckRelay(C, 614);

    if(C_in == HIGH) {
      A = C;
    } else {
      B = C;
    }

    count++;
  } while((A-B) > precision);
  
  Serial.print("HIGH at ");
  Serial.print(measuredVoltage / 256 * C);
  Serial.print(" V (");
  Serial.print(5.0 / 256 * C);
  Serial.print(" V )\t\tC (A/B): ");
  
  Serial.print(C);
  Serial.print(" (");
  Serial.print(A);
  Serial.print("/");
  Serial.print(B);
  Serial.print(")");
  Serial.println();
  
  Serial.print("Steps: ");
  Serial.println(count);
  Serial.println();

  Serial.println("Check by step up...");
  for(C = 0; C <= 255; C += precision) {
    C_in = CheckRelay(C, 614);
    if(C_in) break;
  }

  Serial.print("HIGH at ");
  Serial.print(measuredVoltage / 256 * C);
  Serial.print(" V (");
  Serial.print(5.0 / 256 * C);
  Serial.print(" V )\t\tC: ");
  Serial.println(C);
  Serial.println();

  timePassedCheckLevel = millis() - timePassedCheckLevel;
  Serial.print("Time for check HIGH: ");
  Serial.print(timePassedCheckLevel);
  Serial.println(" ms");
  Serial.println();

// ********************************************
  timePassedCheckLevel = millis();
  Serial.println("Begin check LOW level...");
  Serial.println();
  
  Serial.println("Check by mean...");
  A = 255;
  B = 0;
  count = 0;
  do {
    C = (A + B) / 2.0;
    C_in = CheckRelay(C, 410);

    if(C_in == HIGH) {
      A = C;
    } else {
      B = C;
    }

    count++;
  } while((A-B) > precision);
  
  Serial.print("LOW at ");
  Serial.print(measuredVoltage / 256 * C);
  Serial.print(" V (");
  Serial.print(5.0 / 256 * C);
  Serial.print(" V )\t\tC (A/B): ");
  
  Serial.print(C);
  Serial.print(" (");
  Serial.print(A);
  Serial.print("/");
  Serial.print(B);
  Serial.print(")");
  Serial.println();
  
  Serial.print("Steps: ");
  Serial.println(count);
  Serial.println();

  Serial.println("Check by step down...");
  for(C = 255; C >= 0; C -= precision) {
    C_in = CheckRelay(C, 410);
    if(C_in) break;
  }

  Serial.print("LOW at ");
  Serial.print(measuredVoltage / 256 * C);
  Serial.print(" V (");
  Serial.print(5.0 / 256 * C);
  Serial.print(" V )\t\tC: ");
  Serial.println(C);
  Serial.println();

  timePassedCheckLevel = millis() - timePassedCheckLevel;
  Serial.print("Time for check LOW: ");
  Serial.print(timePassedCheckLevel);
  Serial.println(" ms");
}

int CheckRelay (int PWM_out, int threshold) {
  const int delayBetweenChecks = 1000;
  bool level = LOW;

  analogWrite(pinOut, PWM_out);
  delay(delayBetweenChecks);
  for(int i = 1; i <= 3; i++) {
    for(int j = 1; j <= 100; j++) {
      if(analogRead(pinIn) >= threshold) {
        level = HIGH;
        break;
      }
    }
    if(level) break;
    delayMicroseconds(47563);
  }
  digitalWrite(LED_BUILTIN, level);
  analogWrite(pinOut, 0);
  delay(delayBetweenChecks);
  digitalWrite(LED_BUILTIN, LOW);

  return level;
}

