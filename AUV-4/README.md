# AUV 4.0 Electrical Firmware

This folder contains the firmware used in BBAS AUV 4.0 electrical system.

## Getting Started

If you are new to this project then take a look at this folder for guide on how to run a copy of the project on your local machine.

## Directory

### Firmware Directory
This project contains the following firmware:
1. Actuator
 - Firmware of the manipulator arm.
2. CAN (Controlled Area Network)
 - General overview of the CAN protocol
3. PMB (Power Monitoring Board)
 - Monitor battery status 
4. SBC-CAN
 - Serial communication with SBC
 - Translate SBC message to CAN format
5. STB (Sensor & Telemetry Board)
 - Internal Sensors & Screen Display
6. TAB (Thruster & Actuation Board)
 - Thruster control
 - High level actuation controls (TBA)

### Other Directory
1. Getting Started
 - For plebs that is new to the project.
2. Scripts
 - For ~~lazy~~ people who wants the machine to do their work.

## Built With

* [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) - Main IDE  used for STM32 development
* [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html/) - Graphical tool for STM32 pinout configuration

## Acknowledgement

* **Chua Chong Yu** - *Integration & Power Distribution*
* **Ng Yong Jie** - *Acoustics & Navigation*
* **Kang QingXuan** - *SBC-CAN & PMB*
* **Yang YiHang** - *STB*
* **Joshua Nathanael** - *PCB*
* **Ben Chong** - *TAB*
* **Nathania Santoso** - *PMB*