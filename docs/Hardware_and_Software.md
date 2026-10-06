# SCP/SCPC Hardware and Software Reference

[Project home](../README.md) · [Build & firmware](BUILD_AND_FIRMWARE.md) · [Development costs](DEVELOPMENT_COSTS.md)

Matthew Stubler • Research build reference

This document identifies the hardware, software, tools, and purchases documented during development of the removable Arduino interface for a permanently decommissioned Sorin SCP/SCPC centrifugal pump. It supports review of the project and planning a comparable bench build. The final interface requirements are separated from equipment used during reverse engineering and purchases whose eventual use is uncertain.

The project demonstrated command replay, bounded command offsets, and pressure feedback in a recirculating saline circuit. Use this inventory alongside the [illustrated build guide](SCP_SCPC_GitHub_Build_Guide.pdf) and [firmware](../firmware/scp_scpc_pressure_control/scp_scpc_pressure_control.ino), which contain the wiring and operating details. This inventory alone is not an assembly procedure.

Research use only. The system was demonstrated with surrogate fluid, not patients, animals, blood, or organs. Use only permanently decommissioned equipment. Relay fallback restores native DATA control; it does not stop the pump.

## Development costs

The [development cost record](DEVELOPMENT_COSTS.md) contains the historical spending summary, chart, and all 30 itemized purchases. It distinguishes development spending from the requirements for reproducing one interface.

## Hardware for the final interface

Quantities below describe one interface where the build guide specifies them. Expense amounts in the [development cost record](DEVELOPMENT_COSTS.md) are purchase-line totals and may cover multipacks or spare inventory. Manufacturer links identify the component family; they do not establish that a current retail revision is identical to the tested unit.

