# Decision Tree Based Machine Learning Framework for Early Risk Prediction in Soldier Health Monitoring Using IoT and LoRa Communication

## Overview

This project presents a real-time Soldier Safety Monitoring and Risk Prediction System that combines IoT sensors, LoRa communication, GPS tracking, and Machine Learning to continuously monitor a soldier's physiological condition, movement, environmental parameters, and location.

The system collects vital health and motion data using multiple sensors connected to an ESP32-based transmitter node. The data is transmitted wirelessly through LoRa technology to a receiver node and visualized on a Flask-based web dashboard. A Decision Tree Machine Learning model predicts the soldier's health state and provides early warnings for potential risks.

---

## Features

* Real-time Heart Rate (BPM) monitoring using MAX30102
* Blood Oxygen (SpO₂) monitoring using MAX30102
* Temperature and Humidity monitoring DHT22
* Motion and activity tracking using MPU6050
* Live GPS location tracking
* Long-range LoRa communication
* Flask-based monitoring dashboard
* Interactive location map visualization
* Machine Learning-based health state prediction
* Real-time risk alerts and classification

---

## Predicted Health States

* Normal
* Fatigue
* Panic_Risk
* Injury_Risk
* Heat_Risk
* Cold_Risk
* Severe_Cold_Risk
* Heat_Stroke
* Data_Inconsistency

---

## Hardware Components

* ESP32 WROOM-32 Development Board (Transmitter)
* ESP32 WROOM-32 Development Board (Receiver)
* MAX30102 Pulse Oximeter Sensor
* MPU6050 Accelerometer and Gyroscope
* DHT22 Temperature and Humidity Sensor
* GPS NEO-6M Module
* LoRa RA-02 (433 MHz) Module
* Breadboard and Connecting Wires
* Power Bank / USB Power Supply

---

## Software and Technologies

* Arduino IDE
* Python (in VS Code)
* Flask
* HTML
* CSS
* JavaScript
* Decision Tree Machine Learning
* Leaflet Maps
* LoRa Library
* TinyGPS++

---

## Project Architecture

Sensor Layer
→ ESP32 Transmitter
→ LoRa Transmitter
→ LoRa Receiver
→ ESP32 Receiver
→ Python Flask Application
→ Machine Learning Prediction
→ Web Dashboard

---

## Machine Learning Model

Algorithm Used:

* Decision Tree Classifier

Input Features:

* BPM
* SpO₂
* Temperature
* Humidity
* Motion(AX)
* Motion(AY)
* Motion(AZ)

Output:

* Predicted Soldier's Health Status

---

## Dashboard Features

* Live sensor monitoring
* Health status prediction
* Real-time GPS location tracking
* Interactive map view
* Historical sensor graphs
* Color-coded risk alerts

---

## Authors

### Project Team

* M A Seshasaai
* K Bala Manikanta
* Sai Janavi P H
* Lakshana R
* Kamaleshwaran M

### Faculty Mentor

* Dr. Vijayalakshmi S

### Affiliation

Department of Electronics and Communication Engineering

### College

R.M.K. Engineering College

---

**Project Title:**
ML-Based Smart Soldier Safety Monitoring and Risk Prediction System

**Project Type:**
IEEE Research Project

**Academic Year:**
2025–2026

---

## Demo Videos

Google Drive Link:
Data_samples_demo_1.mp4 : https://drive.google.com/file/d/1WrTiTX44Egabc7yUeexPw88KjaGeTVl_/view?usp=drive_link
Data_samples_demo_2.mp4 : https://drive.google.com/file/d/1PPMwjo2mYmFnjx0auXtJb-slaMJXoDDK/view?usp=drive_link
Data_samples_demo_3.mp4 : https://drive.google.com/file/d/1ptXvJlerkpQGO30RxPR4ScSQaOC-MSjj/view?usp=drive_link
Real_time_data_ML_prediction.mp4 : https://drive.google.com/file/d/1IRUuj4BIjzKpXsUBT5HspeIYI4qy4p33/view?usp=drive_link
Real_time_ML_prediction_via_web_dashboard.mp4 : https://drive.google.com/file/d/1Unu3BH-jNns_KMHXtIqW-LuH0GTr507T/view?usp=drive_link

---

## Future Enhancements

* Low Power Consumption using Li-ion battery
* Data Encryption between TX LoRa and RX LoRa
* Cloud database integration
* SMS and emergency alert system
* Multi-soldier monitoring capability
* Deep Learning-based prediction models
* Edge AI deployment on embedded devices
