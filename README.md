# Temperature-Controlled Fan & Heater System using 8051

An embedded systems project featuring an automated climate control system built using the **8051 Microcontroller (AT89C51/AT89S52)**. The system dynamically reads real-time temperature data from an analog sensor (**LM35**) via an **ADC0804**, displays the readings and device statuses on a **16x2 LCD**, and intelligently triggers a multi-stage cooling fan or heater based on defined temperature zones.



---

##  Features

*   **Automated Climate Regulation:** Implements full automation across 5 distinct temperature bands (Heating, Normal, and Cooling states).
*   **Multi-Stage Speed & Heat Intensity:** 
    *   3-Level simulated Pulse Width Modulation (PWM) duty cycles for progressive Fan Speed control.
    *   2-Level staged heating element configuration.
*   **Real-Time Visual Interface:** Clean data monitoring layout on a 16x2 LCD showing temperature (`TEMPR: XX C`) and current system actions (`FANON`, `HTRON`, `NORM.`).
*   **Optimized Resource Management:** Developed completely in Keil C51 with state-driven polling for reliable loop performance.

---

##  Hardware Specifications & Pin Configuration

| Component / Interface | Hardware Pin / Port | Purpose |
| :--- | :--- | :--- |
| **ADC0804 Data Port** | `P3` (P3.0 - P3.7) | Captures 8-bit digital temperature readings from LM35 |
| **LCD Data Port** | `P1` (P1.0 - P1.7) | Sends commands and ASCII characters to the display |
| **ADC Controls** | `P2.4` (WR), `P2.5` (RD), `P2.3` (INTR) | Directs conversion cycles & timing flags |
| **LCD Controls** | `P2.0` (RS), `P2.1` (RW), `P2.2` (EN) | Manages display data registry/enable strobes |
| **Fan Motor Drive** | `P2.6` (MTR) | Actuates fan speed control circuit |
| **Heater Elements** | `P0.0` (HTR1), `P0.1` (HTR2) | Switches independent thermal coils |

---

---

---

## System Logic & Threshold Zones

The core control unit monitors incoming analog-to-digital (ADC) values and segments system responses into specific temperature boundaries:

| Temperature Range | System State | Hardware Action |
| :--- | :--- | :--- |
| **$< 10^{\circ}\text{C}$** | Critical Heating (Max) | Heaters 1 & 2 **ON** (`0`), Fan **OFF** (`1`) |
| **$10^{\circ}\text{C}$ to $19^{\circ}\text{C}$** | Mild Heating (Low) | Heater 1 **ON** (`0`), Heater 2 & Fan **OFF** (`1`) |
| **$20^{\circ}\text{C}$ to $24^{\circ}\text{C}$** | Room Equilibrium | Heaters 1 & 2 **OFF** (`1`), Fan **OFF** (`1`) |
| **$25^{\circ}\text{C}$ to $29^{\circ}\text{C}$** | Cooling Level 1 | Heaters **OFF** (`1`), Fan **PWM 50% Duty Cycle** |
| **$30^{\circ}\text{C}$ to $34^{\circ}\text{C}$** | Cooling Level 2 | Heaters **OFF** (`1`), Fan **PWM 75% Duty Cycle** |
| **$\ge 35^{\circ}\text{C}$** | Critical Cooling (Max) | Heaters **OFF** (`1`), Fan **ON Max Continuous** (`0`) |

> ⚠️ **Note on Logic Polarity:** The heater pins (`HTR1`, `HTR2`) and the fan driver pin (`MTR`) operate on **Active-Low** logic based on the code design. Writing a `0` turns the component ON, while writing a `1` turns it OFF.

---

  ### Detailed Breakdown
*   **Critical Heating Zone ($< 10^{\circ}\text{C}$):** Both Heater Stage 1 and Stage 2 are enabled (`HTR1 = 0`, `HTR2 = 0` active low) to reach ambient levels swiftly. LCD prints `HEAT:LEVEL 2`.
*   **Mild Heating Zone ($10^{\circ}\text{C}$ to $19^{\circ}\text{C}$):** Low heat state where only Heater Stage 1 operates. LCD prints `HEAT:LEVEL 1`.
*   **Normal Room Zone ($20^{\circ}\text{C}$ to $24^{\circ}\text{C}$):** Equilibrium state. Both heaters and the fan are kept **OFF**. LCD prints `NORM.` on line 1 and `FAN,HTR OFF` on line 2.
*   **Cooling Zone 1 ($25^{\circ}\text{C}$ to $29^{\circ}\text{C}$):** Low Fan Speed activated using a 50% duty cycle delay profile. LCD prints `SPEED:LEVEL 1`.
*   **Cooling Zone 2 ($30^{\circ}\text{C}$ to $34^{\circ}\text{C}$):** Medium Fan Speed activated using an optimized 75% duty cycle pulse profile. LCD prints `SPEED:LEVEL 2`.
*   **Critical Cooling Zone ($\ge 35^{\circ}\text{C}$):** Maximum Fan Speed mode (`MTR = 0` continuous low draw). LCD prints `SPEED:LEVEL 3`.

---

## 💻 Software Architecture

The source code is modularized into distinct blocks for clean readability and maintenance:
*   `lcdInit()` / `lcdCmd()` / `lcdData()`: Handles the low-level 8-bit handshake sequence and clears busy flags safely before printing text.
*   `adcRead()`: Pulses the `WR` line to initiate conversion, polls the active-low interrupt pin (`INTR`), and captures raw data through port 3 (`P3`).
*   `convert()` / `update()`: Parses raw bytes mathematically, scales the value into single and tens digits, converts them to printable ASCII hex string masks (`0x30`), and maps them directly to LCD coordinates.
*   `motorcontrol()`: The centralized infinite execution loop (`while(1)`) running state machines that adjust peripheral pin outputs synchronously with ADC fluctuations.

---

## ⚙️ How to Compile & Flash

### Prerequisites
*   **IDE:** Keil uVision (with C51 compiler options).
*   **Flashing Utility / Simulation:** ISP Programmer (e.g., Willar or PROISP) if using a physical chip, or **Proteus VSM** for schematic simulation.

### Execution Steps
1. Create a new project in Keil uVision targeting the **AT89C51** or **AT89S52** device database.
2. Add the `code.c` file to your target Source Group.
3. Open Project Options $\rightarrow$ **Output** tab $\rightarrow$ Check **"Create HEX File"**.
4. Click **Build Target (F7)** to compile the code and generate the standard production `.hex` binary string file.
5. Upload the compiled `.hex` file directly to your microcontroller flash memory or point to it in your Proteus schematic design for simulation.
