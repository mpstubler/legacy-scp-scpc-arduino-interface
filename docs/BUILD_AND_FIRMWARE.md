# Build & firmware

[Project home](../README.md) · [Build & firmware](BUILD_AND_FIRMWARE.md) · [Interface](INTERFACE.md)

This page is the entry point for understanding or reproducing the documented bench interface. The illustrated PDF contains the wiring, photographs, connector reference, and operating procedure; this page does not replace those assembly instructions.

## Start here

1. Read the [safety architecture and research-use scope](SAFETY.md).
2. Use the **[illustrated build guide (PDF)](SCP_SCPC_GitHub_Build_Guide.pdf)** for connector orientation, wiring, removable harness, pre-power checks, and operating procedure.
3. Check the [hardware and software reference](Hardware_and_Software.md) for components, development tools, dependencies, and unresolved identifiers.
4. Review the **[Arduino firmware](../firmware/scp_scpc_pressure_control/scp_scpc_pressure_control.ino)** alongside the guide. For pressure feedback, also read the [instrumentation notes](PRESSURE_INSTRUMENTATION.md).

## What the build guide covers

- ZPR 9909 A / CON2 signal identification and connector pinout
- Removable DuPont-style Y-harness and native DATA bypass
- SN74HC125N buffer, inverter, relay, and external power arrangement
- Arduino Uno R4 Minima connections
- Pressure-feedback setup
- Pre-power checks, operating modes, and return to native control

Use the documented **Uno R4 Minima** target. Arduino pin numbers, component pins, board rows, and CON2 contact numbers are distinct references; use the guide's exact mapping.

## What the firmware does

The sketch monitors the native command stream, stores accepted commands, and reproduces native-format DATA using the pump's CLOCK and FRAME timing. It permits bounded command modification and uses an analog pressure signal for the feedback demonstration.

The Arduino does not generate CLOCK or FRAME. External buffer logic selects native or replay DATA within the appropriate FRAME window. TACH remains in the native path; the published sketch has no TACH input or calibrated RPM/flow feedback.

## Operator controls

After completing the guide's setup and checks, use the Arduino Serial Monitor at **115200 baud**, with **Newline** or **Both NL & CR** line ending.

| Input | Behavior |
| --- | --- |
| `P` or `p`, then Enter | Capture the current pressure signal and accepted command; start pressure mode. |
| `r`, then Enter | Capture the current accepted command; start manual offset mode. |
| Signed integer, then Enter | In manual mode, request that CMD16 offset from the captured baseline. Offsets are absolute relative to the baseline, not cumulative jogs. |
| Blank Enter or `n`, then Enter | Return to native DATA control and clear any latched control-limit error. This does not stop the pump or restart a control mode. |

Only `P`/`p` accepts both cases; `r` and `n` are lowercase.

## Bounds and interpretation

Manual requests are clamped to ±65 CMD16 counts. In pressure mode, a requested correction reaching that bound latches an error and deenergizes the relay before assigning that target. Both modes also use the absolute command range `0x0000`–`0x0700`.

These are **command bounds**, not calibrated pressure, RPM, or flow limits. The displayed RPM-change estimate is load dependent and uncalibrated. Read the source comments and guide for command-acceptance and relay-engagement checks.

Do not reset or upload firmware while relying on synthetic DATA with the pump powered. [Review fallback behavior and limitations →](SAFETY.md)

## Supporting evidence

[Representative signal captures](SIGNAL_CAPTURES.md) document interface characterization. [Development costs](DEVELOPMENT_COSTS.md) record historical purchases, including reusable tools and exploratory components; they are not a minimum one-unit bill of materials.
