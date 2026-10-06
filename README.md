# SCP/SCPC — Programmable Research Pump Interface

**Retired perfusion hardware, with external command control and a pressure-feedback demonstration.**

A removable Arduino interface adds programmable bench control to a permanently decommissioned Sorin/Stöckert SCP/SCPC centrifugal pump. It reads and modifies native pump commands while retaining the original control panel, motor drive, and pump electronics.

**Status:** Proof of concept · Demonstrated in a surrogate-fluid bench circuit · Research use only

[Watch the demos](#demonstrations) · [Explore the documentation](#project-resources) · [Project background](WHAT_AM_I_LOOKING_AT.md)

## Demonstrations

| Watch | What it demonstrates |
| --- | --- |
| **[Pressure-feedback control](PressureDemo_Revised.mp4)** | The Arduino adjusts the pump command in response to a pressure signal as circuit resistance changes. |
| **[Return to native control](RelayDemo_Revised.mp4)** | Relay switching returns command authority to the original pump panel. **This does not stop the pump.** |

[Demo setup and interpretation →](docs/DEMONSTRATIONS.md)

## Why this project exists

Ex vivo perfusion and organ-preservation research motivate the work: a programmable pump could respond to measured conditions instead of requiring repeated manual adjustments. This project demonstrates that control approach on a saline bench loop.

The broader goal is to reuse functional retired clinical hardware for research automation. The SCP/SCPC is the proof of concept; applying the approach to other systems would require new characterization and validation.

## How it works

<p align="center">
  <img src="system-overview.png" alt="Native pump commands are read by an Arduino; a relay selects the native DATA path or a buffered interface that substitutes Arduino commands during the native command window. Pressure feedback adjusts the replacement command." width="1000">
</p>

The interface connects at **ZPR 9909 A / CON2** and intercepts only **DATA**. Native CLOCK, FRAME, TACH, motor drive, and pump electronics remain in place.

| Retained hardware | Added interface | Demonstrated result |
| --- | --- | --- |
| Native command stream and timing | Arduino command decoding and replay | Synchronized, bounded command modification |
| Original panel and direct DATA path | Relay-selected native bypass | Return to native command control |
| Surrogate-fluid bench circuit | Analog pressure feedback | Automatic command adjustment around a captured baseline |

[Interface architecture and reverse-engineering scope →](docs/INTERFACE.md)

## Project resources

| Explore | Contents |
| --- | --- |
| **[Build & firmware](docs/BUILD_AND_FIRMWARE.md)** | Illustrated build guide, source code, operating modes, and where to start |
| **[Hardware & software](Hardware_and_Software.md)** | Final interface components, development tools, dependencies, and known gaps |
| **[Pressure instrumentation](docs/PRESSURE_INSTRUMENTATION.md)** | Demonstration sensor, ADC-based control, and measurement limitations |
| **[Signal captures](docs/SIGNAL_CAPTURES.md)** | Six representative logic-analyzer recordings |
| **[Development costs](docs/DEVELOPMENT_COSTS.md)** | Historical spending, itemized purchases, and cost-chart context |
| **[Safety & research scope](docs/SAFETY.md)** | Native fallback behavior, validation boundaries, and full disclaimer |

## Research scope and licensing

Demonstrated: native command decoding, synchronized replay, bounded modification, relay fallback, and pressure feedback. Complete protocol emulation was outside scope. Animal, cadaveric, and isolated-organ applications were not validated.

**Use only permanently decommissioned equipment. Not for clinical or patient-care use. Do not return modified or interfaced equipment to clinical service.**

By **Matthew Stubler**. [Code: PolyForm Noncommercial](LICENSE-CODE.md) · [Documentation: CC BY-NC 4.0](LICENSE-DOCS.md). Commercial use requires separate permission.
