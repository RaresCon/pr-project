const express = require('express');
const mongoose = require('mongoose');
const bodyParser = require('body-parser');
const cors = require('cors');
const fs = require('fs');
const mqtt = require('mqtt');
require('dotenv').config(); // Load environment variables from .env file

// MongoDB Atlas connection
mongoose.connect(process.env.MONGO_URI, {
  useNewUrlParser: true,
  useUnifiedTopology: true,
})
  .then(() => console.log('Connected to MongoDB Atlas'))
  .catch((err) => console.error('MongoDB Atlas connection error:', err));

// Define a Mongoose schema for sensor readings
const readingSchema = new mongoose.Schema({
  temperature: Number,
  pressure: Number,
  timestamp: { type: Date, default: Date.now },
});

// Define a Mongoose schema for boards
const boardSchema = new mongoose.Schema({
  boardId: String,
  readings: [readingSchema],
});

const Board = mongoose.model('Board', boardSchema);

// Initialize Express app
const app = express();
app.use(cors());
app.use(bodyParser.json());

// MQTT secure connection options
const mqttOptions = {
  host: 'your-broker-address.com', // Replace with your MQTT broker address
  port: 8883, // Standard MQTT secure port
  protocol: 'mqtts',
  rejectUnauthorized: true,
  ca: fs.readFileSync('ca.crt'), // CA certificate
  cert: fs.readFileSync('client.crt'), // Client certificate (optional)
  key: fs.readFileSync('client.key'), // Client key (optional)
};

// Connect to the MQTT broker
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

      // Validate the data
      if (!data.boardId || data.temperature === undefined || data.pressure === undefined) {
        console.error('Invalid data format:', data);
        return;
      }

      // Find the board or create a new one
      let board = await Board.findOne({ boardId: data.boardId });
      if (!board) {
        board = new Board({ boardId: data.boardId, readings: [] });
      }

      // Add the new reading to the board
      board.readings.push({
        temperature: data.temperature,
        pressure: data.pressure,
      });

      // Save the updated board to the database
      await board.save();
      console.log('New data added for board:', data.boardId);
    } catch (err) {
      console.error('Failed to process MQTT message:', err);
    }
  }
});

// API endpoint to get all board data
app.get('/api/boards', async (req, res) => {
  try {
    const boards = await Board.find();
    res.json(boards);
  } catch (err) {
    res.status(500).send('Error fetching board data');
  }
});

// API endpoint to get the readings for a specific board
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

// Start the Express server
const PORT = 5000;
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
