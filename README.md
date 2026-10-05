# SCP/SCPC Semi-Autonomous Arduino Interface

**Reverse-engineered native pump commands, with a pressure-feedback demonstration.**

A removable Arduino interface for bench control of a permanently decommissioned Sorin SCP/SCPC centrifugal pump. The project recovered the native command field and timing needed to inject modified commands while retaining the original motor drive and pump electronics.

<p align="center">
  <img src="system-overview.png" alt="Reverse-engineered SCP/SCPC interface: Arduino reads native pump commands and supplies replacement CMD16 commands through a relay, with native fallback and a pressure-feedback demonstration." width="1100">
</p>

<p align="center">
  <sub>Simplified system overview. The pressure-feedback loop demonstrates one use of the recovered command interface.</sub>
</p>

**Confused what you're looking at? [Start here.](./WHAT_AM_I_LOOKING_AT.md)**

[Build guide](./SCP_SCPC_GitHub_Build_Guide.pdf) · [Firmware](./scp_scpc_pressure_control.ino) · [Hardware & software](./Hardware_and_Software.md) · [Signal captures](#representative-signal-captures) · [Development costs](#development-costs)

## Interface overview

| Native system | Added interface | Demonstrated capability |
| --- | --- | --- |
| Original control panel and command timing | Read the recurring native command field | Synchronized command replay |
| Original motor drive and pump electronics | Inject bounded modifications to native-format commands | Programmable bench control |
| Direct native DATA path | Relay selection of native or replacement DATA | Return to native control |
| Surrogate-fluid benchtop circuit | Pressure signal used to adjust the replacement command | Pressure-feedback control |

The interface connects at **ZPR 9909 A / CON2** and intercepts only **DATA**. Native CLOCK, FRAME, TACH, motor drive, and pump electronics remain in place. A relay provides a direct native DATA path when deenergized.

**Relay fallback restores command control to the original pump panel. It does not stop the pump.**

The goal is to document the minimum recovered interface needed to reuse existing pump hardware as a programmable research platform. Complete SCP/SCPC protocol emulation was outside the scope of the project.

## How to use

For someone attempting to understand or reproduce the demonstrated system:

| Resource | Contents |
| --- | --- |
| **[Build guide](./SCP_SCPC_GitHub_Build_Guide.pdf)** | Wiring, connector pinout, removable harness, interface circuit, photographs, and operating procedure |
| **[Arduino firmware](./scp_scpc_pressure_control.ino)** | Native command decoding, replay, bounded offsets, relay control, and pressure-feedback demonstration |
| **[Hardware and software reference](./Hardware_and_Software.md)** | Components, development tools, software, documentation links, and known gaps in the historical record |
| **[Recorded expenses](./RECORDED_EXPENSES.csv)** | Itemized development purchases and a summary of project spending |
| **[Representative signal captures](#representative-signal-captures)** | Example recordings used during interface characterization |

## Build guide

The illustrated build guide documents the final working interface, including:

- ZPR 9909 A / CON2 signal identification
- removable DuPont-style Y-harness
- native DATA bypass
- SN74HC125N signal-selection circuit
- relay behavior
- Arduino Uno R4 Minima connections
- pressure-feedback setup
- operating and fallback procedure

## Firmware

The [Arduino firmware](./scp_scpc_pressure_control.ino) implements the final demonstrated control system.

It monitors the native command stream, stores accepted commands, reproduces them using the pump's native CLOCK and FRAME timing, and permits bounded modification of the transmitted command.

The pressure-control demonstration uses an analog pressure signal to modify the command around an operator-selected baseline while enforcing firmware limits.

### Pressure-control instrumentation

The pressure-feedback demonstration intentionally used an inexpensive generic 5-V pressure transducer as a low-cost proof of concept. **That sensor should not be treated as the preferred instrumentation choice for a formal laboratory experiment.**

For experimental work, use a pressure transducer with documented accuracy, calibration, electrical output, measurement range, and fluid compatibility. The firmware should then be updated for that transducer's transfer function, operating range, calibration, and appropriate alarm or control thresholds.

Where a sterile fluid pathway is required, isolate the reusable transducer from the circuit using an appropriate sterile pressure dome or diaphragm interface with compatible Luer-lock adapters, and validate the complete pressure-monitoring assembly for the intended experiment.

The demonstrated controller primarily treated the sensor as an analog signal source referenced to an operator-selected baseline rather than as a calibrated absolute-pressure instrument. Pressure-control values and thresholds in the published demonstration firmware are therefore based on raw/filtered ADC counts, not calibrated pressure units.

## Hardware and software reference

The [hardware and software reference](./Hardware_and_Software.md) provides a consolidated description of the hardware, software, bench equipment, and development tools documented during the project.

It distinguishes:

- hardware required for the final interface
- equipment used during reverse engineering
- pressure-circuit components
- software used for signal capture, analysis, and firmware development
- exploratory purchases that were not required by the final design

Where available, links are provided to manufacturer or upstream project documentation.

## Development costs

[`RECORDED_EXPENSES.csv`](./RECORDED_EXPENSES.csv) contains the 30 purchase lines preserved in the project expense record.

| Recorded development spending | Amount |
| --- | ---: |
| Before tax | **$268.09** |
| Reported actual spending | **$291.21** |

These values describe historical project purchases, not the minimum cost of reproducing one interface. The purchases include reusable tools, multipacks, exploratory components, and general bench supplies. Existing pump equipment, computers, labor, and some pre-existing supplies are not included.

![Recorded development spending by category](recorded_expenses.png)

## Representative signal captures

The repository includes representative `.sr` logic-analyzer recordings used during interface characterization:

- [0 RPM / sweep sample](./0rpm%20sweeping%20up%20sample%20.sr)
- [1000 RPM](./1000rpm.sr)
- [2000 RPM](./2000%20rpm.sr)
- [3000 RPM](./3000%20rpm.sr)
- [slow sweep upward](./slow%20sweep%201000%20up.sr)
- [slow sweep downward](./slow%20sweep%20down%201200%20down.sr)

These are representative development captures rather than a complete archive of every exploratory recording or analysis step.

## Scope of the reverse engineering

The project intentionally stopped short of complete protocol emulation.

Communication between the native control panel and motor drive was characterized only far enough to:

1. identify the relevant DATA, CLOCK, FRAME, TACH, and reference conductors;
2. decode the recurring command field used for the demonstrated work;
3. synchronize replacement DATA with the native communication timing;
4. modify the command within tested limits; and
5. return control to the native DATA path.

Unresolved portions of the communication protocol were not pursued once they were unnecessary for the demonstrated control functions.

Temporary exploratory analysis scripts were treated as development tools and were not systematically preserved or organized for distribution. They are not required to operate or reproduce the final interface.

The reproducible output of the project is the documented interface, firmware, representative captures, hardware/software reference, and benchtop operating procedure.

## Safety architecture

The interface is removable and does not permanently modify the pump.

Only DATA is intercepted. Native CLOCK, FRAME, TACH, and motor-control electronics remain in place.

With the relay deenergized, native DATA passes directly from the control panel to the motor-control board. Energizing the relay selects the replacement DATA path during the appropriate transmission window.

**Relay fallback restores native command control. It does not stop the pump.**

Development of the interface also demonstrated that experimental manipulation of undocumented hardware can produce unexpected behavior. All characterization and testing should therefore be restricted to equipment permanently removed from clinical service.

## License

| Material | License |
| --- | --- |
| Arduino firmware | **[PolyForm Noncommercial License 1.0.0](./LICENSE-CODE.md)** |
| Build guide, original photographs, and project documentation | **[Creative Commons Attribution-NonCommercial 4.0 International](./LICENSE-DOCS.md)** |

Commercial use requires separate permission from the author.

## Research use and disclaimer

This repository documents a research prototype for use only with perfusion equipment permanently removed from clinical service.

**Not for clinical or patient-care use. Do not return modified or interfaced equipment to clinical service.**

The system was demonstrated using surrogate fluid in a benchtop circuit. Animal, cadaveric, isolated-organ, and other preclinical applications were not validated. Any such use requires independent technical assessment, protocol-specific validation, and applicable approvals.

The inexpensive pressure sensor used in the demonstration was selected to establish proof of concept at minimal cost and should not be interpreted as recommended laboratory instrumentation.

The hardware, software, and documentation are provided as is, without warranty. Anyone building, connecting, modifying, or operating the interface assumes responsibility for equipment behavior and its use.
