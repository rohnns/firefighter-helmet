# Firefighter Project — Bare Metal STM32F4xx

A bare metal C firmware for an STM32F4xx microcontroller that monitors carbon monoxide levels and ambient temperature, sounds a buzzer on hazardous readings, and streams live GPS coordinates — all without HAL or any vendor middleware.

---

## Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Pin Map](#pin-map)
- [System Architecture](#system-architecture)
- [Clock Configuration](#clock-configuration)
- [Peripheral Details](#peripheral-details)
  - [ADC — Gas & Temperature Sensing](#adc--gas--temperature-sensing)
  - [USART1 — GPS Module](#usart1--gps-module)
  - [USART2 — Debug Output](#usart2--debug-output)
  - [GPIO — Buzzer](#gpio--buzzer)
  - [SysTick — Millisecond Timebase](#systick--millisecond-timebase)
- [Serial Output Format](#serial-output-format)
- [Buzzer Trigger Thresholds](#buzzer-trigger-thresholds)
- [Building](#building)
- [Flashing](#flashing)
- [Porting to Other STM32F4 Variants](#porting-to-other-stm32f4-variants)
- [Known Limitations](#known-limitations)

---

## Hardware Requirements

| Component | Part |
|-----------|------|
| Microcontroller | STM32F4xx (tested target: F401/F411/F446) |
| Gas sensor | MQ-7 (carbon monoxide) |
| Temperature sensor | LM35 (analog, 10 mV/°C) |
| GPS module | Any NMEA-0183 UART GPS (e.g. Neo-6M, Neo-8M) |
| Buzzer | Active buzzer, active HIGH |
| Supply voltage | 3.3 V (VDDA = 3.3 V assumed for ADC reference) |

---

## Pin Map

| Pin | Direction | Function |
|-----|-----------|----------|
| PA0 | Analog in | MQ-7 gas sensor output → ADC1 CH0 |
| PA1 | Analog in | LM35 temperature output → ADC1 CH1 |
| PA2 | AF7 (TX) | USART2 debug TX |
| PA3 | AF7 (RX) | USART2 debug RX |
| PA5 | Output | Buzzer (HIGH = ON) |
| PA9 | AF7 (TX) | USART1 GPS TX (optional, not used for receive) |
| PA10 | AF7 (RX) | USART1 GPS RX |

---

## System Architecture

```
┌──────────────┐     ADC CH0     ┌──────────────────────────┐
│   MQ-7 Gas   │ ──────────────► │                          │
└──────────────┘                 │       STM32F4xx          │
                                 │                          │   USART2 (PA2)
┌──────────────┐     ADC CH1     │  Read_Sensors()          │ ──────────────► Debug Terminal
│    LM35      │ ──────────────► │  Update_Buzzer()         │
└──────────────┘                 │  Read_GPS()              │
                                 │                          │
┌──────────────┐  USART1 (PA10)  │                          │
│  GPS Module  │ ──────────────► │                          │
└──────────────┘                 └──────────────────────────┘
                                          │ PA5
                                          ▼
                                      [ Buzzer ]
```

The main loop runs every 500 ms:

1. **Read_Sensors** — samples ADC CH0 (MQ-7) and CH1 (LM35), converts raw counts to voltage and °C.
2. **Update_Buzzer** — drives PA5 HIGH if either threshold is exceeded.
3. **Read_GPS** — drains any available NMEA bytes from USART1 RX, parses `$GPGGA` sentences into decimal-degree latitude/longitude.
4. **printf** — prints a consolidated status line over USART2.

---

## Clock Configuration

The PLL is configured entirely through RCC registers using the internal 16 MHz HSI oscillator as the source:

| Parameter | Value |
|-----------|-------|
| PLL source | HSI (16 MHz) |
| PLLM | 16 → VCO input = 1 MHz |
| PLLN | 336 → VCO output = 336 MHz |
| PLLP | /4 → **SYSCLK = 84 MHz** |
| PLLQ | /7 → USB/SDIO = 48 MHz |
| AHB prescaler | /1 → HCLK = 84 MHz |
| APB1 prescaler | /2 → PCLK1 = 42 MHz |
| APB2 prescaler | /1 → PCLK2 = 84 MHz |
| Flash latency | 2 wait states |

To change the target frequency, adjust `PLLM`, `PLLN`, `PLLP` in `SystemClock_Config()` and update `SYSCLK_HZ`, `APB1_HZ`, `APB2_HZ` at the top of `main.c`.

---

## Peripheral Details

### ADC — Gas & Temperature Sensing

- **Resolution:** 12-bit (0–4095)
- **Trigger:** Software (`SWSTART`)
- **Sample time:** 84 cycles per channel (set in `SMPR2`)
- **Clock:** PCLK2/4 = 21 MHz (within the 36 MHz ADC max)
- **Conversion formula:**

```
mq7_voltage = (raw / 4095.0) × 3.3          [Volts]
lm35_temp   = (raw / 4095.0) × 3.3 / 0.01   [°C]
```

Channels are switched by rewriting `ADC1->SQR3` between reads; no DMA or scan mode is used.

### USART1 — GPS Module

- **Baud rate:** 115200, 8N1
- **Bus clock:** PCLK2 (84 MHz) → `BRR = 84000000 / 115200`
- **Operation:** Polling with a per-byte 10 ms timeout (`usart1_getchar_timeout`)
- **Parser:** Accumulates characters into `gps_buffer`; on `\n`, checks for `$GPGGA` and tokenises with `strtok` to extract fields 3–6 (lat, N/S, lon, E/W)
- **NMEA parsing:** DDMM.MMMM → decimal degrees via `parse_latitude` / `parse_longitude`

### USART2 — Debug Output

- **Baud rate:** 115200, 8N1
- **Bus clock:** PCLK1 (42 MHz) → `BRR = 42000000 / 115200`
- **Operation:** Blocking TX via `TXE` flag polling
- **`printf` redirect:** The newlib `_write` syscall is overridden to send bytes through `usart2_putchar`, so all `printf` calls go straight to PA2.

### GPIO — Buzzer

Buzzer state is toggled using the **BSRR** register for atomic, glitch-free writes:

```c
GPIOA->BSRR = (1U << 5);        // PA5 HIGH — buzzer ON
GPIOA->BSRR = (1U << (5 + 16)); // PA5 LOW  — buzzer OFF
```

### SysTick — Millisecond Timebase

SysTick is loaded with `(84000000 / 1000) - 1 = 83999` and counts down at the processor clock rate. `SysTick_Handler` increments the global `g_tick_ms` counter. Both `delay_ms()` and `usart1_getchar_timeout()` use this counter for non-blocking elapsed-time checks.

---

## Serial Output Format

Connect a serial terminal (115200 8N1) to **PA2 (USART2 TX)**. Typical output:

```
Firefighter Project Started
MQ7 Voltage: 0.85 V | LM35 Temp: 27.43 C
Buzzer: OFF
GPS: Latitude 12.971599 | Longitude 77.594566
Gas: 0.85V | Temp: 27.43C | Lat: 12.971599 | Lon: 77.594566
```

---

## Buzzer Trigger Thresholds

| Condition | Threshold | Meaning |
|-----------|-----------|---------|
| `mq7_voltage > 1.2 V` | ~1490 ADC counts | Elevated CO level |
| `lm35_temp > 10.0 °C` | Baseline sanity check — **raise this** for real use | High temperature |

> **Note:** The `lm35_temp > 10.0°C` threshold is from the original code and will trigger the buzzer at room temperature. Adjust it to a realistic fire-detection value (e.g. `> 60.0f`) before deploying.

---

## Building

The project is a single translation unit. With an ARM GCC toolchain:

```bash
arm-none-eabi-gcc \
  -mcpu=cortex-m4 \
  -mfpu=fpv4-sp-d16 \
  -mfloat-abi=hard \
  -mthumb \
  -O2 \
  -Wall -Wextra \
  -T linker_script.ld \
  -nostartfiles \
  -lm \
  -o firefighter.elf \
  startup_stm32f4xx.s \
  main.c
```

You will need:
- A startup file (`startup_stm32f4xx.s`) that sets up the vector table and calls `main`.
- A linker script (`linker_script.ld`) appropriate for your specific STM32F4 variant's flash and RAM sizes.

Both are available from ST's CMSIS device pack or can be generated by STM32CubeIDE (then stripped of HAL content).

---

## Flashing

```bash
# Using OpenOCD with an ST-Link
openocd -f interface/stlink.cfg \
        -f target/stm32f4x.cfg \
        -c "program firefighter.elf verify reset exit"
```

Or drag-and-drop the `.bin` onto the virtual USB drive if your Nucleo/Discovery board supports it.

---

## Porting to Other STM32F4 Variants

The code targets the STM32F4xx memory map, which is consistent across F401, F411, F446, and most other F4 devices. Things to adjust per variant:

| What | Where | Notes |
|------|-------|-------|
| PLL constants | `SystemClock_Config()` | Different max frequencies per device |
| `SYSCLK_HZ` / `APB1_HZ` / `APB2_HZ` | top of `main.c` | Must match PLL config |
| Flash wait states | `SystemClock_Config()` | 2WS is safe up to 90 MHz on F4 |
| Linker script | external | Flash/RAM sizes vary per part |
| Startup file | external | Vector table count varies |

The peripheral base addresses (`GPIOA`, `ADC1`, `USART1`, `USART2`, `RCC`) are identical across all STM32F4xx parts.
