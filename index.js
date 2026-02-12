const express = require('express');
const http = require('http');
const path = require('path');
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const WebSocket = require('ws');

const app = express();
const server = http.createServer(app);
const io = require('socket.io')(server);

const PORT = process.env.PORT || 3000;
const SERIAL_PATH = process.env.SERIAL_PORT; // se não setado => modo simulado
const BAUD = parseInt(process.env.BAUD_RATE || '9600', 10);

app.use(express.static(path.join(__dirname, 'public')));

let port = null;
let deviceWs = null; // conexão de dispositivo via WebSocket (ESP32)

const serialEnabled = !!SERIAL_PATH;

if (serialEnabled) {
  try {
    port = new SerialPort(SERIAL_PATH, { baudRate: BAUD });
    const parser = port.pipe(new Readline({ delimiter: '\n' }));
    parser.on('data', line => {
      const text = line.toString().trim();
      console.log('> [ARDUINO]', text);
      io.emit('log', text);
    });
    port.on('error', err => console.error('Serial port error:', err.message));
  } catch (e) {
    console.error('Erro ao abrir porta serial:', e.message);
    port = null;
  }
}

// Simulador de dispositivo quando não há serial nem ESP conectado
// Separação entre controle manual (manualLight) e PIR (pirActive).
const simulator = {
  manualLight: false, // ligado manualmente pelo usuário
  pirActive: false,   // estado atual do PIR (detecção)
  sensorOn: true,
  emit(text) {
    console.log('> [SIM]', text);
    io.emit('log', text);
  },
  sendStatus() {
    const lightOn = this.manualLight ? 'ON' : 'OFF';
    this.emit(`STATUS LIGHT=${lightOn} SENSOR=${this.sensorOn ? 'ON' : 'OFF'}`);
  },
  handleCommand(cmd) {
    const c = ('' + cmd).toUpperCase().trim();
    if (c === 'LIGHT ON' || c === 'LIGHTON') {
      this.manualLight = true; this.emit('Luz ligada (sim, manual)');
    } else if (c === 'LIGHT OFF' || c === 'LIGHTOFF') {
      this.manualLight = false; this.emit('Luz desligada (sim, manual)');
    } else if (c === 'TOGGLE' || c === 'TOGGLE LIGHT' || c === 'TOGGLE_LIGHT') {
      this.manualLight = !this.manualLight; this.emit('Luz alternada (sim, manual)');
    } else if (c === 'SENSOR ON' || c === 'SENSORON') {
      this.sensorOn = true; this.emit('Sensor ativado (sim)');
    } else if (c === 'SENSOR OFF' || c === 'SENSOROFF') {
      this.sensorOn = false; this.pirActive = false; this.emit('Sensor desativado (sim)');
    } else if (c === 'STATUS') {
      this.sendStatus();
      return;
    } else {
      this.emit('CMD UNKNOWN: ' + cmd);
    }
    this.sendStatus();
  }
};

// Simula detecção de movimento ocasional quando sensorOn=true
setInterval(() => {
  if (!serialEnabled && !deviceWs) {
    if (simulator.sensorOn && Math.random() < 0.15) {
      // detect motion: emit event but DO NOT change light state (user-controlled)
      simulator.emit('Movimento detectado pelo sensor (simulado)!');
      // send status showing sensor only
      simulator.sendStatus();
      setTimeout(() => {
        simulator.emit('Nenhum movimento (simulado).');
        simulator.sendStatus();
      }, 3000);
    }
  }
}, 3000);

// WebSocket server para dispositivo (ESP32) conectar em /device
const wss = new WebSocket.Server({ server, path: '/device' });
wss.on('connection', (ws, req) => {
  console.log('Device connected via WebSocket');
  deviceWs = ws;
  ws.on('message', message => {
    const text = ('' + message).trim();
    console.log('> [DEVICE]', text);
    io.emit('log', text);
  });
  ws.on('close', () => {
    console.log('Device disconnected');
    deviceWs = null;
  });
});

io.on('connection', socket => {
  console.log('Client connected');
  socket.on('command', cmd => {
    console.log('CMD from client:', cmd);
    // prioridade: dispositivo via WS -> serial -> simulador
    if (deviceWs && deviceWs.readyState === WebSocket.OPEN) {
      deviceWs.send(cmd);
    } else if (port && port.writable) {
      port.write(cmd + '\n', err => { if (err) console.error('Write error:', err.message); });
    } else {
      simulator.handleCommand(cmd);
    }
  });
});

server.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
  if (serialEnabled && port) console.log(`Serial port: ${SERIAL_PATH} @ ${BAUD}`);
  else console.log('No serial device configured — running in simulated mode.');
});