| Component and quantity | Function in this project | Identification and source |
| :---- | :---- | :---- |
| Decommissioned Sorin SCP/SCPC system, one matching system | Retains the native operator panel, motor drive, pumping hardware, and communication timing. Physical interface at ZPR 9909 A / CON2. | Existing research equipment. The project used the SCP service manual, version 02/2017, document CP\_SEM\_60-00-00.002. System compatibility must be checked against the build guide. |
| Arduino Uno R4 Minima, one | Decodes native DATA and produces replacement DATA; implements command bounds and pressure feedback. | [Arduino board documentation](https://docs.arduino.cc/hardware/uno-r4-minima). Purchased from Micro Center. This is the 5-V R4 Minima, not an assumed interchangeable Uno model. |
| USB data cable, one | Programming and host serial communication. | USB-C at the Arduino. Exact cable and host adapter models were not recorded. |
| SN74HC125N, one IC | Two tri-state channels select native or Arduino DATA according to FRAME. | [Texas Instruments SN74HC125N](https://www.ti.com/product/SN74HC125/part-details/SN74HC125N). Purchased from Mouser. |
| P2N2222/P2N2222A, one transistor | Inverts FRAME to provide complementary buffer-enable control. | P2N2222A named in assembly notes; [onsemi datasheet](https://www.onsemi.com/pdf/datasheet/p2n2222a-d.pdf) is a part reference. Verify markings and pinout on the actual device. No separate expense line identifies its supplier. |
| DFRobot DFR0017, one relay module | Deenergized relay passes native DATA directly; energized relay selects the buffered interface path. | [DFRobot documentation](https://wiki.dfrobot.com/dfr0017/). Confirm the actual module’s COM, NC, and NO connections and revision before reproducing the bypass arrangement. |
| Half-size Perma-Proto, one board | Soldered implementation of the verified breadboard circuit. | [Adafruit half-size board](https://www.adafruit.com/product/1609); [three-pack](https://www.adafruit.com/product/571). A three-pack was recorded as purchased. |
| Two 1-kΩ resistors; two 10-kΩ resistors | Buffer-output isolation and bias functions in the documented circuit. | Generic parts specified by the build guide. Use its exact placement; individual suppliers, tolerances, and purchase costs were not recorded. |
| 0.1-µF capacitor, one | Local supply decoupling. | Generic part specified by the build guide. Exact manufacturer and separate cost were not recorded. |
| Regulated external 5-V supply, one | Powers the added interface circuitry with the documented shared local reference. | Exact model and current rating were not recorded. Follow the build guide’s power arrangement; do not infer a connection from the voltage label alone. |
| Removable DuPont-style Y-harness, one assembly | Extends the native connection, provides monitoring taps, and intercepts only DATA. | Fabricated from connectors, crimp contacts, and leads. A 2 × 3 female housing mates with CON2 in the documented build. Verify connector orientation and the manufacturer pin numbering in the guide. |
| Headers, sockets, hookup wire, insulation and strain relief | Mechanical assembly and repeatable interconnection. | 14-pin IC sockets, DuPont kits, 22-AWG silicone wire, terminal blocks, and heat shrink appear in the expense record. Exact installed quantities were not recorded. |

## Pressure circuit and bench equipment

The pressure demonstration adds a sensor and hydraulic loop to the command interface. Independent pressure measurement is needed to establish and observe the operating condition; the Arduino maintained a captured sensor signal rather than a calibrated pressure value.

See [pressure instrumentation](PRESSURE_INSTRUMENTATION.md) for the demonstration's ADC-based control values and measurement limitations.

***Experimental-use recommendation:*** 

The pressure-control demonstration intentionally used inexpensive generic components to show that the recovered pump interface could support closed-loop control at minimal cost. This should not be treated as the preferred instrumentation arrangement for a formal laboratory experiment. For experimental use, a pressure transducer with documented accuracy, calibration, electrical output, and fluid compatibility is preferable. The firmware should then be updated for that transducer’s transfer function, measurement range, alarm thresholds, and any required calibration. Where a sterile fluid pathway is required, the pressure measurement should be isolated from the reusable transducer with an appropriate sterile pressure dome or diaphragm interface and compatible Luer-lock adapters, with the complete pressure-monitoring assembly validated for the intended circuit.

| Item | Documented role and identification |
| :---- | :---- |
| Generic 5-V analog pressure transducer, 0–30 psi | The project used an inexpensive generic AliExpress pressure transducer connected to Arduino A0. The original listing was not preserved. For reproduction, a currently available replacement of the same general type may be specified as 0–30 psi with a 1/8-NPT process connection and sold for nominal 5-V systems and oil, fuel, air, or water service. Marketplace listings for these generic sensors are inconsistent about supply voltage, output range, accuracy, and wire assignments, so those electrical details must be verified on the actual unit before connection. The demonstrated controller used the sensor primarily as an analog signal source referenced to an operator-selected baseline rather than as a calibrated absolute-pressure instrument. |
| Brass NPT hose-barb fitting | Recorded purchase for sensor plumbing. For a replacement sensor with a 1/8-NPT process connection, use a matching 1/8-NPT adapter or hose barb sized to the bench-circuit tubing. The exact fitting dimensions used in the original build were not preserved. |
| Centrifugal pump head and perfusion tubing | Discarded/expired perfusion components formed the saline loop. An expired Sorin centrifugal head is described in the log; exact disposable model and tubing dimensions were not recovered. |
| Saline-bag reservoir and connecting fittings | The log describes a 1-L saline bag, perfusion spikes, and tubing used to form the recirculating circuit. These existing/discarded components have no separate purchase valuation in the ledger. |
| Hoffman clamp | Downstream adjustable restriction used to vary hydraulic resistance. Manufacturer and model were not recorded. |
| Existing pressure and flow monitoring | The added sensor was connected in parallel with the pump’s existing pressure-monitoring system. Displayed RPM and indicated flow were observed. Exact external transducer and flow-probe models were not recovered. |
| HiLetgo eight-channel USB logic analyzer | FX2-based analyzer used with PulseView/sigrok; captures at 24 MHz. [HiLetgo product documentation](https://hiletgo.com/ProductDetail/1915369.html). Purchased from Amazon; exact listing/revision not retained. Used for characterization and interface verification. |
| Aicevoos AS-118D multimeter | Named in the activity log. Used for continuity, resistance, and static voltage checks. Its mixed-signal readings did not establish waveform timing or protocol identity. Exact supplier link was not recovered. |
| IC test clips and P1308B probe/test-lead kit | Temporary probing and electrical measurement. Historical supplier: AliExpress. Stable connector-based access replaced unreliable temporary probing for the working interface. |
| Breadboards and assembly tools | MB-102 breadboard kit, soldering clamp, SN-58B crimper, and iFixit tip cleaner are recorded purchases. A soldering iron, solder, wire preparation tools, and ordinary bench supplies support assembly; exact models and separate costs were not recorded. |
| Host computers | ASUS ROG Ally running Bazzite was the preferred acquisition platform in the log. An older Dell running Linux Mint was also used; the log reports USB/serial reliability problems on that machine. Windows was used for initial analyzer driver setup. Exact machine variants and OS versions were not recorded. |

A particular computer brand is not a requirement of the interface. Reliable USB communication and a supported programming/capture environment matter. The published firmware performs feedback control on the Arduino; the host provides programming and serial interaction.

## Software used in development and operation

Historical application, operating-system, board-package, and library versions were not systematically recovered. Links below point to official documentation or upstream projects, not an assertion that the current release is the version used in the experiments.

| Software | Role and documented status | Reference |
| :---- | :---- | :---- |
| Arduino IDE and Uno R4 board support | Used to edit, build, upload, and monitor sketches. Select the Uno R4 Minima target and compatible board support. Historical IDE and core versions remain unspecified. | [Arduino IDE documentation](https://docs.arduino.cc/software/ide/); [Uno R4 documentation](https://docs.arduino.cc/hardware/uno-r4-minima) |
| Project firmware | The published scp\_scpc\_pressure\_control.ino implements decoding, replay, bounded offsets, relay control, and pressure feedback. No third-party library include directives appear in the reviewed sketch. Arduino core support is still required. | [Project repository](https://github.com/mpstubler/legacy-scp-scpc-arduino-interface) |
| Arduino Serial Monitor | Operator interface at 115200 baud, with newline or both NL and CR line ending. Supports pressure mode, manual offset mode, and return to native control. | [Arduino IDE documentation](https://docs.arduino.cc/software/ide/) and repository build guide |
| PulseView | Graphical waveform recording and inspection, including native-versus-replayed DATA comparisons. | [PulseView](https://sigrok.org/wiki/PulseView) |
| sigrok-cli | Command-line analyzer access and device verification during the Linux workflow. The activity log records installation, device detection, and local test use; waveform inspection and VCD export were performed in PulseView. | [sigrok-cli](https://sigrok.org/wiki/Sigrok-cli) |
| libsigrok and fx2lafw support | Hardware-access library and FX2 analyzer firmware support used by the capture stack. | [libsigrok](https://sigrok.org/wiki/Libsigrok); [fx2lafw](https://sigrok.org/wiki/Fx2lafw); [downloads](https://sigrok.org/wiki/Downloads) |
| Python and custom parsers | Offline interpretation of exported VCD transitions, frame extraction, and CSV output. These parsers were exploratory development tools that changed as the signal interpretation evolved. They were not systematically preserved or organized for distribution and are not required to operate or reproduce the final interface. | [Python; exploratory scripts were not systematically preserved](https://www.python.org/) |
| Linux SocketCAN and can-utils | Used during CAN investigation. The log explicitly records candump and cansend for capture and local adapter testing. These are not runtime dependencies of the final DATA-substitution interface. | [SocketCAN documentation](https://docs.kernel.org/networking/can.html); [can-utils](https://github.com/linux-can/can-utils) |
| Zadig and WinUSB | Used for initial Windows access to the FX2 analyzer. This is a Windows-specific setup path, not a Linux dependency. | [Zadig](https://zadig.akeo.ie/) |
| Bazzite and Linux Mint | Documented Linux host environments. Bazzite was used on the ROG Ally; Linux Mint was used on the older Dell. | [Bazzite](https://bazzite.gg/); [Linux Mint](https://linuxmint.com/) |

The firmware uses a 10-bit ADC setting. It schedules sensor sampling/filtering at 20-ms intervals and feedback calculations at 50-ms intervals. These are cooperative-loop schedules, not a claim of guaranteed hard-real-time deadlines. The recorded smoothing factor is 0.45 for the new reading, and the deadband is ±2 ADC counts. See the code for command acceptance, arming, and limit behavior.

## Exploratory tools and other purchases

The isolated [Makerbase MKS CANable Pro](https://github.com/makerbase-mks/CANable-MKS) was used to investigate the upstream candidate CAN connection. No decodable native frames were observed under the reported test conditions. The final interface uses the downstream DATA/CLOCK/FRAME connection, so a CAN adapter is not required to operate that interface. Adapter firmware version and hardware revision were not recovered.

[SavvyCAN](https://github.com/collin80/SavvyCAN) appears in the early planned toolchain. The reviewed activity log establishes use of candump/cansend but does not establish actual use of SavvyCAN. It is therefore listed as planned, not as a confirmed experimental dependency.

The [SparkFun TXS0108E level shifter](https://www.sparkfun.com/sparkfun-level-shifter-8-channel-txs0108e.html) was purchased for possible mixed-voltage work. It is absent from the final build guide. The Nano expansion shield, FR4 prototype boards, and SchmartBoard item are recorded purchases; their exact installed use is not established by the final build record. The historical SchmartBoard description is retained without assigning an unverified catalog number.

The [Adafruit Parts Pal](https://www.adafruit.com/product/2975) is a component assortment with a storage box. The expense sheet classified it as organization only. That classification is preserved in the cost chart, but the records do not establish which, if any, of its components entered the circuit. The \$16.58 MHCN United Co. Ltd order remains unidentified. The early log also records ordering 7W2 high-current D-sub connectors; no separately identifiable line in the expense sheet can be assigned to them with confidence.

ChatGPT and Gemini assisted with technical explanations, code, analysis, and writing, as disclosed in the manuscript. They were development aids and are not required to run the interface. Specific model versions and attributable subscription costs were not recorded. Google Docs, Google Sheets, and GitHub were used to maintain the project documentation, expense record, and public distribution.

## What remains unidentified

Remaining historical details include the exact original pressure-sensor listing and its electrical transfer function, accuracy, and wire assignments; the power-supply model/rating; relay-board revision; software/core versions; and disposable circuit/monitoring model numbers. A present-day replacement specification for the pressure sensor is given above, but its actual electrical output and pinout must still be verified before connection. The transistor and passive components lack separately attributed costs. Exact marketplace listings for generic tools and consumables were not recovered; replacement listings are not presented as the original purchases.

A minimum one-unit build cost cannot be derived reliably from this ledger because purchased quantities, spare inventory, unitemized components, and existing equipment are mixed. The [development cost record](DEVELOPMENT_COSTS.md) retains the complete purchase total instead of inventing a reduced bill of materials.

## Evidence and document status

This reference reconciles the Canonical Dossier and Workday Timeline with the chronological Activity Log, the Project running total workbook, the current manuscript, and the public build guide and firmware. The dossier is primarily an index; final hardware descriptions follow the later build documentation. Historical purchase classifications are retained only as accounting records. Private working-document links, institutional correspondence, and personal order identifiers are omitted from this public-facing reference.

The repository revision reviewed was **abf0d6c55661ab5097691dda26416205331d9636**. This pins the public files reviewed for this inventory; it does not claim that this later published revision is byte-identical to every experimental sketch. The repository contains the illustrated PDF build guide, Arduino firmware, representative .sr captures, and license files. Follow the repository license terms for code and documentation.

Public links identify manufacturer documentation, upstream software, or the project itself. Unknown listings and versions remain explicitly unknown. Product pages establish identity and specifications; they do not validate this research assembly. The inventory should be updated when the missing identifiers are recovered. If a build-critical detail is missing or unclear, contact the author through the project repository; additional historical material may be available even where exploratory files were not organized for release.
