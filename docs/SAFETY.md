# Safety architecture and research-use scope

[Project home](../README.md) · [Build & firmware](BUILD_AND_FIRMWARE.md) · [Interface](INTERFACE.md)

## Native-control fallback

The interface is removable and does not permanently modify the pump.

Only DATA is intercepted. Native CLOCK, FRAME, TACH, and motor-control electronics remain in place.

With the relay deenergized, native DATA passes directly from the control panel to the motor-control board. Energizing the relay selects the buffered interface path. External FRAME-controlled buffer logic then passes native DATA while FRAME is high and Arduino replacement DATA while FRAME is low.

**Relay fallback restores native command control. It does not stop the pump.**

Development of the interface also demonstrated that experimental manipulation of undocumented hardware can produce unexpected behavior. All characterization and testing should therefore be restricted to equipment permanently removed from clinical service.

## Firmware limits are not comprehensive fault protection

The published sketch checks command acceptance and relay engagement and releases the relay when pressure-control correction reaches its programmed command bound. Those checks are not a general watchdog, disconnected-sensor detector, communication-loss shutdown, or calibrated pressure/RPM/flow limit.

Do not reset or upload while relying on synthetic DATA with the pump powered. Follow the illustrated build guide for power sequencing, pre-power checks, operation, and return to native control. Native fallback leaves pump operation under the original panel.

## Research use and disclaimer

This repository documents a research prototype for use only with perfusion equipment permanently removed from clinical service.

**Not for clinical or patient-care use. Do not return modified or interfaced equipment to clinical service.**

The system was demonstrated using surrogate fluid in a benchtop circuit. Animal, cadaveric, isolated-organ, and other preclinical applications were not validated. Any such use requires independent technical assessment, protocol-specific validation, and applicable approvals.

The inexpensive pressure sensor used in the demonstration was selected to establish proof of concept at minimal cost and should not be interpreted as recommended laboratory instrumentation.

The hardware, software, and documentation are provided as is, without warranty. Anyone building, connecting, modifying, or operating the interface assumes responsibility for equipment behavior and its use.

## Build and license references

[Build guide (PDF)](../SCP_SCPC_GitHub_Build_Guide.pdf) · [Firmware](../scp_scpc_pressure_control.ino) · [Pressure instrumentation](PRESSURE_INSTRUMENTATION.md)

Code is licensed under [PolyForm Noncommercial 1.0.0](../LICENSE-CODE.md). Original documentation, diagrams, and photographs are licensed under [CC BY-NC 4.0](../LICENSE-DOCS.md). Commercial use requires separate permission from the author.
