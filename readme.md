# Baja SAE Central Data Acquisition (DAQ) Node

A custom, ruggedized telemetry and data logging PCB engineered for the University of Waterloo Baja SAE racing vehicle. 

Unlike standard microcontroller breakout boards, this DAQ node is designed to survive the harsh electrical and mechanical environment of an automotive chassis. It features a 4-layer load-dump protected layout, dedicated hardware for high-speed logging, and a real-time FreeRTOS firmware architecture.

<img width="2520" height="1208" alt="baja_daq" src="https://github.com/user-attachments/assets/908fbbb0-38d3-4c7e-af37-8a5154cd2bff" />
<img width="2880" height="1800" alt="image" src="https://github.com/user-attachments/assets/94d59bee-c6a2-439d-994c-aac49caae539" />
<img width="932" height="800" alt="bajafirmwarepic" src="https://github.com/user-attachments/assets/9923cc9a-0ab1-46d6-89db-76a6d06c8913" />
<img width="989" height="680" alt="LL9DJzj04BtlhnXn9L0uzD93XS64HbL8iSs7r5JLr1b_DFQkjJDBe5N_UtVN8GcNnBFllT6RDtF1WbPStobffIEunDy8QN87qzRs6xG49p2rH5cno67nJFQvfOrIl6xrW1PxA4q_64rQSddrUakbeAuxEg1mWokpQNKD5NPCIWrOhh4cECf9ix7m0HweQKild-d12H_1NmNmFKcl89qdIvq" src="https://github.com/user-attachments/assets/b59d729f-b1ed-4d84-b2ab-2bfd6053cc74" />









## ⚙️ Hardware Architecture
* **Microcontroller:** STM32F405RGTx (ARM Cortex-M4) handling real-time sensor polling and SDIO data pipelines.
* **PCB Stackup:** 4-Layer board (Signal, Solid Ground, Split Power, Signal) optimized for signal integrity, return paths, and thermal dissipation.

## 🔋 Automotive-Grade Power System
The board interfaces directly with the vehicle's 12V starter battery and is hardened against off-road electrical surges:
* **Surge Protection:** Input clamped by an SMAJ15A TVS Diode with localized solid copper pour spillways to instantly shunt 60V load dumps to the ground plane.
* **Analog Rail (5V):** L7805 linear regulator feeding suspension and rotary sensors with a clean, heavily filtered reference voltage.
* **Digital Rail (3.3V):** AMS1117 LDO isolated with dedicated bulk tantalum/ceramic decoupling arrays to prevent STM32 brownouts under severe chassis vibration.

## 📡 Sensor & Communication Interfaces
* **Vehicle Network:** CAN Bus (TJA1051T transceiver) acting as a non-blocking broadcast node for dashboard telemetry, timed via an 8MHz external crystal.
* **Inertial Tracking:** On-board 6-axis IMU (ISM330DHCX) communicating via high-speed SPI.
* **Telemetry Logging:** MicroSD card slot wired via 4-bit SDIO with 47k pull-up stability arrays for reliable 100Hz writes.
* **External I/O (Locking Molex Connectors):** * Front & Rear Suspension Travel (Generic 3-pin ADC)
    * Engine & Secondary CVT RPM (Hardware Timers / Input Capture)
    * Belt Temperature Probes (I2C)

## 💻 Firmware Design (FreeRTOS)
The system runs a custom C firmware utilizing the STM32 HAL and CMSIS-RTOS.
* **Decoupled Logging:** A high-priority sensor task handles SPI/I2C polling and mathematically derives ground speed from timer deltas. Data is formatted and pushed to a FreeRTOS queue.
* **Non-Blocking File I/O:** A lower-priority FatFs SD Writer task pulls from the queue to write to the MicroSD card, ensuring that standard file-system latency does not freeze the 100Hz real-time data gathering loop.
* **Data Protection:** Implements an automated 15-minute file rotation state machine to prevent total file corruption in the event of a sudden power loss or rollover.
