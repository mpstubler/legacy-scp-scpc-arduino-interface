# Representative signal captures

[Project home](../README.md) · [Build & firmware](BUILD_AND_FIRMWARE.md) · [Interface](INTERFACE.md)

The repository includes representative `.sr` logic-analyzer recordings used during interface characterization:

- [0 RPM / sweep sample](../captures/0rpm%20sweeping%20up%20sample%20.sr)
- [1000 RPM](../captures/1000rpm.sr)
- [2000 RPM](../captures/2000%20rpm.sr)
- [3000 RPM](../captures/3000%20rpm.sr)
- [slow sweep upward](../captures/slow%20sweep%201000%20up.sr)
- [slow sweep downward](../captures/slow%20sweep%20down%201200%20down.sr)

These are representative development captures rather than a complete archive of every exploratory recording or analysis step.

## Context

The project used a HiLetgo FX2-based USB logic analyzer with PulseView/sigrok and 24-MHz acquisition during characterization. The `.sr` files are sigrok session recordings for waveform inspection in PulseView.

The filenames identify the recorded operating condition or sweep. They are development evidence, not a calibrated pump-performance dataset or a complete protocol specification. Temporary exploratory parsers were not systematically preserved and are not needed to operate the final interface.

For the signal roles and limits of the recovered protocol, see [interface architecture and scope](INTERFACE.md). For capture-tool references and historical version gaps, see [hardware and software](Hardware_and_Software.md).
