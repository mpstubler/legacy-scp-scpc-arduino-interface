// ZPR 9909 A / CON2: native command decode and relay-gated DATA replay
// Arduino Uno R4 Minima | Serial Monitor demonstration firmware
//
// READ THIS FIRST
// This is experimental firmware for permanently decommissioned equipment and
// benchtop research, not clinical or patient-care use. The relay's deenergized
// hardware path restores native DATA; it does not stop the pump. Do not reset or
// upload while relying on synthetic DATA with the pump powered.
//
// WHAT THE INTERFACE DOES
// The pump supplies CLOCK, FRAME and native DATA. This sketch listens to those
// signals, reconstructs the 16-bit command (CMD16), and drives replacement DATA
// on D7. It does not generate CLOCK or FRAME and does not measure flow or RPM.
// In the documented external circuit, the relay selects the native bypass or
// the buffered interface. With the interface selected, external FRAME-controlled
// buffer logic passes native DATA while FRAME is high and replay DATA while
// FRAME is low. The sketch itself does not implement that electrical selection.
// Use the build guide for CON2 contacts, HC125, inverter, relay and power wiring;
// Arduino D-pin numbers below are not CON2 contact numbers.
//
// TWO JOBS RUN AT DIFFERENT SPEEDS
// 1. Interrupt handlers track FRAME boundaries and falling CLOCK edges. Native
//    DATA is decoded; a previously accepted command or synthetic command is
//    shifted out most-significant bit first using the native timing.
// 2. loop() polls native DATA transitions, samples pressure, handles serial
//    commands and slowly walks the synthetic command toward its target.
//    DATA history is polled, not captured by a DATA-pin interrupt.
//
// SERIAL MONITOR: 115200 baud, Newline or Both NL & CR line ending
//   P or p + Enter : capture current pressure and command; start pressure mode.
//   r + Enter      : capture current command; start manual offset mode.
//   50 / -10 / 0   : manual offset from that captured baseline, then Enter.
//   blank Enter/n  : return to native control and clear any latched hard error.
// Offsets are absolute relative to the captured baseline, not cumulative jogs.
// Only P/p accepts both cases; r and n are lowercase. Carriage return is ignored.
//
// THREE COMMAND VALUES
//   synth_base_command   : accepted native command captured when a mode starts.
//   synth_target_command : requested destination after limits are applied.
//   synth_command        : current value used by the replay bit-output helper.
// The baseline stays fixed until a mode is restarted. Native DATA monitoring
// continues while synthetic control is active.
//
// LIMITS AND UNITS
// Manual requests are clamped to +/-65 CMD16 counts. In pressure mode, a clamped
// target at +/-65 counts trips a latched error and releases the relay BEFORE
// that target is assigned. Both modes also use the absolute 0x0000..0x0700 range.
// These are command limits, not measured RPM, pressure or flow limits.
// The observed ~3 RPM change per CMD16 count is load dependent and uncalibrated;
// it is used only for the manual report, never as speed feedback.
// Pressure is a filtered 10-bit ADC reading, not calibrated pressure units.
//
// OUTPUT
// Manual entry prints the baseline, requested command, delta and RPM estimate.
// Both modes use compact +/- markers while the synthetic command moves. A line
// prints only when its signed display bucket changes, not on every CMD16 change.
// Markers are visual offset buckets; the hexadecimal suffix is the command.
// The pressure-limit error repeats until blank Enter or n clears it. Clearing
// does not reengage the relay or restart either mode.
//
// COMMENTING PASS
// Executable statements, constants, pin assignments and serial strings were
// preserved. Nonbreaking spaces introduced by the text paste were converted to
// ordinary spaces. Comments explain this implementation, including its limits;
// names such as "safe" are not a claim of independent safety validation.

// ------------------------------------------------------------
// Pin mapping
// ------------------------------------------------------------

// D2: native CLOCK input; FALLING interrupt clocks decode and replay advance.
// D3: native FRAME input; LOW means the command window is active.
// D4: native DATA upstream of substitution; remains a monitor in both modes.
// No internal pullups are enabled by setup(); electrical levels come from the
// documented interface. This sketch contains no TACH input or speed feedback.
const byte CLOCK_PIN = 2;
const byte FRAME_PIN = 3;
const byte DATA_PIN  = 4;

// D6: logic-analyzer/debug output showing the bit selected by lookback decode.
// D7: replay DATA output into the external buffer, not a complete bus driver.
// D8: relay-module control, HIGH requests interface selection, LOW native bypass.
// Reading D8 later checks the commanded GPIO level, not relay contact feedback.
const byte LIVE_DEBUG_PIN = 6;
const byte REPLAY_PIN     = 7;
const byte RELAY_ENABLE_PIN = 8;

// A0: analog sensor input; setup() requests 10-bit ADC readings (0..1023).
// Sensor scaling, offset calibration and wiring are outside this sketch.
const byte PRESSURE_PIN = A0;


// ------------------------------------------------------------
// Timing / tuning
// ------------------------------------------------------------

// For each CLOCK interrupt, reconstruct the DATA level observed about 10 us
// before the micros() timestamp taken inside the handler. This is a history
// lookup, not a 10-us delay. Interrupt latency and loop polling affect accuracy.
// The ring holds eight observed states, not eight uniformly spaced samples.
const unsigned long LOOKBACK_US = 10;
const byte HISTORY_SIZE = 8;


// ------------------------------------------------------------
// Replay safety filter
// ------------------------------------------------------------

// Only 16-bit frames in the inclusive command range can become replay frames.
// The 0x0200 (512-count) step filter is applied ONLY while D8 reads HIGH and a
// previous accepted frame exists. With D8 LOW, valid-range native commands can
// resynchronize replay immediately, even after a larger command change.
// These decode-acceptance limits are distinct from the +/-65 synthetic limit.
const unsigned long SAFE_REPLAY_BIT_COUNT = 16;

