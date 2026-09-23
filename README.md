# legacy-scp-scpc-arduino-interface
Removable Arduino interface for bench control of a decommissioned Sorin SCP/SCPC pump.

# SCP/SCPC Arduino Command Interface

A removable Arduino interface for a permanently decommissioned Sorin SCP/SCPC centrifugal pump. It connects at ZPR 9909 A / CON2 and intercepts the DATA line while leaving native CLOCK, FRAME, TACH, and the pump electronics in place. A relay provides a direct native DATA path when deenergized; relay fallback returns command control to the native panel, **not** a pump stop.

The included firmware reads the native command stream and demonstrates command replay, bounded offsets, and pressure feedback control on a surrogate-fluid benchtop circuit.

## Build files

* [Build guide: wiring, board layout, harness, photos, and operation](./SCP_SCPC_GitHub_Build_Guide.pdf)
* [Arduino firmware](./scp_scpc_pressure_control.ino)
## License

The Arduino firmware is licensed under the PolyForm Noncommercial License 1.0.0. The build guide and original photos are licensed under Creative Commons Attribution-NonCommercial 4.0 International. Commercial use requires separate permission from the author.

## Research use and disclaimer

Research prototype for use only with perfusion equipment permanently removed from clinical service. **Not for clinical or patient-care use.** Do not return modified or interfaced equipment to clinical service.

This project was demonstrated with surrogate fluid on a benchtop circuit. Animal, cadaveric, isolated-organ, and other preclinical applications were not validated; any such use requires independent technical assessment, protocol-specific validation, and applicable approvals.

The hardware, software, and documentation are provided as is, without warranty. Anyone building, connecting, or operating the interface assumes responsibility for equipment behavior and its use.
