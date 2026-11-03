# Safety architecture and research-use scope

[Project home](../README.md) · [Build & firmware](BUILD_AND_FIRMWARE.md) · [Interface](INTERFACE.md)

## Proof-of-concept scope

This project demonstrates the feasibility of repurposing decommissioned perfusion equipment for programmable control. It is intended to encourage further research, not provide a finished or validated system.

The pressure-control demonstration was optimized for the limited scope of this project, including a short video, rather than validated perfusion performance. Researchers adapting it for isolated-organ studies should upgrade the pressure instrumentation, refine the feedback-control code, and validate performance for their specific application.

Researchers interested in further development or collaboration are welcome to contact the author.

## Native-control fallback

The interface is removable and does not permanently modify the pump. Only DATA is intercepted; native CLOCK, FRAME, TACH, and motor-control electronics remain unchanged.

With the relay deenergized, native DATA passes directly from the control panel to the motor-control board. When energized, the buffered interface passes native DATA while FRAME is high and Arduino replacement DATA while FRAME is low.

**Relay fallback restores native command control. It does not stop the pump.**

Experimental manipulation of undocumented hardware can produce unexpected behavior. Testing must be restricted to equipment permanently removed from clinical service.

## Firmware and safety limitations

The published firmware checks command acceptance and relay engagement and releases the relay at its programmed pressure-correction bound. These checks do not constitute a general watchdog, sensor-failure detector, communication-loss protection, or calibrated pressure/RPM/flow limits.

**Do not reset or upload firmware while relying on synthetic DATA with the pump powered.** Follow the [build guide](SCP_SCPC_GitHub_Build_Guide.pdf) for power sequencing, pre-power checks, operation, and return to native control.

## Research use and disclaimer

**For research use with permanently decommissioned equipment only. Not for clinical or patient-care use. Do not return interfaced equipment to clinical service.**

Testing was limited to surrogate fluid in a benchtop circuit. Animal, cadaveric, isolated-organ, and other preclinical applications were not validated. Such applications require independent technical assessment, protocol-specific validation, and applicable approvals.

The inexpensive demonstration pressure sensor was selected to minimize proof-of-concept costs and is not recommended as validated laboratory instrumentation.

All hardware, software, and documentation are provided **as is, without warranty**. Anyone building, modifying, or operating the interface assumes responsibility for equipment behavior and its use.

## Build and license references

[Build guide (PDF)](SCP_SCPC_GitHub_Build_Guide.pdf) · [Firmware](../firmware/scp_scpc_pressure_control/scp_scpc_pressure_control.ino) · [Pressure instrumentation](PRESSURE_INSTRUMENTATION.md)

Code: [PolyForm Noncommercial 1.0.0](../LICENSE-CODE.md). Documentation, diagrams, and photographs: [CC BY-NC 4.0](../LICENSE-DOCS.md). Commercial use requires separate permission from the author.