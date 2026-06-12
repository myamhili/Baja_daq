# Baja SAE Central Data Acquisition (DAQ) Node

A custom, ruggedized telemetry and data logging PCB engineered for an off-road Baja SAE racing vehicle. 

Unlike standard microcontroller breakout boards, this DAQ node is designed to survive the harsh electrical and mechanical environment of an automotive chassis. It features a 4-layer load-dump protected layout, dedicated hardware for high-speed logging, and CAN bus communication.

## ⚙️ Core Architecture
* **Microcontroller:** STM32F405RGTx (ARM Cortex-M4) handling real-time sensor polling and SDIO data pipelines.
* **PCB Stackup:** 4-Layer board (Signal, Solid Ground, Split Power, Signal) optimized for signal integrity, return paths, and thermal dissipation.

## 🔋 Automotive-Grade Power System
The board interfaces directly with the vehicle's 12V starter battery and is hardened against off-road electrical surges:
* **Surge Protection:** Input clamped by an SMAJ15A TVS Diode with localized solid copper pour spillways to instantly shunt 60V load dumps to the ground plane.
* **Analog Rail (5V):** L7805 linear regulator feeding suspension and rotary sensors with a clean, heavily filtered reference voltage.
* **Digital Rail (3.3V):** AMS1117 LDO isolated with dedicated bulk tantalum/ceramic decoupling arrays to prevent STM32 brownouts under severe chassis vibration.

## 📡 Sensor & Communication Interfaces
* **Vehicle Network:** Isolated CAN Bus (TJA1051T transceiver) with an 8MHz external crystal for precision timing.
* **Inertial Tracking:** On-board 6-axis IMU (ISM330DHCX) communicating via high-speed SPI.
* **Telemetry Logging:** MicroSD card slot wired via 4-bit SDIO with 47k pull-up stability arrays.
* **External I/O (Locking Molex Connectors):** * Front & Rear Suspension Travel (ADC)
    * Engine & Secondary CVT RPM (Hardware Timers)
    * I2C Temperature Probes

## 🛠️ Viewing the Project
To view the schematic and physical layout, open `baja_daq.kicad_pro` using KiCad 10.0 or newer.