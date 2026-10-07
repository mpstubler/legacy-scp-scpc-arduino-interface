# Interface architecture and reverse-engineering scope

[Project home](../README.md) · [Build & firmware](BUILD_AND_FIRMWARE.md) · [Orientation](WHAT_AM_I_LOOKING_AT.md)

The project recovered the minimum native interface needed to make the existing pump hardware programmable for bench research. The added interface is removable and connects at **ZPR 9909 A / CON2**.

![Simplified native and added command paths](../media/images/system-overview.png)

## Native and added functions

Only **DATA** is intercepted. CLOCK, FRAME, TACH, the local reference, motor drive, and pump electronics retain their native connections. CLOCK and FRAME provide communication timing; the Arduino does not generate them.

| State | DATA source reaching the motor-control board |
| --- | --- |
| Relay deenergized | Direct native DATA from the original panel |
| Relay energized, FRAME high | Buffered native DATA |
| Relay energized, FRAME low | Arduino replay/replacement DATA |

The relay selects the direct native bypass or the buffered interface path. With the interface selected, FRAME-controlled buffer logic performs the windowed DATA selection. **The relay itself does not switch on every command window.**

The Arduino monitors native DATA upstream of substitution, decodes the recurring 16-bit command field (CMD16), and supplies synchronized replay or a bounded modified command. Pressure feedback is one application of that recovered interface.

TACH remains in place but is not used for feedback by the published sketch. Command offsets are not calibrated RPM or flow setpoints.

**Relay fallback restores command authority to the native panel. It does not stop the pump.** See [safety architecture](SAFETY.md).

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

## Evidence and reproduction

- [Signal captures](SIGNAL_CAPTURES.md): representative native recordings from development.
- [Build & firmware](BUILD_AND_FIRMWARE.md): final interface implementation and operating documentation.
- [Hardware and software](Hardware_and_Software.md): tools, components, exploratory work, and historical gaps.
- [Demonstrations](DEMONSTRATIONS.md): pressure feedback, direct RPM control, and return to native control on the bench.