const uint32_t SAFE_CMD16_MIN = 0x0000;
const uint32_t SAFE_CMD16_MAX = 0x0700;

const uint32_t MAX_SAFE_CMD16_STEP = 0x0200;


// ------------------------------------------------------------
// Relay arming / enable lock
// ------------------------------------------------------------

// Arming requires >=25 frames processed by loop() and >=10 frames accepted in
// the FRAME ISR, plus the checks in relayMayEnable(). These counts accumulate
// since startup; they are not consecutive-good-frame requirements or timeouts.
// 0x0020 means 32 decimal counts of allowed native/stored replay disagreement.
const unsigned long ARM_AFTER_TOTAL_FRAMES = 25;
const unsigned long ARM_AFTER_SAFE_ACCEPTS = 10;

// Native decoded command must be close to replay command before relay can engage.
const uint32_t MAX_RELAY_ENABLE_DELTA = 0x0020;


// ------------------------------------------------------------
// Synthetic command target control
// ------------------------------------------------------------

// Shared state: interrupt handlers read these values while loop() updates them.
// volatile makes accesses visible to the compiler; it does not make a sequence
// of reads/writes atomic. Existing interrupt guards and update order are retained.
// When override is false, replay uses the accepted native frame. When true, the
// output helper reads synth_command for each bit; it is not latched per frame.
volatile bool synth_override_enabled = false;

// Keep actual output, captured baseline and requested destination separate.
// The target can change before the current output has finished walking to it.
volatile uint32_t synth_command = 0;
volatile uint32_t synth_base_command = 0;
volatile uint32_t synth_target_command = 0;

unsigned long last_synth_step_ms = 0;

volatile int current_synth_step_size = 1;

// Command walking is due at most once per 50 ms and is attempted outside FRAME.
// Manual mode starts at one count per step; pressure mode chooses 1/2/4/6/8.
// These are cooperative loop intervals, not hard real-time scheduler guarantees.
const int SYNTH_MAX_STEP_SIZE = 8;
const unsigned long SYNTH_STEP_INTERVAL_MS = 50;

// +70/-70 tested clean, +75 alarmed. Use +/-65 margin.
// The test note above records observations from the development setup, not a
// universal pump specification. Manual mode can hold a clamped +/-65 target;
// pressure mode instead releases the relay when its clamped target reaches it.
const int SYNTH_MAX_DELTA_FROM_BASE = 65;

const uint32_t SYNTH_MIN = 0x0000;
const uint32_t SYNTH_MAX = 0x0700;

// Newline-terminated command buffer. Input beyond 16 characters clears the
// buffer; the remainder of that line is still collected by the existing parser.
String serial_line = "";

// Last compact command marker printed.
// Used so serial output only prints when the visual +/- bucket changes.
int last_printed_command_marker_bucket = 0;

// ------------------------------------------------------------
// Pressure control
// ------------------------------------------------------------

// Pressure mode adds target generation on top of synthetic DATA replay.
// hard_error_latched blocks mode entry until an explicit blank line or n.
// Here, the hard-error trigger is the pressure correction bound; it is not a
// general watchdog, disconnected-sensor detector or communication-loss shutdown.
volatile bool pressure_control_enabled = false;
volatile bool hard_error_latched = false;

unsigned long last_error_print_ms = 0;
const unsigned long ERROR_PRINT_INTERVAL_MS = 500;

// All three pressure values are ADC counts. The target is captured at P/p;
// it is not a pressure value typed by the operator and does not track later
// changes made with the native panel.
float pressure_filtered_raw = 0.0;
float pressure_target_raw = 0.0;
int pressure_raw = 0;

unsigned long last_pressure_sample_ms = 0;
unsigned long last_pressure_control_ms = 0;

const unsigned long PRESSURE_SAMPLE_INTERVAL_MS = 20;
const unsigned long PRESSURE_CONTROL_INTERVAL_MS = 50;

// Exponential smoothing: filtered = 0.45*new + 0.55*previous.
// An error within +/-2 filtered ADC counts stops further command walking.
const float PRESSURE_FILTER_ALPHA = 0.45;
const float PRESSURE_DEADBAND_RAW = 2.0;

// If pressure response runs away backward, change this to -1.
// With +1, pressure below target requests a higher command; pressure above
// target requests a lower command. This assumes that response sign for the
// sensor/circuit being tested. The code does not identify or verify the sign.
const int PRESSURE_CONTROL_DIRECTION = 1;


// ------------------------------------------------------------
// Estimated RPM change display
// ------------------------------------------------------------

// Display estimate only.
// We are not claiming absolute RPM from CMD16.
// We only estimate change as requested_delta * 3.
// Author's bench observation: approximately 3 RPM of displayed speed change
// per command count, varying with hydraulic load. No calibrated speed conversion
// or RPM limit is implemented; the original serial labels are left unchanged.
const float EST_RPM_CHANGE_PER_CMD16 = 3.0;


// ------------------------------------------------------------
// Frame state
// ------------------------------------------------------------

// FRAME ISR owns window start/end. CLOCK ISR shifts sampled bits into live_bits.
// completed_* is a one-slot mailbox to loop(): if occupied, a newly completed
// frame is dropped from that mailbox and counted. ISR replay acceptance still
// runs for that frame even when the reporting mailbox cannot accept it.
volatile bool frame_active = false;

volatile uint32_t live_bits = 0;
volatile unsigned long live_bit_count = 0;

volatile bool completed_frame_ready = false;
volatile uint32_t completed_bits = 0;
volatile unsigned long completed_bit_count = 0;
volatile unsigned long dropped_completed_frames = 0;


