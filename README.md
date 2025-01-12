# PR Project - ESP CaveNow

### Rares Constantin - 343C1

# Overview


This project is an IoT-based monitoring system designed for mines and caves. It collects environmental data such as temperature, pressure, and gas concentrations. The system utilizes ESP32 microcontrollers and the ESP-Now protocol to form a dynamic sensor network. A central coordinator node relays data to a cloud platform using the MQTT protocol for real-time monitoring and alerting.

## **Network Topology**

The system is structured as a **tree network**, combining direct and routed communication between nodes:

- **Sensor Nodes**: These nodes measure environmental parameters using sensors (e.g., BMP280 for temperature/pressure and MQ-135 for gases).
  - **Direct Connection**: Nodes can communicate directly with the central coordinator if within range.
  - **Routed Communication**: Nodes outside the coordinator's range send data through intermediate sensor nodes, which route the messages to the coordinator.

- **Coordinator**:
  - Collects all sensor data and uploads it to a cloud platform using MQTT.
  - Features a **blue LED indicator** that remains ON to signify it is active and properly functioning.

This topology ensures flexibility, automatic node integration, and coverage extension in environments with challenging connectivity, such as mines or caves.


## **Hardware Connections**

Below is the pin configuration for connecting the **ESP32** microcontroller to the **BMP280** sensor (temperature/pressure) and the **MQ-135** sensor (gas detection):

### **BMP280 Sensor**
| **BMP280 Pin** | **ESP32 Pin** | **Description**               |
|----------------|---------------|--------------------------------|
| VCC            | 3V3           | Power supply                  |
| GND            | GND           | Ground                        |
| SCL            | GPIO 22       | I2C Clock line                |
| SDA            | GPIO 21       | I2C Data line                 |

### **MQ-135 Sensor**
| **MQ-135 Pin** | **ESP32 Pin** | **Description**               |
|----------------|---------------|--------------------------------|
| VCC            | 5V            | Power supply                  |
| GND            | GND           | Ground                        |
| AOUT           | GPIO 34       | Analog output for gas reading |

## **Features**

1. **Dynamic Sensor Network**
   - Automatic integration of new sensor nodes without prior configuration.
   - Nodes can communicate directly or through other nodes (routed mode).

2. **Environmental Monitoring**
   - Measures **temperature**, **pressure**, and **gas concentrations** using BMP280 and MQ-135 sensors.

3. **Cloud Integration**
   - Data is sent to a cloud platform via the MQTT protocol for real-time visualization and alerting.

4. **Local Alerts**
   - Nodes trigger a local buzzer alarm when environmental thresholds (e.g., high gas concentrations) are exceeded.

5. **Coordinator LED Indicator**
   - A **blue LED** on the coordinator remains ON to confirm its operational status.

# **Getting Started**

### **Requirements**
- **ESP32-WROOM boards** for nodes and coordinator.
- **Sensors**: BMP280 (temperature/pressure), MQ-135 (gas).
- **Power supply**: Battery or USB.

### **Steps to Deploy**
1. **Assemble Sensor Nodes**:
   - Connect the BMP280 and MQ-135 sensors to the ESP32 boards as per the pin configuration above.

2. **Setup the Coordinator**:
   - Connect a blue LED to an available GPIO pin to indicate active status.
   - Configure MQTT credentials and cloud endpoint in the coordinator's firmware.

3. **Deploy and Power Up**:
   - Power on all nodes and the coordinator.
   - Monitor the network as it dynamically integrates all nodes and starts collecting data.

---

## Implementation
### Backend
- The backend uses **Express** to create RESTful APIs for interacting with the data.
- **Mongoose** is used for schema definition and interaction with MongoDB Atlas.
- An **MQTT client** subscribes to sensor data topics and processes incoming messages. The data is saved in MongoDB under the `readings` schema.
- Toggle routes (`/api/alarm` and `/api/disable`) publish MQTT messages to the broker, allowing real-time control of boards.

### ESP32
- The ESP32 Aquisition boards send sensor data over ESP-Now after pairing dynamically with other slaves or with the Coordinator and they control the local alarm, activating it if the sensor values are exceeding the
thresholds.
- The ESP32 Coordinator publishes sensor data (temperature, pressure, air quality) to an MQTT broker over a secure TLS connection.
- JSON format for messages ensures consistency and compatibility.

### React Frontend
- The frontend fetches board data from the backend API and dynamically renders graphs and controls.
- **Chart.js** is used for data visualization, offering line charts for each sensor type.
- Controls include alarm toggling and measurement disabling/enabling.

---

## Visualization and Processing of Data
### Data Flow
1. ESP32 sends sensor data to the MQTT broker.
2. Backend receives and processes the data, storing it in MongoDB.
3. React frontend fetches data via RESTful APIs and visualizes it in real time.

### Graph Features
- Continuous lines represent sensor data.
- Adjustable time ranges (last 5 minutes to 7 days).
- Real-time updates every 30 seconds.

### Table View
- Switch between graph and table views to see raw data entries.

---

## Security
### TLS Implementation
- The ESP32 and backend communicate securely with the MQTT broker using TLS.
- Certificates are required:
  - **CA Certificate:** Verifies the broker's identity.
  - **Client Certificate & Key:** Authenticates the ESP32 and backend to the broker.

### Generating Certificates
1. Create a certificate authority (CA):
   ```bash
   openssl req -new -x509 -days 365 -key ca.key -out ca.crt
   ```
2. Generate a key and certificate signing request (CSR) for the client:
   ```bash
   openssl req -new -key client.key -out client.csr
   ```
3. Sign the CSR with the CA:
   ```bash
   openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365
   ```
4. Use these certificates in the `.env` file for the backend and flash them to the ESP32.

### Other Security Measures
- **Rate Limiting:** Limits email alerts to a maximum of one every 5 minutes.
- **Validation:** Ensures incoming data conforms to the expected structure.
- **Environment Variables:** Securely stores sensitive information like database URIs and TLS paths.

---

# **Example Applications**
- **Mine Safety**: Monitor gas levels to ensure worker safety.
- **Cave Research**: Track environmental changes over time for scientific studies.
- **Industrial Monitoring**: Deploy in tunnels or underground facilities for real-time condition tracking.
