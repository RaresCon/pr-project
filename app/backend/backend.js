const express = require('express');
const mongoose = require('mongoose');
const bodyParser = require('body-parser');
const cors = require('cors');
const fs = require('fs');
const mqtt = require('mqtt');
const nodemailer = require('nodemailer');
require('dotenv').config();

let emailTimeout = 0;
const EMAIL_TIMEOUT_MS = 5 * 60 * 1000;

mongoose.connect(process.env.MONGO_URI, {
})
  .then(() => console.log('Connected to MongoDB Atlas'))
  .catch((err) => console.error('MongoDB Atlas connection error:', err));

const readingSchema = new mongoose.Schema({
  temperature: Number,
  pressure: Number,
  airQuality: Number,
  timestamp: { type: Date, default: Date.now },
});

const boardSchema = new mongoose.Schema({
  boardId: String,
  readings: [readingSchema],
});

const Board = mongoose.model('Board', boardSchema);

const app = express();
app.use(cors());
app.use(bodyParser.json());

const transporter = nodemailer.createTransport({
  service: 'gmail',
  auth: {
    user: process.env.EMAIL_USER,
    pass: process.env.EMAIL_PASS,
  },
});

const sendEmail = async (boardId, subject, message, recipient) => {
  const now = Date.now();

  if (now - emailTimeout < EMAIL_TIMEOUT_MS) {
    console.log(`Email not sent for board ${boardId}: timeout not elapsed`);
    return;
  }

  console.log(message);
  try {
    emailTimeout = now;
    await transporter.sendMail({
      from: process.env.EMAIL_USER,
      to: recipient,
      subject: subject,
      text: message,
    });

    console.log('Email sent successfully!');
  } catch (error) {
    console.error('Error sending email:', error);
  }
};

const mqttOptions = {
  host: 'localhost',
  port: 8883,
  protocol: 'mqtts',
  rejectUnauthorized: false,
  ca: fs.readFileSync('../../auth/ca.crt'),
  cert: fs.readFileSync('backend.crt'),
  key: fs.readFileSync('backend.key'),
};

const mqttClient = mqtt.connect(mqttOptions);

mqttClient.on('connect', () => {
  console.log('Securely connected to MQTT broker');
  mqttClient.subscribe('sensor/data', (err) => {
    if (err) {
      console.error('Failed to subscribe to topic:', err);
    } else {
      console.log('Subscribed to topic: sensor/data');
    }
  });
});

mqttClient.on('message', async (topic, message) => {
  if (topic === 'sensor/data') {
    try {
      const data = JSON.parse(message.toString());

      if (!data.boardId || data.temperature === undefined || data.pressure === undefined) {
        console.error('Invalid data format:', data);
        return;
      }

      let board = await Board.findOne({ boardId: data.boardId });
      if (!board) {
        board = new Board({ boardId: data.boardId, readings: [] });
      }

      board.readings.push({
        temperature: data.temperature,
        pressure: data.pressure,
        airQuality: data.airQuality,
      });

      await board.save();
      console.log('New data added for board:', data.boardId);

      let emailMessage = '';
      if (data.temp_type === 'CRITICAL') {
        emailMessage += `Temperature is CRITICAL: ${data.temperature}°C\n`;
      }
      if (data.pres_type === 'CRITICAL') {
        emailMessage += `Pressure is CRITICAL: ${data.pressure} hPa\n`;
      }
      if (data.aq_type === 'CRITICAL') {
        emailMessage += `Air quality is CRITICAL: ${data.airQuality}\n`;
      }

      if (emailMessage) {
        sendEmail(
          data.boardId,
          `Sensor Alert: Critical Levels Detected for Board ${data.boardId}`,
          `Board ID: ${data.boardId}\n\n${emailMessage}`,
          'rares.constantin2002@gmail.com'
        );
      }
    } catch (err) {
      console.error('Failed to process MQTT message:', err);
    }
  }
});

app.get('/api/boards', async (req, res) => {
  try {
    const boards = await Board.find();
    res.json(boards);
  } catch (err) {
    res.status(500).send('Error fetching board data');
  }
});

app.get('/api/boards/:boardId', async (req, res) => {
  try {
    const board = await Board.findOne({ boardId: req.params.boardId });
    if (!board) {
      return res.status(404).send('Board not found');
    }
    res.json(board);
  } catch (err) {
    res.status(500).send('Error fetching board data');
  }
});

app.get('/api/boards/:boardId/readings', async (req, res) => {
  try {
    const board = await Board.findOne({ boardId: req.params.boardId });
    if (!board) {
      return res.status(404).send('Board not found');
    }
    res.json(board.readings);
  } catch (err) {
    res.status(500).send('Error fetching board readings');
  }
});

app.get('/api/boards/:boardId/data', async (req, res) => {
  const { boardId } = req.params;
  const { range } = req.query;

  let startTime = new Date();
  switch (range) {
    case 'last5m':
      startTime.setMinutes(startTime.getMinutes() - 5);
      break;
      case 'last30m':
      startTime.setMinutes(startTime.getMinutes() - 30);
      break;
    case 'last1h':
      startTime.setHours(startTime.getHours() - 1);
      break;
    case 'last6h':
      startTime.setHours(startTime.getHours() - 6);
      break;
    case 'last12h':
      startTime.setHours(startTime.getHours() - 12);
      break;
    case 'last24h':
      startTime.setDate(startTime.getDate() - 1);
      break;
    case 'last7d':
      startTime.setDate(startTime.getDate() - 7);
      break;
    default:
      return res.status(400).send('Invalid time range');
  }

  try {
    const board = await Board.findOne({ boardId: boardId });
    if (!board) {
      return res.status(404).send('Board not found');
    }

    const filteredReadings = board.readings.filter((reading) => new Date(reading.timestamp) >= startTime);
    res.json(filteredReadings);
  } catch (err) {
    console.error('Error fetching board data:', err);
    res.status(500).send('Error fetching board data');
  }
});

app.post('/api/boards/:boardId/disable', (req, res) => {
  const { boardId } = req.params;

  mqttClient.publish('sensor/command', JSON.stringify({ boardId, command: 'disable' }), (err) => {
    if (err) {
      console.error('Failed to send disable command:', err);
      res.status(500).send('Failed to disable board');
    } else {
      res.sendStatus(200);
    }
  });
});

app.post('/api/boards/:boardId/enable', (req, res) => {
  const { boardId } = req.params;

  mqttClient.publish('sensor/command', JSON.stringify({ boardId, command: 'enable' }), (err) => {
    if (err) {
      console.error('Failed to send enable command:', err);
      res.status(500).send('Failed to enable board');
    } else {
      res.sendStatus(200);
    }
  });
});

app.get('/api/boards/:boardId/alarm', (req, res) => {
  const { boardId } = req.params;

  mqttClient.publish('sensor/command', JSON.stringify({
    boardId,
    command: 'alarm',
  })), (err) => {
    if (err) {
      console.error('Failed to send MQTT message:', err);
      return res.status(500).json({ error: 'Failed to send MQTT message' });
    }
  }
  res.json({ message: `Alarm is toggled` });
});

const PORT = 5000;
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