// ------------------------------------------------------------
// Last known-good replay frame
// ------------------------------------------------------------

// replay_frame_* stores the latest accepted native command for future windows.
// active_replay_* snapshots its bits/length at FRAME LOW and tracks output index.
// "Known-good" here means it passed this sketch's filters, not external proof.
// Synthetic override substitutes its command value but uses the active length.
volatile bool have_replay_frame = false;
volatile uint32_t replay_frame_bits = 0;
volatile unsigned long replay_frame_bit_count = 0;

volatile uint32_t active_replay_bits = 0;
volatile unsigned long active_replay_bit_count = 0;
volatile unsigned long active_replay_index = 0;
volatile bool active_replay_valid = false;


// ------------------------------------------------------------
// D4 transition history
// ------------------------------------------------------------

// Parallel ring arrays: observed transition time and the DATA state after it.
// FRAME LOW seeds entry zero with the current level; loop() adds observed changes.
// History resets every frame. Missed transitions cannot be recovered afterward.
volatile unsigned long history_time_us[HISTORY_SIZE];
volatile byte history_state[HISTORY_SIZE];

volatile byte history_head = 0;
volatile byte history_count = 0;

volatile byte last_polled_d4 = LOW;


// ------------------------------------------------------------
// Counters
// ------------------------------------------------------------

// Diagnostics are retained in memory; this build has no periodic counter dump.
// total_frames/count_* describe consumed mailbox frames. safe_replay_* counts
// are updated in the ISR, so these totals need not agree when the mailbox drops
// frames. last_decoded_* is the latest consumed frame, even if replay rejected it.
volatile unsigned long total_frames = 0;
volatile unsigned long count_16 = 0;
volatile unsigned long count_24 = 0;
volatile unsigned long count_32 = 0;
volatile unsigned long count_other = 0;

volatile unsigned long uncertain_samples = 0;
volatile unsigned long clock_edges_seen = 0;
volatile unsigned long d4_transitions_seen = 0;
volatile unsigned long replay_frames_started = 0;

volatile unsigned long safe_replay_accepts = 0;
volatile unsigned long safe_replay_rejects = 0;
volatile unsigned long safe_replay_step_rejects = 0;
volatile unsigned long relay_enable_blocks = 0;

volatile uint32_t last_decoded_bits = 0;
volatile unsigned long last_decoded_count = 0;


// ------------------------------------------------------------
// Small helpers
// ------------------------------------------------------------

// Unsigned absolute difference without subtracting in the underflow direction.
// Used for replay acceptance and relay-entry command-agreement checks.
uint32_t absoluteDelta(uint32_t a, uint32_t b) {
 if (a >= b) {
   return a - b;
 }

 return b - a;
}


// Magnitude of pressure error in ADC counts; sign is handled separately below.
float absoluteFloat(float value) {
 if (value < 0.0) {
   return -value;
 }

 return value;
}


// Manual-report arithmetic only. No tachometer sample or measured speed is used.
float estimateRPMChangeFromDelta(int delta_cmd16) {
 return (float)delta_cmd16 * EST_RPM_CHANGE_PER_CMD16;
}


// Request the external relay's deenergized native-DATA path. This is not a zero
// command, motor power cut or stop instruction. Hardware wiring must supply the
// bypass behavior; the sketch cannot verify relay motion or contact continuity.
void forceRelayOff() {
 digitalWrite(RELAY_ENABLE_PIN, LOW);
}


// Mirror the reconstructed input bit on an output (used for D6 debug).
void writeBitToPin(byte pin, byte bit_value) {
 if (bit_value == HIGH) {
   digitalWrite(pin, HIGH);
 } else {
   digitalWrite(pin, LOW);
 }
}


// ------------------------------------------------------------
// Pressure-mode compact command output
// ------------------------------------------------------------

// Compact display shared by MANUAL and PRESSURE modes despite the function name.
// delta=0 maps to bucket 0; otherwise bucket magnitude is 1 + floor(|delta|/5).
// For example +1..+4 share bucket +1, +5..+9 share bucket +2. Suppress output
// when the signed bucket is unchanged, even if the command value has changed.
// Displayed signs are capped at 11; the stored bucket itself is not capped.
// The suffix is the current command in hex, not pressure or measured speed.
// At baseline the prefix is the literal "0" followed by "0x" and the hex value.
void printPressureCommandMarker(uint32_t cmd16_value) {

 int delta_from_base =
   (int32_t)cmd16_value - (int32_t)synth_base_command;

 int bucket = 0;

 if (delta_from_base != 0) {

   int abs_delta = delta_from_base;

   if (abs_delta < 0) {
     abs_delta = -abs_delta;
   }

   bucket = 1 + (abs_delta / 5);

   if (delta_from_base < 0) {
     bucket = -bucket;
   }
 }

 if (bucket == last_printed_command_marker_bucket) {
   return;
 }

 last_printed_command_marker_bucket = bucket;

 if (bucket == 0) {
   Serial.print("0");
 } else {

   char marker = '+';

   if (bucket < 0) {
     marker = '-';
   }

   int marker_count = bucket;

   if (marker_count < 0) {
     marker_count = -marker_count;
   }

   if (marker_count > 11) {
     marker_count = 11;
   }

   for (int i = 0; i < marker_count; i++) {
     Serial.print(marker);
   }
 }

 Serial.print("0x");
 Serial.println(cmd16_value, HEX);
}

// ------------------------------------------------------------
// Manual command report
// ------------------------------------------------------------

