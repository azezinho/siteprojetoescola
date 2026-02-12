#include <WiFi.h>
#include <WebSocketsClient.h>

// --- CONFIGURE AQUI ---
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* serverHost = "192.168.1.100"; // IP do servidor (onde o node está rodando)
const uint16_t serverPort = 3000;
// -----------------------

WebSocketsClient webSocket;

bool luzLigada = false;
bool sensorOn = true;

void sendStatus() {
  String s = "STATUS LIGHT=";
  s += (luzLigada ? "ON" : "OFF");
  s += " SENSOR=";
  s += (sensorOn ? "ON" : "OFF");
  webSocket.sendTXT(s);
}

void handleCommand(String cmd) {
  cmd.toUpperCase();
  cmd.trim();
  if (cmd == "LIGHT ON" || cmd == "LIGHTON") {
    luzLigada = true;
    webSocket.sendTXT("Luz ligada via WiFi!");
  } else if (cmd == "LIGHT OFF" || cmd == "LIGHTOFF") {
    luzLigada = false;
    webSocket.sendTXT("Luz desligada via WiFi!");
  } else if (cmd == "TOGGLE" || cmd == "TOGGLE LIGHT" || cmd == "TOGGLE_LIGHT") {
    luzLigada = !luzLigada;
    webSocket.sendTXT("Luz alternada via WiFi!");
  } else if (cmd == "SENSOR ON" || cmd == "SENSORON") {
    sensorOn = true;
    webSocket.sendTXT("Sensor ativado via WiFi!");
  } else if (cmd == "SENSOR OFF" || cmd == "SENSOROFF") {
    sensorOn = false;
    luzLigada = false;
    webSocket.sendTXT("Sensor desativado via WiFi!");
  } else if (cmd == "STATUS") {
    sendStatus();
  } else {
    webSocket.sendTXT("CMD UNKNOWN: " + cmd);
  }
  sendStatus();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Desconectado");
      break;
    case WStype_CONNECTED:
      Serial.println("[WS] Conectado ao servidor");
      sendStatus();
      break;
    case WStype_TEXT: {
      String msg = String((char*)payload);
      msg.trim();
      Serial.println("[WS] Recebido: " + msg);
      handleCommand(msg);
      break;
    }
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.print("WiFi conectado: "); Serial.println(WiFi.localIP());

  webSocket.begin(serverHost, serverPort, "/device");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

unsigned long lastSim = 0;

void loop() {
  webSocket.loop();

  // Simula PIR: ocasionalmente envia movimento se sensorOn=true
  if (sensorOn && millis() - lastSim > 8000) {
    if (random(0, 100) < 20) {
      webSocket.sendTXT("Movimento detectado pelo sensor!");
      luzLigada = true;
      sendStatus();
      delay(2000);
      luzLigada = false;
      webSocket.sendTXT("Nenhum movimento.");
      sendStatus();
    }
    lastSim = millis();
  }
}
