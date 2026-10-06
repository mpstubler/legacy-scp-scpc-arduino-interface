# Pressure instrumentation

[Project home](../README.md) · [Build & firmware](BUILD_AND_FIRMWARE.md) · [Interface](INTERFACE.md)

The pressure-feedback demonstration intentionally used an inexpensive generic 5-V pressure transducer as a low-cost proof of concept. **That sensor should not be treated as the preferred instrumentation choice for a formal laboratory experiment.**

For experimental work, use a pressure transducer with documented accuracy, calibration, electrical output, measurement range, and fluid compatibility. The firmware should then be updated for that transducer's transfer function, operating range, calibration, and appropriate alarm or control thresholds.

Where a sterile fluid pathway is required, isolate the reusable transducer from the circuit using an appropriate sterile pressure dome or diaphragm interface with compatible Luer-lock adapters, and validate the complete pressure-monitoring assembly for the intended experiment.

The demonstrated controller primarily treated the sensor as an analog signal source referenced to an operator-selected baseline rather than as a calibrated absolute-pressure instrument. Pressure-control values and thresholds in the published demonstration firmware are therefore based on raw/filtered ADC counts, not calibrated pressure units.

## Demonstrated implementation

| Parameter | Published implementation |
| --- | --- |
| Sensor | Generic nominal 5-V, 0–30 psi analog pressure transducer; original listing not preserved |
| Arduino input | A0, with 10-bit ADC readings (`0`–`1023`) |
| Target | Current filtered sensor reading captured when pressure mode starts |
| Sampling/filtering | Scheduled every 20 ms |
| Filter | `0.45 × new reading + 0.55 × previous filtered value` |
| Feedback calculation | Scheduled every 50 ms |
| Deadband | ±2 filtered ADC counts |
| Correction limit | ±65 CMD16 counts from the captured command; reaching the bound in pressure mode releases the relay and latches an error |

The timing values are cooperative-loop schedules, not guaranteed hard-real-time deadlines. The correction limit is expressed in **pump-command counts**, separate from pressure-sensor ADC counts. Neither is a calibrated pressure limit.

The feedback loop runs on the Arduino. The host computer supplies programming and serial interaction. Independent pressure measurement establishes and observes the operating condition.

## Record limitations

The exact original sensor listing, transfer function, accuracy, and wire assignments were not preserved. Generic marketplace listings do not establish the electrical behavior of an individual replacement unit. The [hardware reference](../Hardware_and_Software.md) records the known sensor and plumbing details and the remaining uncertainties.

See [build & firmware](BUILD_AND_FIRMWARE.md) for the source and operating guide, and [demonstrations](DEMONSTRATIONS.md) for the bench context.