// Report the captured baseline and clamped requested TARGET, not necessarily the
// command currently being transmitted while it ramps. The native_cmd16 argument
// is synth_base_command here, not a fresh reading from the native panel.
void printManualCommandReport(uint32_t native_cmd16,
                             uint32_t requested_cmd16) {

 int delta_cmd16 =
   (int32_t)requested_cmd16 - (int32_t)native_cmd16;

 float estimated_rpm_change =
   estimateRPMChangeFromDelta(delta_cmd16);

 Serial.println();

 Serial.print("Native pump CMD16: 0x");
 Serial.println(native_cmd16, HEX);

 Serial.print("Requested CMD16: 0x");
 Serial.println(requested_cmd16, HEX);

 Serial.print("Requested CMD16 delta: ");
 if (delta_cmd16 >= 0) {
   Serial.print("+");
 }
 Serial.println(delta_cmd16);

 Serial.print("Estimated RPM change: ");
 if (estimated_rpm_change >= 0) {
   Serial.print("+");
 }
 Serial.println(estimated_rpm_change, 0);

 Serial.println();
}


// ------------------------------------------------------------
// Relay enable permission check
// ------------------------------------------------------------

// Snapshot related ISR/loop state with interrupts briefly disabled, then check
// startup counts, available replay, both lengths and native/replay agreement.
// Called when entering a mode, not continuously while a mode is running.
// There is no age check for the snapshot and no consecutive-accept requirement.
// The latest decoded mailbox value and accepted replay value can come from
// different frames; the 32-count tolerance checks their values, not timestamps.
bool relayMayEnable() {
 uint32_t local_last_bits;
 unsigned long local_last_count;
 uint32_t local_replay_bits;
 unsigned long local_replay_count;
 unsigned long local_total_frames;
 unsigned long local_safe_accepts;
 bool local_have_replay;

 noInterrupts();

 local_last_bits = last_decoded_bits;
 local_last_count = last_decoded_count;
 local_replay_bits = replay_frame_bits;
 local_replay_count = replay_frame_bit_count;
 local_total_frames = total_frames;
 local_safe_accepts = safe_replay_accepts;
 local_have_replay = have_replay_frame;

 interrupts();

 if (local_total_frames < ARM_AFTER_TOTAL_FRAMES) {
   return false;
 }

 if (local_safe_accepts < ARM_AFTER_SAFE_ACCEPTS) {
   return false;
 }

 if (local_have_replay == false) {
   return false;
 }

 if (local_last_count != SAFE_REPLAY_BIT_COUNT) {
   return false;
 }

 if (local_replay_count != SAFE_REPLAY_BIT_COUNT) {
   return false;
 }

 if (absoluteDelta(local_last_bits, local_replay_bits) > MAX_RELAY_ENABLE_DELTA) {
   return false;
 }

 return true;
}


// ------------------------------------------------------------
// Store a D4 transition
// ------------------------------------------------------------

// Append an observed DATA state to the circular history, overwriting the oldest
// entry once full. Called by loop(), not by a DATA edge interrupt. The history
// is shared with the CLOCK and FRAME ISRs without a critical section here;
// these writes are preserved as supplied, not claimed to be an atomic snapshot.
void storeD4State(byte state_value, unsigned long time_us) {
 byte next_head = history_head + 1;

 if (next_head >= HISTORY_SIZE) {
   next_head = 0;
 }

 history_time_us[next_head] = time_us;
 history_state[next_head] = state_value;

 history_head = next_head;

 if (history_count < HISTORY_SIZE) {
   history_count++;
 }

 d4_transitions_seen++;
}


// ------------------------------------------------------------
// Find D4 state at target time
// ------------------------------------------------------------

// Search newest-to-oldest for an observed state timestamp at or before the
// requested instant. Signed subtraction supports nearby times across micros()
// wraparound on this target; it assumes differences fit the signed time range.
// No history: count uncertainty and return LOW. No qualifying entry: count
// uncertainty and return history_state[idx] after the scan. That fallback is
// not a verified historical sample (with a full ring, idx wraps to the head).
// An uncertain sample still enters decoding; this counter does not reject it.
byte getD4StateAt(unsigned long target_time_us) {
 byte local_count = history_count;

 if (local_count == 0) {
   uncertain_samples++;
   return LOW;
 }

 byte idx = history_head;

 for (byte n = 0; n < local_count; n++) {
   unsigned long transition_time = history_time_us[idx];

   if ((long)(target_time_us - transition_time) >= 0) {
     return history_state[idx];
   }

   if (idx == 0) {
     idx = HISTORY_SIZE - 1;
   } else {
     idx--;
   }
 }

 uncertain_samples++;
 return history_state[idx];
}


// ------------------------------------------------------------
// Replay output helpers
// ------------------------------------------------------------

// Set D7 for the current bit index, MSB first. No valid frame or an exhausted
// index drives LOW. With synthetic override enabled, fetch synth_command now;
// otherwise use the accepted native bits captured at this FRAME start.
// This helper prepares a level; the native CLOCK provides the actual timing.
void outputReplayBitNow() {

 if (active_replay_valid == false) {
   digitalWrite(REPLAY_PIN, LOW);
   return;
 }

 if (active_replay_index >= active_replay_bit_count) {
   digitalWrite(REPLAY_PIN, LOW);
   return;
 }

 uint32_t bits_to_output = active_replay_bits;

 if (synth_override_enabled == true) {
   bits_to_output = synth_command;
 }

 unsigned long shift_amount =
   active_replay_bit_count - 1 - active_replay_index;

 uint32_t mask = 1UL << shift_amount;

 if ((bits_to_output & mask) != 0) {
   digitalWrite(REPLAY_PIN, HIGH);
 } else {
   digitalWrite(REPLAY_PIN, LOW);
 }
}


// After a falling CLOCK edge, advance to the next output bit. The first bit was
// already placed on D7 at FRAME LOW. Advancing past the final bit drives LOW.
void advanceReplayForNextClock() {
 if (active_replay_valid == false) {
   digitalWrite(REPLAY_PIN, LOW);
   return;
 }

 active_replay_index++;
 outputReplayBitNow();
}


