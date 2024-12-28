const express = require("express");
const mqtt = require("mqtt");

// MQTT Broker Configuration (Local Mosquitto Broker)
const mqttBroker = "mqtt://192.168.45.82:1883"; // Local Mosquitto broker address
const mqttTopic = "sensor/data"; // Topic to subscribe to

// Store the latest sensor data
let latestSensorData = {
  boardId: "N/A",
  temperature: "N/A",
  pressure: "N/A",
};

// List of connected SSE clients
const sseClients = [];

// Connect to the Local MQTT Broker
const mqttClient = mqtt.connect(mqttBroker, {
  clean: true,
  connectTimeout: 4000,
  username: 'MQTT Client',
  reconnectPeriod: 1000,
});

mqttClient.on("connect", () => {
  console.log("Connected to local MQTT Broker");

  // Subscribe to the sensor data topic
  mqttClient.subscribe(mqttTopic, (err) => {
    if (err) {
      console.error("Failed to subscribe:", err);
    } else {
      console.log(`Subscribed to topic: ${mqttTopic}`);
    }
  });

  mqttClient.on("message", (topic, message) => {
    console.log(topic);
    if (topic === mqttTopic) {
      try {
        const data = JSON.parse(message.toString());
        latestSensorData = {
          boardId: data.boardId || "Unknown",
          temperature: data.temperature || "N/A",
          pressure: data.pressure || "N/A",
        };
        console.log("Received data:", latestSensorData);

        // Notify all SSE clients with the updated data
        sseClients.forEach((res) =>
          res.write(`data: ${JSON.stringify(latestSensorData)}\n\n`)
        );
      } catch (err) {
        console.error("Error parsing MQTT message:", err);
      }
    }
  });
});

// Set up Express HTTP server
const app = express();

// Serve the HTML page
app.get("/", (req, res) => {
  const html = `
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>MQTT Sensor Data</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          text-align: center;
          margin-top: 50px;
        }
        #data {
          margin-top: 20px;
          font-size: 1.5rem;
        }
        .sensor-data {
          margin-bottom: 15px;
        }
      </style>
    </head>
    <body>
      <h1>MQTT Sensor Data Viewer</h1>
      <div id="data">
        <div class="sensor-data">
          <strong>Board ID:</strong> <span id="boardId">N/A</span><br>
          <strong>Temperature:</strong> <span id="temperature">N/A</span>°C<br>
          <strong>Pressure:</strong> <span id="pressure">N/A</span> hPa<br>
        </div>
      </div>
      <script>
        const eventSource = new EventSource('/events');
        eventSource.onmessage = (event) => {
          const data = JSON.parse(event.data);
          // Update the HTML elements with new data
          document.getElementById('boardId').textContent = data.boardId;
          document.getElementById('temperature').textContent = data.temperature;
          document.getElementById('pressure').textContent = data.pressure;
        };

        eventSource.onerror = (error) => {
          console.error("SSE connection error:", error);
        };
      </script>
    </body>
    </html>
  `;
  res.send(html);
});

// SSE endpoint to stream data to clients
app.get("/events", (req, res) => {
  res.setHeader("Content-Type", "text/event-stream");
  res.setHeader("Cache-Control", "no-cache");
  res.setHeader("Connection", "keep-alive");

  // Add the client to the list
  sseClients.push(res);

  // Remove the client when the connection is closed
  req.on("close", () => {
    const index = sseClients.indexOf(res);
    if (index !== -1) {
      sseClients.splice(index, 1);
    }
  });
});

// Start the server
const PORT = 3000;
app.listen(PORT, () => {
  console.log(`HTTP server running at http://localhost:${PORT}`);
});
