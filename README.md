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

# **Example Applications**
- **Mine Safety**: Monitor gas levels to ensure worker safety.
- **Cave Research**: Track environmental changes over time for scientific studies.
- **Industrial Monitoring**: Deploy in tunnels or underground facilities for real-time condition tracking.