// ------------------------------------------------------------
// Replay safety check
// ------------------------------------------------------------

// Validate native decode for reuse. Width/range checks always apply. The first
// valid frame is accepted directly; with the relay commanded OFF, later valid
// frames also bypass the step test to follow native panel changes promptly.
// Only when D8 is HIGH is a >0x0200 change from the stored replay rejected.
// Rejection keeps the old replay available; it does not release the relay or
// latch a hard error. This filter has no checksum or communication-age test.
bool decodedFrameIsSafeForReplay(uint32_t candidate_bits,
                                unsigned long candidate_bit_count) {
 if (candidate_bit_count != SAFE_REPLAY_BIT_COUNT) {
   return false;
 }

 if (candidate_bits < SAFE_CMD16_MIN) {
   return false;
 }

 if (candidate_bits > SAFE_CMD16_MAX) {
   return false;
 }

 if (have_replay_frame == false) {
   return true;
 }

 // When relay is OFF, native DATA is still driving the pump.
 // In this state, use native decode to keep replay synchronized.
 if (digitalRead(RELAY_ENABLE_PIN) == LOW) {
   return true;
 }

 uint32_t delta = absoluteDelta(candidate_bits, replay_frame_bits);

 if (delta > MAX_SAFE_CMD16_STEP) {
   safe_replay_step_rejects++;
   return false;
 }

 return true;
}


// ------------------------------------------------------------
// CLOCK falling-edge ISR
// ------------------------------------------------------------

// CLOCK ISR: ignore edges outside FRAME LOW. For an active window, recover the
// earlier native DATA state from history, shift it into the command, mirror it
// on D6, then prepare the next replay bit on D7. No Serial printing in this ISR.
// The accumulator retains at most the first 32 bits; the bit count continues
// past 32 so overlength frames can still be rejected by the 16-bit filter.
void onClockFallingEdge() {
 if (frame_active == false) {
   return;
 }

 clock_edges_seen++;

 unsigned long clock_time_us = micros();
 unsigned long target_time_us = clock_time_us - LOOKBACK_US;

 byte sampled_bit = getD4StateAt(target_time_us);

 if (live_bit_count < 32) {
   live_bits = live_bits << 1;

   if (sampled_bit == HIGH) {
     live_bits = live_bits | 1;
   }
 }

 live_bit_count++;

 writeBitToPin(LIVE_DEBUG_PIN, sampled_bit);

 advanceReplayForNextClock();
}


// ------------------------------------------------------------
// FRAME change ISR
// ------------------------------------------------------------

// FRAME ISR handles both edges:
//   LOW  -> reset native decode/history; snapshot accepted replay and preload
//           its first bit (or drive LOW if none has been accepted).
//   HIGH -> offer the decoded frame to loop()'s mailbox; independently evaluate
//           it for future replay; invalidate active output and drive D6/D7 LOW.
// External hardware selects native DATA outside the active FRAME window.
// Acceptance happens here, before loop() processes the diagnostic mailbox.
void onFrameChange() {
 byte frame_state = digitalRead(FRAME_PIN);

 if (frame_state == LOW) {
   frame_active = true;

   live_bits = 0;
   live_bit_count = 0;

   byte d4_now = digitalRead(DATA_PIN);
   unsigned long now_us = micros();

   history_time_us[0] = now_us;
   history_state[0] = d4_now;
   history_head = 0;
   history_count = 1;

   last_polled_d4 = d4_now;

   if (have_replay_frame == true) {
     active_replay_bits = replay_frame_bits;
     active_replay_bit_count = replay_frame_bit_count;
     active_replay_index = 0;
     active_replay_valid = true;
     replay_frames_started++;

     outputReplayBitNow();

   } else {
     active_replay_bits = 0;
     active_replay_bit_count = 0;
     active_replay_index = 0;
     active_replay_valid = false;

     digitalWrite(REPLAY_PIN, LOW);
   }

   digitalWrite(LIVE_DEBUG_PIN, LOW);

 } else {
   frame_active = false;

   if (completed_frame_ready == false) {
     completed_bits = live_bits;
     completed_bit_count = live_bit_count;
     completed_frame_ready = true;
   } else {
     dropped_completed_frames++;
   }

   if (decodedFrameIsSafeForReplay(live_bits, live_bit_count) == true) {
     replay_frame_bits = live_bits;
     replay_frame_bit_count = live_bit_count;
     have_replay_frame = true;
     safe_replay_accepts++;
   } else {
     safe_replay_rejects++;
   }

   active_replay_valid = false;

   digitalWrite(LIVE_DEBUG_PIN, LOW);
   digitalWrite(REPLAY_PIN, LOW);
 }
}


// ------------------------------------------------------------
// Process completed frame in loop
// ------------------------------------------------------------

// Consume the one-slot ISR mailbox with a short interrupt-protected copy/clear.
// Update frame counts and last decoded value in loop context. This does not
// decide replay acceptance; the FRAME ISR already made that decision.
void processCompletedFrame() {
 if (completed_frame_ready == false) {
   return;
 }

 uint32_t local_bits;
 unsigned long local_count;

 noInterrupts();

 local_bits = completed_bits;
 local_count = completed_bit_count;
 completed_frame_ready = false;

 interrupts();

 total_frames++;

 last_decoded_bits = local_bits;
 last_decoded_count = local_count;

 if (local_count == 16) {
   count_16++;
 } else if (local_count == 24) {
   count_24++;
 } else if (local_count == 32) {
   count_32++;
 } else {
   count_other++;
 }
}


// ------------------------------------------------------------
// Pressure sensor sampling
// ------------------------------------------------------------

