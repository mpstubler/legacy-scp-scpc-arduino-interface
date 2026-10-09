# SCP/SCPC — Arduino Pump Interface

**Programmable control of decommissioned perfusion hardware for research.**

A removable Arduino interface for the Sorin/Stöckert SCP/SCPC centrifugal pump, demonstrating direct pump-speed control and pressure-feedback automation.

## Lost or confused? [Orientation Page here →](docs/WHAT_AM_I_LOOKING_AT.md)

An orientation page introducing the project, its purpose, and its scope:

- The original equipment and the added interface
- Research applications for programmable perfusion hardware
- Demonstrated capabilities and current limitations
- Opportunities for collaboration and further development

## Demonstrations

*Two quick bench demos. Cinematography was outside the scope of the project.*

**Proof of concept · Saline bench testing · Research use only**

| Pressure control | Direct RPM control |
| --- | --- |
| [![Watch the pressure-control demonstration](media/images/pressure-control.jpg)](media/demos/PressureDemo_1080p30_SquareView.mp4) | [![Watch the direct RPM control demonstration](media/images/native-control-fallback.jpg)](media/demos/RelayDemo_1080p30_Boxed.mp4) |
| **[Watch video →](media/demos/PressureDemo_1080p30_SquareView.mp4)** | **[Watch video →](media/demos/RelayDemo_1080p30_Boxed.mp4)** |
| The Arduino adjusts pump speed in response to changes in circuit resistance, returning the pressure-sensor signal toward its baseline. | The Arduino changes pump speed by replacing the internal control command. Releasing the interface restores control to the original panel. |

## Project documentation

| Resource | Contents |
| --- | --- |
| **[Interface overview](docs/INTERFACE.md)** | System architecture, command interception, and native-control fallback |
| **[Build & firmware](docs/BUILD_AND_FIRMWARE.md)** | Illustrated assembly guide, Arduino code, component references, and operating instructions |
| **[Demonstration notes](docs/DEMONSTRATIONS.md)** | Experimental setup, control behavior, and measurement limitations |
| **[Safety & research scope](docs/SAFETY.md)** | Tested conditions, fallback limitations, and use restrictions |

## Research use and licensing

**Use only permanently decommissioned equipment. Not for clinical or patient-care use. Do not return modified or interfaced equipment to clinical service.**

Matthew Stubler · [Code: PolyForm Noncommercial](LICENSE-CODE.md) · [Documentation: CC BY-NC 4.0](LICENSE-DOCS.md)

Commercial use requires separate permission.