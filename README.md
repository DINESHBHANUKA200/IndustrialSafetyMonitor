# IoT-based Industrial Safety Monitor System

## Overview
This project is an **IoT-based Industrial Safety Monitor System** designed to track various environmental and safety parameters in an industrial setting. It utilizes ESP32 along with various sensors to monitor air quality, temperature, vibration, current, and voltage levels, providing real-time data to the user via an OLED display and enabling remote monitoring via ThingSpeak.

## Features
- **Air Quality Monitoring**: MQ-135 and MQ-4 sensors for detecting gases.
- **Temperature and Humidity**: DHT-11 sensor.
- **Current and Voltage Monitoring**: ACS712 current sensor and a voltage sensor.
- **Vibration Monitoring**: Vibration sensor for detecting mechanical issues.
- **Real-time Data Visualization**: Display data on a 128x64 OLED screen and send data to ThingSpeak.
  
## Components
- **ESP32**: Main microcontroller.
- **MQ-135**: Air quality sensor (for detecting gases like CO2, NH3, etc.).
- **MQ-4**: Methane and natural gas detection sensor.
- **DHT-11**: Temperature and humidity sensor.
- **LM35**: Temperature sensor.
- **ACS712 30A**: Current sensor.
- **Voltage Sensor**: Measures voltage (up to 25V).
- **Vibration Sensor**: Detects vibrations in machinery.
- **128x64 OLED**: Displays system data.

## Circuit Diagram
[Circuit Diagram]

## Setup Instructions

### Hardware Setup
1. **Connect the components as follows:**
   - **MQ-135**: Connect to analog input of ESP32.
   - **MQ-4**: Connect to analog input of ESP32.
   - **DHT-11**: Connect to a digital input pin of ESP32.
   - **LM35**: Connect to an analog input pin of ESP32.
   - **ACS712**: Connect to analog input of ESP32.
   - **Voltage Sensor**: Connect to analog input of ESP32.
   - **Vibration Sensor**: Connect to a digital input pin of ESP32.
   - **OLED Display**: Connect to I2C pins (SDA and SCL) of ESP32.

### Software Setup
1. Clone the repository:
   ```bash
   git clone https://github.com/DINESHBHANUKA200/IndustrialSafetyMonitor.git

