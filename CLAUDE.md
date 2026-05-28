# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Five-channel infrared line-tracking robot car on CH32V307VCT6 (RISC-V, 120MHz), built with the Seekfree (逐飞科技) open-source library for the 17th National University Student Intelligent Car Competition.

## Build & Flash

This is a MounRiver Studio (Eclipse-based) project. There is no CLI build system — all building is done through the IDE.

- **Build:** Import `project/mrs/` as existing project in MounRiver Studio, then `Project → Build Project`
- **Flash:** WCH-Link debugger via `Run → Debug`, or flash `project/mrs/obj/*.hex` directly with WCH-LinkUtility
- **Compiler:** GNU MCU RISC-V GCC (`riscv-none-embed-gcc`), target rv32imafc with ilp32f ABI

## Architecture

Two layers with strict separation — do not modify `libraries/`:

```
libraries/          ← Third-party Seekfree library + WCH official SDK (read-only)
  sdk/              ← CH32V307 register-level SDK (Peripheral/, Core/, Startup/, Ld/)
  zf_common/        ← Clock init, debug UART, FIFO, type definitions
  zf_driver/        ← HAL wrappers: gpio, pwm, uart, timer, dvp, exti, delay, soft_iic
  zf_device/        ← Higher-level device drivers: camera, wireless, gnss, dl1a, dm1xa

project/            ← User code lives here
  user/src/main.c   ← Entry point: all init + main control loop
  user/src/isr.c    ← All interrupt handlers (UART, TIM, EXTI, DVP)
  user/inc/         ← User headers
  code/             ← Drop-in folder for additional user modules
  mrs/              ← IDE project config, linker settings, build output
```

### Key Abstractions (zf_driver layer)

All peripheral access goes through the zf_driver wrappers, not raw register manipulation:
- `gpio_init(pin, dir, default_val, mode)` — pin enum like `A5`, `C8`, `B1`
- `pwm_init(channel, freq, duty)` — channel enum encodes timer+mapping+pin, e.g. `TIM5_PWM_MAP0_CH2_A1`
- `pwm_set_duty(channel, duty)` — duty range 0-10000 (`PWM_DUTY_MAX`)
- `clock_init(SYSTEM_CLOCK_120M)` — must be first call in main
- `debug_init()` — sets up UART3 at 115200 on B10/B11

The PWM channel enum encodes `[timer_id][map_id][channel][pin]` in its hex value — see `zf_driver_pwm.h`.

### User Code Entry Points

- `main.c`: `main()` — contains all initialization (clock, GPIO, PWM) and the `while(1)` control loop
- `isr.c`: Interrupt handlers — most are empty stubs from the library template; only USART3 (debug), UART5 (camera), UART7 (wireless), UART8 (GNSS), DVP, and EXTI13-15 have active handlers

## User Application Logic

The line-tracking algorithm in `main.c`:
1. Read 5 IR sensors (GPIO digital input, pull-up; LOW = black line detected)
2. Map sensor pattern to error value: center=0, left=-4, right=4, lost=keep last
3. Compute differential motor speed: `left = BASE + error*KP`, `right = BASE - error*KP`
4. Apply via PWM on TIM5 channels (A1=left, A3=right) + GPIO direction pins (A5/A7=left, C5/B1=right)

Current tuning constants: `KP=60`, `BASE_SPEED=3000`, `PWM_DUTY_MAX=10000`. Right motor has +160 compensation for hardware asymmetry.

## Conventions

- Pin naming: single-letter port + number, e.g. `A5`, `C8`, `B1` (from `gpio_pin_enum`)
- All `zf_*` headers use GBK encoding (Chinese comments in source files appear garbled in UTF-8 editors)
- `zf_common_headfile.h` is the master include — includes all SDK and library headers; user code typically just includes this one file
- The `project/code/` folder is the intended location for new user modules (per MRS convention)
- `infrared_tracking.h` defines data structures and function prototypes that are not yet implemented — it was designed but `main.c` reads GPIO directly instead
