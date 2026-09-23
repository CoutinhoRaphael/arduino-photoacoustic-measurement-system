# Automated Measurement System with Arduino

Embedded system developed with **Arduino** to automate laser positioning across a measurement grid, using stepper motors, limit switches, and acoustic signal acquisition through a microphone.

This project was developed as part of a **Technology Initiation Program**, involving automation, motion control, embedded programming, and signal acquisition.

## 🔬 About the Project

The system uses two stepper motors to control the movement of a laser along the **X and Y axes**, allowing it to scan different positions across a measurement grid.

During the scanning process, a microphone continuously captures acoustic signals. The readings are processed using a moving average and a configurable threshold to detect potential impacts or relevant acoustic events.

The system is controlled through a **finite state machine**, which coordinates the initial positioning, axis movement, laser activation, and measurement sequence.

## ⚙️ Features

* Control of two stepper motors on the X and Y axes;
* Automatic homing using limit switches;
* Automated movement across predefined measurement positions;
* Timed laser activation;
* Continuous microphone signal acquisition;
* Signal smoothing through sample averaging;
* Threshold-based event detection;
* Finite state machine for system control;
* Real-time monitoring through the Serial Monitor.

## 🧰 Technologies & Components

### Software

* Arduino IDE
* C/C++
* [AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper/)

### Hardware

* Arduino Uno
* 2× Stepper Motors
* Stepper Motor Drivers
* Microphone Module
* Laser Module
* Limit Switches
* Breadboard and supporting components

## 📐 System Workflow

The system follows an automated scanning cycle:

```text
HOMING
   ↓
Move along the X axis
   ↓
Activate the laser at predefined positions
   ↓
Move along the Y axis
   ↓
Move along the X axis in the opposite direction
   ↓
Move along the Y axis
   ↓
Repeat the scanning process
```

After completing the configured number of rows, the system returns to the **homing** state.

## 🎙️ Acoustic Signal Acquisition

The microphone is connected to the Arduino's `A1` analog input.

To reduce fluctuations in the readings, the system calculates the average of **10 samples**:

```cpp
const int NUM_SAMPLES = 10;
```

Event detection is performed using a configurable threshold:

```cpp
const int MIC_THRESHOLD = 600;
```

When the averaged microphone reading exceeds this threshold, the system registers an event and reports it through the Serial Monitor.

## 🔌 Pin Configuration

| Component    | Arduino Pin |
| ------------ | ----------: |
| Microphone   |          A1 |
| X-Axis STEP  |           2 |
| X-Axis DIR   |           5 |
| Y-Axis STEP  |           3 |
| Y-Axis DIR   |           6 |
| Limit Switch |           9 |
| Limit Switch |          10 |
| Laser        |          11 |

> Pin assignments and system parameters can be modified directly in the source code according to the hardware configuration.

## 🚀 Getting Started

### Requirements

* Arduino IDE
* Arduino Uno
* [AccelStepper library](https://www.airspayce.com/mikem/arduino/AccelStepper/)
* Required hardware components

### Installation

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Install the **AccelStepper** library.
3. Connect the Arduino to your computer.
4. Assemble the hardware according to the pin configuration.
5. Open the `.ino` file in the Arduino IDE.
6. Adjust the speed, movement, and microphone sensitivity parameters if necessary.
7. Compile and upload the program to the Arduino.
8. Open the Serial Monitor at **115200 baud** to monitor the system.

## 📁 Project Structure

```text
├── src/
│   └── measurement_system.ino
├── docs/
│   └── images/
└── README.md
```

## 🎯 Project Goal

The project aims to contribute to the **automation of an experimental measurement system**, integrating motion control, laser activation, and acoustic signal acquisition into a single embedded platform.

## 👨‍💻 Development

Developed as part of a **Technology Initiation Program**, involving embedded programming, automation, stepper motor control, and acoustic signal acquisition.

---

⭐ Developed for research and technological development purposes.
