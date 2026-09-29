# SCP/SCPC Arduino Command Interface

A removable Arduino interface for bench control of a permanently decommissioned Sorin SCP/SCPC centrifugal pump.

The interface connects at ZPR 9909 A / CON2 and intercepts only the DATA line while leaving the native CLOCK, FRAME, TACH, motor drive, and pump electronics in place. A relay provides a direct native DATA path when deenergized. Relay fallback therefore returns command control to the original pump panel; it is not a pump-stop function.

The included Arduino firmware reads the native command stream and demonstrates:

- synchronized command replay
- bounded modification of the native command
- return to native control
- pressure-feedback control in a surrogate-fluid benchtop circuit

The goal of the project is not to reproduce the complete SCP/SCPC communication protocol. It is to document the minimum recovered interface needed to reuse the existing pump hardware as a programmable research platform.

## Start here

For someone attempting to understand or reproduce the demonstrated system:

1. **Build guide** — wiring, connector pinout, removable harness, interface circuit, photographs, and operating procedure
2. **Arduino firmware** — native command decoding, replay, bounded offsets, relay control, and pressure-feedback demonstration
3. **Hardware and software reference** — components, development tools, software, documentation links, and known gaps in the historical record
4. **Recorded expenses** — itemized development purchases and a summary of project spending
5. **Representative signal captures** — example native and replayed communication used during interface characterization

## Repository contents

### Build guide

The illustrated build guide documents the final working interface, including:

- ZPR 9909 A / CON2 signal identification
- removable DuPont-style Y-harness
- native DATA bypass
- SN74HC125N signal-selection circuit
- relay behavior
- Arduino Uno R4 Minima connections
- pressure-feedback setup
- operating and fallback procedure

### Firmware

The Arduino firmware implements the final demonstrated control system.

It monitors the native command stream, stores accepted commands, reproduces them using the pump's native CLOCK and FRAME timing, and permits bounded modification of the transmitted command.

The pressure-control demonstration uses an analog pressure signal to modify the command around an operator-selected baseline while enforcing firmware limits.

### Hardware and software reference

`HARDWARE_AND_SOFTWARE.md` provides a consolidated description of the hardware, software, bench equipment, and development tools documented during the project.

It distinguishes:

- hardware required for the final interface
- equipment used during reverse engineering
- pressure-circuit components
- software used for signal capture, analysis, and firmware development
- exploratory purchases that were not required by the final design

Where available, links are provided to manufacturer or upstream project documentation.

### Development costs

`RECORDED_EXPENSES.csv` contains the 30 purchase lines preserved in the project expense record.

Recorded development spending was:

- **$268.09 before tax**
- **$291.21 reported actual spending**

These values describe historical project purchases, not the minimum cost of reproducing one interface. The purchases include reusable tools, multipacks, exploratory components, and general bench supplies. Existing pump equipment, computers, labor, and some pre-existing supplies are not included.

![Recorded development spending by category](recorded_expenses.png)

## Scope of the reverse engineering

The project intentionally stopped short of complete protocol emulation.

Communication between the native control panel and motor drive was characterized only far enough to:

1. identify the relevant DATA, CLOCK, FRAME, TACH, and reference conductors;
2. decode the recurring command field used for the demonstrated work;
3. reproduce the native command timing;
4. modify that command within tested limits; and
5. return control to the native DATA path.

Unresolved portions of the communication protocol were not pursued once they were unnecessary for the demonstrated control functions.

Temporary exploratory analysis scripts were treated as development tools and were not systematically preserved. The reproducible output of the project is the documented interface, firmware, representative captures, and benchtop operating procedure.

## Safety architecture

The interface is removable and does not permanently modify the pump.

Only DATA is intercepted. Native CLOCK, FRAME, TACH, and motor-control electronics remain in place.

With the relay deenergized, native DATA passes directly from the control panel to the motor-control board. Energizing the relay selects the replacement DATA path during the appropriate transmission window.

**Relay fallback restores native command control. It does not stop the pump.**

Development of the interface also demonstrated that experimental manipulation of undocumented hardware can produce unexpected behavior. All characterization and testing should therefore be restricted to equipment permanently removed from clinical service.

## License

The Arduino firmware is licensed under the **PolyForm Noncommercial License 1.0.0**.

The build guide, original photographs, and project documentation are licensed under **Creative Commons Attribution-NonCommercial 4.0 International**.

Commercial use requires separate permission from the author.

## Research use and disclaimer

This repository documents a research prototype for use only with perfusion equipment permanently removed from clinical service.

**Not for clinical or patient-care use. Do not return modified or interfaced equipment to clinical service.**

The system was demonstrated using surrogate fluid in a benchtop circuit. Animal, cadaveric, isolated-organ, and other preclinical applications were not validated. Any such use requires independent technical assessment, protocol-specific validation, and applicable approvals.

The hardware, software, and documentation are provided as is, without warranty. Anyone building, connecting, modifying, or operating the interface assumes responsibility for equipment behavior and its use.
