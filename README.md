# # ESP32 Environmental Temperature Monitoring System
A Wifi Environmental monitor that monitors temperature and displays it on the LCD Module.
# ESP32 Environmental Temperature Monitoring System

## Overview

This project is an ESP32-based embedded system that continuously monitors ambient temperature and provides real-time visual, physical, and audible feedback. It combines multiple hardware components into one integrated system to duinisplay temperature information, alert users when temperatures exceed a configurable threshold, and provide intuitive status indicators.

This project was built to strengthen my understanding of embedded systems, C++ programming, sensors, actuators, and hardware integration using the Arduino IDE platform.

---

## Features

- Real-time temperature monitoring using a thermistor
- LCD displays:
  - Current temperature
  - User-adjustable alert temperature
  - Highest recorded temperature
  - Lowest recorded temperature
  - Wi-Fi connection status
- Adjustable temperature alert threshold using a potentiometer
- Continuous audible alarm when the measured temperature exceeds the alert threshold
- RGB LED temperature indicator:
  - Blue = Cold
  - Green = Comfortable
  - Orange = Warm
  - Red = Hot
- Servo motor that physically indicates temperature:
  - Rotates toward the cold side when temperatures are low
  - Centers at moderate temperatures
  - Rotates toward the hot side as temperatures increase
- Automatically records the highest and lowest temperatures observed since startup

---

## Hardware Used

- ESP32 Arduino UNO Style Freenove Board (Most Arduino Uno and ESP32 Boards work.)
- Thermistor
- LCD Display
- RGB LED
- Servo Motor
- Piezo Buzzer
- Potentiometer
- Breadboard
- Jumper Wires

---

## Software

- Arduino IDE
- C++
- ESP32 Installable Libraries

---

## How It Works

The ESP32 continuously reads analog temperature data from the thermistor. The current temperature is displayed on the LCD together with the configurable alert temperature, the highest recorded temperature, the lowest recorded temperature, and the Wi-Fi connection status.

The alert temperature can be adjusted in real time using the potentiometer. When the measured temperature rises above the selected threshold, the buzzer sounds continuously until the temperature falls below the limit.

The RGB LED changes color to visually represent the temperature range, while the servo motor rotates to provide a physical indication of the temperature, pointing toward colder or hotter conditions.

---

## What I Learned

- Reading analog sensors with the ESP32
- Programming in C++
- Controlling multiple hardware peripherals simultaneously
- Integrating displays, sensors, actuators, and LEDs into one embedded system
- Debugging both hardware wiring and software logic
- Designing an embedded system that communicates information through multiple forms of feedback

---

## Future Improvements

- Build a custom enclosure
- Add remote temperature monitoring through a web dashboard
- Store historical temperature data
- Add configurable alert profiles
- Improve the user interface