// Cooperative sampler: when >=20 ms elapsed, take one A0 reading and smooth it.
// Missed intervals are not replayed. A filtered value <=0 initializes directly
// from the reading; this condition can also recur after zero readings.
// Sampling runs in native/manual modes too, keeping a recent value for P/p.
// Calling this function does not force a fresh sample if its interval is not due.
void samplePressureSensor() {

 if (millis() - last_pressure_sample_ms < PRESSURE_SAMPLE_INTERVAL_MS) {
   return;
 }

 last_pressure_sample_ms = millis();

 pressure_raw = analogRead(PRESSURE_PIN);

 if (pressure_filtered_raw <= 0.0) {
   pressure_filtered_raw = (float)pressure_raw;
   return;
 }

 pressure_filtered_raw =
   pressure_filtered_raw +
   PRESSURE_FILTER_ALPHA * ((float)pressure_raw - pressure_filtered_raw);
}


// ------------------------------------------------------------
// Clamp command target around baseline
// ------------------------------------------------------------

// Intersect [baseline-65, baseline+65] with [0x0000,0x0700], then clamp the
// requested target into that interval. Signed arithmetic prevents subtraction
// near zero from wrapping to a huge unsigned command. This helper only clamps;
// pressure-mode error handling is a separate check after it returns.
uint32_t clampTargetAroundBaseline(int32_t requested_target) {

 int32_t lower_limit = (int32_t)synth_base_command - SYNTH_MAX_DELTA_FROM_BASE;
 int32_t upper_limit = (int32_t)synth_base_command + SYNTH_MAX_DELTA_FROM_BASE;

 if (lower_limit < (int32_t)SYNTH_MIN) {
   lower_limit = SYNTH_MIN;
 }

 if (upper_limit > (int32_t)SYNTH_MAX) {
   upper_limit = SYNTH_MAX;
 }

 if (requested_target < lower_limit) {
   requested_target = lower_limit;
 }

 if (requested_target > upper_limit) {
   requested_target = upper_limit;
 }

 return (uint32_t)requested_target;
}


// ------------------------------------------------------------
// Hard error handling
// ------------------------------------------------------------

// Pressure correction limit response: request native bypass first, then disable
// both control modes, clear synthetic state and latch the error. The printed
// "Max safe RPM change reached!" is legacy wording: the trigger is a CMD16
// target bound, not measured RPM. Native decoding and sensor sampling continue.
void latchHardError() {

 forceRelayOff();

 synth_override_enabled = false;
 pressure_control_enabled = false;

 synth_command = 0;
 synth_base_command = 0;
 synth_target_command = 0;
 current_synth_step_size = 1;
 last_printed_command_marker_bucket = 0;

 Serial.println("Return to Native Control");

 hard_error_latched = true;

 Serial.println("!!!---ERROR---!!!---ERROR---!!!---ERROR---!!!");
 Serial.println();
 Serial.println("Max safe RPM change reached!");
}


// While latched, repeat the error banner at most once per 500 ms from loop().
// This is status output only; latchHardError() already requested relay OFF.
void updateHardErrorPrint() {

 if (hard_error_latched == false) {
   return;
 }

 if (millis() - last_error_print_ms < ERROR_PRINT_INTERVAL_MS) {
   return;
 }

 last_error_print_ms = millis();

 Serial.println("!!!---ERROR---!!!---ERROR---!!!---ERROR---!!!");
}


// ------------------------------------------------------------
// Stop all synthetic / pressure control
// ------------------------------------------------------------

// Common return-to-native operation. Release relay first, then clear synthetic
// and pressure state. This function does NOT clear hard_error_latched itself;
// the blank-line/n branches explicitly do that after calling it.
void stopControlAndRelay() {

 forceRelayOff();

 synth_override_enabled = false;
 pressure_control_enabled = false;

 synth_command = 0;
 synth_base_command = 0;
 synth_target_command = 0;
 current_synth_step_size = 1;

 Serial.println("Return to Native Control");
}


// ------------------------------------------------------------
// Start pressure control
// ------------------------------------------------------------

// Refuse a latched error or failed relay-entry checks. Ask the sampler to update
// if due, energize the relay, capture the accepted command as a fixed baseline,
// then capture filtered pressure as the fixed target. Start with zero offset.
// This preserves the supplied order: relay HIGH precedes state initialization;
// the whole transition is not wrapped in an interrupt-disabled critical section.
// Reissuing P/p repeats this capture. It does not accept a numerical setpoint.
void startPressureControl() {

 if (hard_error_latched == true) {
   Serial.println("START BLOCKED - HARD ERROR LATCHED");
   return;
 }

 if (relayMayEnable() == false) {
   stopControlAndRelay();
   relay_enable_blocks++;
   Serial.println("PRESSURE CONTROL BLOCKED - RELAY NOT READY");
   return;
 }

 samplePressureSensor();

 digitalWrite(RELAY_ENABLE_PIN, HIGH);

 synth_base_command = replay_frame_bits;
 synth_command = synth_base_command;
 synth_target_command = synth_base_command;
 current_synth_step_size = 1;

 pressure_target_raw = pressure_filtered_raw;

 synth_override_enabled = true;
 pressure_control_enabled = true;

 last_synth_step_ms = millis();
 last_pressure_control_ms = millis();
 last_printed_command_marker_bucket = 0;

 Serial.println("Pressure Mode On");
}


// ------------------------------------------------------------
// Pressure controller
// ------------------------------------------------------------

