# Temperature-Controlled Fan & Heater System using 8051

An embedded systems project featuring an automated climate control system built using the **8051 Microcontroller (AT89C51/AT89S52)**. The system dynamically reads real-time temperature data from an analog sensor (**LM35**) via an **ADC0804**, displays the readings and device statuses on a **16x2 LCD**, and intelligently triggers a multi-stage cooling fan or heater based on defined temperature zones.

This project was developed as part of the Micro Project requirements for the Computer Architecture and Microcontrollers course at **Mar Athanasius College of Engineering, Kothamangalam**.

---

## 🚀 Features

*   **Automated Climate Regulation:** Implements full automation across 5 distinct temperature bands (Heating, Normal, and Cooling states).
*   **Multi-Stage Speed & Heat Intensity:** 
    *   3-Level simulated Pulse Width Modulation (PWM) duty cycles for progressive Fan Speed control.
    *   2-Level staged heating element configuration.
*   **Real-Time Visual Interface:** Clean data monitoring layout on a 16x2 LCD showing temperature (`TEMPR: XX C`) and current system actions (`FANON`, `HTRON`, `NORM.`).
*   **Optimized Resource Management:** Developed completely in Keil C51 with state-driven polling for reliable loop performance.

---

## 🛠️ Hardware Specifications & Pin Configuration

| Component / Interface | Hardware Pin / Port | Purpose |
| :--- | :--- | :--- |
| **ADC0804 Data Port** | `P3` (P3.0 - P3.7) | Captures 8-bit digital temperature readings from LM35 |
| **LCD Data Port** | `P1` (P1.0 - P1.7) | Sends commands and ASCII characters to the display |
| **ADC Controls** | `P2.4` (WR), `P2.5` (RD), `P2.3` (INTR) | Directs conversion cycles & timing flags |
| **LCD Controls** | `P2.0` (RS), `P2.1` (RW), `P2.2` (EN) | Manages display data registry/enable strobes |
| **Fan Motor Drive** | `P2.6` (MTR) | Actuates fan speed control circuit |
| **Heater Elements** | `P0.0` (HTR1), `P0.1` (HTR2) | Switches independent thermal coils |

---

## 📊 System Logic & Threshold Zones

The core control unit monitors incoming ADC variables and strictly segments behavior into the following thresholds:
