# Smart Energy Optimization System (ESP32 + ThingsBoard)

---

## 📌 Project Overview
This project is a Smart Energy Optimization System built using ESP32.
It automatically controls Light, Fan, and AC based on:

🕵️ Motion Detection (PIR Sensor)

🌞 Light Intensity (LDR)

🌡 Temperature (DHT22)

The system sends real-time telemetry data to ThingsBoard Cloud using MQTT for monitoring energy consumption and cost analysis.

---

## ⚙️ Hardware Used

ESP32

PIR Motion Sensor

LDR (Photoresistor)

DHT22 Temperature Sensor

3 Relay Modules (Light, Fan, AC)

---

## 🧠 Logic Implemented

**If Motion Detected:**

🌞 Bright Condition:

Temp > 41°C → AC ON

Temp > 37°C → Fan ON

Light OFF

🌑 Dark Condition:

Light ON

Temp > 41°C → AC ON

Temp > 37°C → Fan ON

❌ If No Motion:

Light OFF

Fan OFF

AC OFF

---

## ⚡ Energy & Cost Calculation

**The system calculates:**

Individual device running time

Total energy consumption (kWh)


Estimated electricity cost

All values are sent to ThingsBoard via MQTT.


---

## ☁️ Cloud Platform

MQTT Server: demo.thingsboard.io

**Dashboard includes:**

Temperature Graph

Energy Consumption (kWh)

Cost Graph

Device Status Indicators

---

## 🔥 Features
✔ Smart automation

✔ Energy optimization

✔ Cost estimation

✔ Cloud monitoring

✔ Real-time data visualization

---

## 🧑‍💻 Simulation

Project tested on Wokwi Simulator

Click here to open 
https://wokwi.com/projects/456027893800274945

