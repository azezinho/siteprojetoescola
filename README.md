# SmartHome — Web UI + Serial Bridge

Pequeno projeto que conecta um Arduino (PIR + LED + botões) a uma interface web via porta serial.

Como usar

1. Grave o sketch em `sketch/SmartHome/SmartHome.ino` no seu Arduino (ex.: UNO, Nano, etc.).
2. No Windows, descubra a porta COM do Arduino (ex.: COM3). Exporte a variável `SERIAL_PORT` ou edite o `index.js`.
3. Abra um terminal em `server` e instale dependências:

```bash
cd server
npm install
```

4. Inicie o servidor (ajuste SERIAL_PORT se necessário):

```bash
set SERIAL_PORT=COM3
npm start
```

5. Abra http://localhost:3000 no navegador. Use os botões para enviar comandos ao Arduino. Logs e status aparecem na interface.

Comandos suportados (via serial):
- `LIGHT ON`, `LIGHT OFF`, `TOGGLE` — controlar luz manualmente
- `SENSOR ON`, `SENSOR OFF` — ativar/desativar controle por PIR
- `STATUS` — pede o estado atual

Modo desconectado / simulado

Se você não tiver o Arduino/ESP disponível, inicie o servidor sem definir `SERIAL_PORT`. O servidor entrará em modo simulado e reagirá aos comandos vindos do frontend, além de gerar eventos de movimento simulados para testar a interface.

Exemplo (Windows):

```bash
cd server
rem não exporta SERIAL_PORT
npm start
```

Usando ESP32 (Wi‑Fi)

1. Abra `sketch/ESP32/SmartHome_ESP32.ino` e configure `ssid`, `password` e `serverHost` para o IP onde o servidor Node está rodando.
2. Instale a biblioteca `arduinoWebSockets` (ou `WebSockets` / `WebSocketsClient`) via Library Manager.
3. Grave no ESP32. O ESP32 se conectará ao servidor pelo WebSocket em `ws://<server>:3000/device` e trocará mensagens de texto (mesmo formato do serial).

Com o ESP32 conectado, o servidor encaminhará comandos do frontend diretamente ao dispositivo via WebSocket. Se houver serial e ESP conectados ao mesmo tempo, o servidor prioriza o dispositivo via WebSocket.