// Generate a moving command target from pressure error, at most every 50 ms.
// This is an incremental, error-banded controller, not a conventional PID.
// Positive error means pressure is below target; direction selects command sign.
//
// Magnitude in filtered ADC counts -> command step:
//   <=2: hold current command; >2..3: 1; >3..5: 2; >5..8: 4;
//   >8..12: 6; >12: 8. The comparisons below are strictly greater-than.
//
// Add the step to the PREVIOUS TARGET, not to the current output; clamp it;
// then trip if its offset reaches +/-65. Output walking occurs separately in
// updateSyntheticTarget(). Near an absolute endpoint, clamping may stop progress
// before +/-65 is reached; that absolute endpoint alone does not latch an error.
// No sensor plausibility, calibrated pressure ceiling or response timeout is
// implemented here. This pass documents the existing behavior without adding it.
void updatePressureControl() {

 if (hard_error_latched == true) {
   return;
 }

 if (pressure_control_enabled == false) {
   return;
 }

 if (synth_override_enabled == false) {
   return;
 }

 if (digitalRead(RELAY_ENABLE_PIN) == LOW) {
   pressure_control_enabled = false;
   return;
 }

 if (millis() - last_pressure_control_ms < PRESSURE_CONTROL_INTERVAL_MS) {
   return;
 }

 last_pressure_control_ms = millis();

 float error = pressure_target_raw - pressure_filtered_raw;
 float abs_error = absoluteFloat(error);

 // If pressure is already close enough, stop walking toward any old target.
 if (abs_error <= PRESSURE_DEADBAND_RAW) {
   current_synth_step_size = 1;
   synth_target_command = synth_command;
   return;
 }

 int step_size = 1;

 if (abs_error > 3.0) {
   step_size = 2;
 }

 if (abs_error > 5.0) {
   step_size = 4;
 }

 if (abs_error > 8.0) {
   step_size = 6;
 }

 if (abs_error > 12.0) {
   step_size = 8;
 }

 if (step_size > SYNTH_MAX_STEP_SIZE) {
   step_size = SYNTH_MAX_STEP_SIZE;
 }

 current_synth_step_size = step_size;

 int command_step = 0;

 if (error > PRESSURE_DEADBAND_RAW) {
   command_step = step_size * PRESSURE_CONTROL_DIRECTION;
 }

 if (error < -PRESSURE_DEADBAND_RAW) {
   command_step = -step_size * PRESSURE_CONTROL_DIRECTION;
 }

// Integrate one signed step into the target. Actual output can lag behind if
// FRAME activity or loop work delays the separate command walker.
 int32_t requested_target =
   (int32_t)synth_target_command + command_step;

 uint32_t clamped_target = clampTargetAroundBaseline(requested_target);

 int32_t delta_after_clamp =
   (int32_t)clamped_target - (int32_t)synth_base_command;

// Inclusive limit: reaching +65 or -65 is enough to trip. This tests the clamped
// requested target before assigning it, not a measured/output speed excursion.
 if (delta_after_clamp >= SYNTH_MAX_DELTA_FROM_BASE ||
     delta_after_clamp <= -SYNTH_MAX_DELTA_FROM_BASE) {

   Serial.print("PRESSURE CONTROL LIMIT HIT delta=");
   Serial.println(delta_after_clamp);

   latchHardError();
   return;
 }

 synth_target_command = clamped_target;
}


// ------------------------------------------------------------
// Synthetic command target updater
// ------------------------------------------------------------

// Move the actual synthetic command toward its target by 1..8 counts per due
// 50-ms step, stopping exactly at the target instead of overshooting it.
// The frame_active check defers this normal walker while FRAME is active. It
// is not an atomic frame-boundary lock: an ISR can run after the check, and
// mode-start/stop functions also assign synth_command independently of this gate.
// The original timing and synchronization are preserved in this comment pass.
void updateSyntheticTarget() {

 if (hard_error_latched == true) {
   return;
 }

 if (synth_override_enabled == false) {
   return;
 }

 if (synth_command == synth_target_command) {
   return;
 }

 if (frame_active == true) {
   return;
 }

 if (millis() - last_synth_step_ms < SYNTH_STEP_INTERVAL_MS) {
   return;
 }

 last_synth_step_ms = millis();

 int local_step_size = current_synth_step_size;

 if (local_step_size < 1) {
   local_step_size = 1;
 }

 if (local_step_size > SYNTH_MAX_STEP_SIZE) {
   local_step_size = SYNTH_MAX_STEP_SIZE;
 }

 if (synth_command < synth_target_command) {

   uint32_t remaining = synth_target_command - synth_command;

   if (remaining <= (uint32_t)local_step_size) {
     synth_command = synth_target_command;
   } else {
     synth_command = synth_command + local_step_size;
   }
 }

 else if (synth_command > synth_target_command) {

   uint32_t remaining = synth_command - synth_target_command;

   if (remaining <= (uint32_t)local_step_size) {
     synth_command = synth_target_command;
   } else {
     synth_command = synth_command - local_step_size;
   }
 }

 printPressureCommandMarker(synth_command);
}


// ------------------------------------------------------------
// Serial command handler
// ------------------------------------------------------------

