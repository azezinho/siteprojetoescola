int pirPin = 3;        // PIR
int ledPin = 4;        // LED
int buttonLight = 7;   // Botão para alternar luz
int buttonSensor = 8;  // Botão para ligar/desligar sensor

bool luzLigada = false;
bool sensorOn = true;  // sensor começa ligado

void sendStatus() {
  Serial.print("STATUS LIGHT=");
  Serial.print(luzLigada ? "ON" : "OFF");
  Serial.print(" SENSOR=");
  Serial.println(sensorOn ? "ON" : "OFF");
}

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonLight, INPUT_PULLUP);  // botão entre pino e GND
  pinMode(buttonSensor, INPUT_PULLUP); // botão entre pino e GND
  Serial.begin(9600);
  delay(1000);
  sendStatus();
}

String readSerialLine() {
  String s = "";
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') break;
    if (c != '\r') s += c;
  }
  s.trim();
  return s;
}

void loop() {
  static bool lastButtonLight = HIGH;
  static bool lastButtonSensor = HIGH;

  bool currentButtonLight = digitalRead(buttonLight);
  bool currentButtonSensor = digitalRead(buttonSensor);

  // --- Botão da luz (toggle manual) ---
  if (lastButtonLight == HIGH && currentButtonLight == LOW) {
    luzLigada = !luzLigada;
    if (luzLigada) {
      digitalWrite(ledPin, HIGH);
      Serial.println("Luz ligada manualmente!");
    } else {
      digitalWrite(ledPin, LOW);
      Serial.println("Luz desligada manualmente!");
    }
    sendStatus();
    delay(300); // debounce
  }
  lastButtonLight = currentButtonLight;

  // --- Botão do sensor (liga/desliga PIR) ---
  if (lastButtonSensor == HIGH && currentButtonSensor == LOW) {
    sensorOn = !sensorOn;
    if (sensorOn) {
      Serial.println("Sensor ativado!");
    } else {
      Serial.println("Sensor desativado!");
      digitalWrite(ledPin, LOW); // garante luz apagada se só o sensor estava controlando
    }
    sendStatus();
    delay(300); // debounce
  }
  lastButtonSensor = currentButtonSensor;

  // --- Controle do PIR (só funciona se sensor estiver ligado) ---
  if (sensorOn) {
    int pirState = digitalRead(pirPin);
    if (pirState == HIGH) {
      digitalWrite(ledPin, HIGH);
      Serial.println("Movimento detectado pelo sensor!");
      sendStatus();
      delay(200); // evita flood
    } else if (!luzLigada) {
      // só apaga se não estiver ligada manualmente
      digitalWrite(ledPin, LOW);
      Serial.println("Nenhum movimento.");
      sendStatus();
    }
  }

  // --- Comandos via Serial ---
  if (Serial.available()) {
    String cmd = readSerialLine();
    if (cmd.length() > 0) {
      cmd.toUpperCase();
      if (cmd == "LIGHT ON" || cmd == "LIGHTON") {
        luzLigada = true;
        digitalWrite(ledPin, HIGH);
        Serial.println("Luz ligada via serial!");
        sendStatus();
      } else if (cmd == "LIGHT OFF" || cmd == "LIGHTOFF") {
        luzLigada = false;
        digitalWrite(ledPin, LOW);
        Serial.println("Luz desligada via serial!");
        sendStatus();
      } else if (cmd == "TOGGLE LIGHT" || cmd == "TOGGLE_LIGHT" || cmd == "TOGGLE") {
        luzLigada = !luzLigada;
        digitalWrite(ledPin, luzLigada ? HIGH : LOW);
        Serial.println("Luz alternada via serial!");
        sendStatus();
      } else if (cmd == "SENSOR ON" || cmd == "SENSORON") {
        sensorOn = true;
        Serial.println("Sensor ativado via serial!");
        sendStatus();
      } else if (cmd == "SENSOR OFF" || cmd == "SENSOROFF") {
        sensorOn = false;
        digitalWrite(ledPin, LOW);
        Serial.println("Sensor desativado via serial!");
        sendStatus();
      } else if (cmd == "STATUS") {
        sendStatus();
      } else {
        Serial.print("CMD UNKNOWN: ");
        Serial.println(cmd);
      }
    }
  }
}