// Consume bytes into a short String until LF. Ignore CR, so use Newline or Both
// NL & CR in Serial Monitor; Carriage return alone does not submit a command.
// trim() permits surrounding whitespace and treats whitespace-only input as stop.
// Each completed line returns from the handler; remaining bytes wait for loop().
// Dispatch order matters: blank/n can clear a latch; other inputs are blocked
// while latched; P/p starts pressure; r starts manual; remaining input is treated
// as a numeric manual delta only if manual synthetic control is active.
// String::toInt() is used without full-line numeric validation: malformed input
// is not reliably rejected (a nonnumeric start yields zero). No parser change
// has been made here. Use explicit signed decimal integers for manual requests.
void handleSerialCommands() {

 while (Serial.available() > 0) {

   char c = Serial.read();

   if (c == '\r') {
     continue;
   }

   if (c == '\n') {

     serial_line.trim();

     // Blank Enter = exit pressure/manual control.
     // If hard error is latched, blank Enter clears it after relay is already off.
     if (serial_line.length() == 0) {

       stopControlAndRelay();

       if (hard_error_latched == true) {
         hard_error_latched = false;
         last_error_print_ms = millis();
         Serial.println("ERROR CLEARED / RELAY OFF");
       }

       serial_line = "";
       return;
     }

     // n also exits / clears hard error.
     if (serial_line == "n") {

       stopControlAndRelay();

       if (hard_error_latched == true) {
         hard_error_latched = false;
         last_error_print_ms = millis();
         Serial.println("ERROR CLEARED / RELAY OFF");
       }

       serial_line = "";
       return;
     }

     if (hard_error_latched == true) {
       Serial.println("COMMAND BLOCKED - HARD ERROR LATCHED / PRESS ENTER OR n TO CLEAR");
       serial_line = "";
       return;
     }

     // Pressure auto-control.
     if (serial_line == "P" || serial_line == "p") {

       startPressureControl();

       serial_line = "";
       return;
     }

     // Relay manual mode.
     if (serial_line == "r") {

       if (relayMayEnable() == true) {

         digitalWrite(RELAY_ENABLE_PIN, HIGH);

         synth_base_command = replay_frame_bits;
         synth_command = synth_base_command;
         synth_target_command = synth_base_command;
         current_synth_step_size = 1;

         synth_override_enabled = true;
         pressure_control_enabled = false;

         last_synth_step_ms = millis();
         last_printed_command_marker_bucket = 0;

         Serial.println("Relay Mode On");

       } else {

         stopControlAndRelay();
         relay_enable_blocks++;
         Serial.println("RELAY BLOCKED");
       }

       serial_line = "";
       return;
     }

     // Numeric manual delta.
     if (pressure_control_enabled == true) {
       Serial.println("MANUAL DELTA BLOCKED - PRESSURE CONTROL ACTIVE");
       serial_line = "";
       return;
     }

     if (synth_override_enabled == true &&
         digitalRead(RELAY_ENABLE_PIN) == HIGH) {

// This delta is relative to the captured baseline on EVERY entry. Two requests
// of 10 both mean baseline+10; they do not produce baseline+20. First clamp the
// delta to +/-65, then clamp the absolute target to the allowed command range.
       int requested_delta = serial_line.toInt();

       if (requested_delta > SYNTH_MAX_DELTA_FROM_BASE) {
         requested_delta = SYNTH_MAX_DELTA_FROM_BASE;
       }

       if (requested_delta < -SYNTH_MAX_DELTA_FROM_BASE) {
         requested_delta = -SYNTH_MAX_DELTA_FROM_BASE;
       }

// Despite its name/printed label, this is the captured baseline, not live DATA.
       uint32_t native_cmd16 = synth_base_command;

       int32_t requested_target =
         (int32_t)synth_base_command + requested_delta;

       synth_target_command = clampTargetAroundBaseline(requested_target);

       printManualCommandReport(native_cmd16, synth_target_command);

     } else {

       Serial.println();
       Serial.println("DELTA BLOCKED - RELAY NOT ON");
       Serial.println();
     }

     serial_line = "";
     return;
   }

   serial_line += c;

   if (serial_line.length() > 16) {
     serial_line = "";
     Serial.println("SERIAL INPUT CLEARED");
   }
 }
}


// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------

// Initialize serial, wait 1000 ms, then configure/request relay OFF and set up
// inputs/outputs. D8 is not configured by this sketch during that initial delay;
// reset/power-up behavior depends on the external circuit, not forceRelayOff().
// Request 10-bit ADC resolution, initialize debug/replay LOW, and attach the
// native timing interrupts. Do not infer a hardware fail-safe guarantee from
// the eventual LOW write. Nothing here automatically enables synthetic control.
void setup() {
 Serial.begin(115200);
 delay(1000);

 pinMode(RELAY_ENABLE_PIN, OUTPUT);
 forceRelayOff();

 pinMode(CLOCK_PIN, INPUT);
 pinMode(FRAME_PIN, INPUT);
 pinMode(DATA_PIN, INPUT);
 pinMode(PRESSURE_PIN, INPUT);

 analogReadResolution(10);

 pinMode(LIVE_DEBUG_PIN, OUTPUT);
 pinMode(REPLAY_PIN, OUTPUT);

 digitalWrite(LIVE_DEBUG_PIN, LOW);
 digitalWrite(REPLAY_PIN, LOW);

 last_polled_d4 = digitalRead(DATA_PIN);

 attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), onClockFallingEdge, FALLING);
 attachInterrupt(digitalPinToInterrupt(FRAME_PIN), onFrameChange, CHANGE);

 Serial.println("Serial Monitor Demo Ready");
 Serial.println("Commands: P = pressure mode, r = relay mode, Enter/n = native control");
}


// ------------------------------------------------------------
// Main loop
// ------------------------------------------------------------

// Preserved work order: serial -> sample pressure -> error display -> pressure
// target -> command walker -> poll DATA transitions -> consume frame mailbox.
// The CLOCK/FRAME interrupts can run during loop work. Native DATA edge history
// depends on how often loop() reaches the polling block; serial/ADC work can
// delay those observations. Counters expose some losses but do not repair them.
// There is no periodic communication watchdog that releases an active relay.
void loop() {

 handleSerialCommands();

 samplePressureSensor();

 updateHardErrorPrint();

 updatePressureControl();

 updateSyntheticTarget();

 if (frame_active == true) {
   byte d4_now = digitalRead(DATA_PIN);

   if (d4_now != last_polled_d4) {
     unsigned long now_us = micros();

     last_polled_d4 = d4_now;
     storeD4State(d4_now, now_us);
   }
 }

 processCompletedFrame();
}