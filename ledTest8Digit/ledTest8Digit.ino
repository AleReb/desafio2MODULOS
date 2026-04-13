#include <Arduino.h>

// ============================================================
// Interactive 4-Digit 7-Segment Debug Driver using 2x 74HC595
// Target: ESP32-S3
// ============================================================

// -------------------------
// Pin definition
// -------------------------
const int DATA_PIN  = 21;
const int LATCH_PIN = 19;
const int CLOCK_PIN = 20;

// -------------------------
// Global flags
// -------------------------
bool serialReady = false;
bool pinsReady = false;
bool displayReady = false;
bool muxEnabled = false;

// -------------------------
// Hardware configuration
// -------------------------
bool commonAnode = true;
bool digitActiveLow = true;
bool segmentByteFirst = false;
bool reverseDigitOrder = false;

// -------------------------
// Manual debug state
// -------------------------
volatile uint8_t displayBuffer[4] = {1, 2, 3, 4};

uint8_t selectedDigit = 0;
uint8_t selectedValue = 8;
uint8_t selectedSegment = 0;

// -------------------------
// Timing
// -------------------------
unsigned long lastMuxMicros = 0;
unsigned long lastManualStepMs = 0;
const unsigned long muxIntervalUs = 2000;
const unsigned long manualStepIntervalMs = 1500;

// -------------------------
// Serial input
// -------------------------
String inputLine = "";

// -------------------------
// Modes
// -------------------------
enum DebugMode {
  MODE_STOP = 0,
  MODE_SHOW_BUFFER,
  MODE_SINGLE_DIGIT,
  MODE_SCAN_DIGITS,
  MODE_SCAN_SEGMENTS,
  MODE_COUNT
};

DebugMode currentMode = MODE_SHOW_BUFFER;

// -------------------------
// Counter
// -------------------------
uint16_t counterValue = 0;

// -------------------------
// Segment maps
// Bit order assumed: DP G F E D C B A
// -------------------------
const uint8_t digitMapCC[10] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

const char* segmentNames[8] = {
  "A", "B", "C", "D", "E", "F", "G", "DP"
};

// ============================================================
// Helpers
// ============================================================

uint8_t adaptSegmentPattern(uint8_t ccPattern) {
  if (commonAnode) {
    return ~ccPattern;
  }
  return ccPattern;
}

uint8_t getBlankSegments() {
  if (commonAnode) {
    return 0xFF;
  }
  return 0x00;
}

uint8_t getAllDigitsOffMask() {
  return digitActiveLow ? 0xFF : 0x00;
}

uint8_t getDigitMaskRaw(uint8_t digitIndex) {
  if (digitIndex > 3) {
    return getAllDigitsOffMask();
  }

  uint8_t logicalIndex = digitIndex;

  if (reverseDigitOrder) {
    logicalIndex = 3 - digitIndex;
  }

  if (digitActiveLow) {
    return (uint8_t)~(1 << logicalIndex);
  } else {
    return (uint8_t)(1 << logicalIndex);
  }
}

uint8_t getDigitPattern(uint8_t value, bool withDot = false) {
  uint8_t ccPattern = 0x00;

  if (value < 10) {
    ccPattern = digitMapCC[value];
  }

  if (withDot) {
    ccPattern |= 0b10000000;
  }

  return adaptSegmentPattern(ccPattern);
}

void writeRegistersRaw(uint8_t segmentByte, uint8_t digitByte) {
  digitalWrite(LATCH_PIN, LOW);

  if (segmentByteFirst) {
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, segmentByte);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, digitByte);
  } else {
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, digitByte);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, segmentByte);
  }

  digitalWrite(LATCH_PIN, HIGH);
}

void allOff() {
  writeRegistersRaw(getBlankSegments(), getAllDigitsOffMask());
}

void showOneDigit(uint8_t digitIndex, uint8_t value, bool withDot = false) {
  uint8_t segmentByte = getDigitPattern(value, withDot);
  uint8_t digitByte = getDigitMaskRaw(digitIndex);

  allOff();
  writeRegistersRaw(segmentByte, digitByte);
}

void showOneSegment(uint8_t digitIndex, uint8_t segmentIndex) {
  if (segmentIndex > 7) {
    return;
  }

  uint8_t ccPattern = (1 << segmentIndex);
  uint8_t segmentByte = adaptSegmentPattern(ccPattern);
  uint8_t digitByte = getDigitMaskRaw(digitIndex);

  allOff();
  writeRegistersRaw(segmentByte, digitByte);
}

void setBufferFromNumber(uint16_t value) {
  displayBuffer[0] = (value / 1000) % 10;
  displayBuffer[1] = (value / 100) % 10;
  displayBuffer[2] = (value / 10) % 10;
  displayBuffer[3] = value % 10;
}

void refreshDisplay() {
  static uint8_t muxDigit = 0;

  uint8_t value = displayBuffer[muxDigit];
  uint8_t segmentByte = getDigitPattern(value, false);
  uint8_t digitByte = getDigitMaskRaw(muxDigit);

  allOff();
  writeRegistersRaw(segmentByte, digitByte);

  muxDigit++;
  if (muxDigit >= 4) {
    muxDigit = 0;
  }
}

void printStatus() {
  Serial.println();
  Serial.println("----- DISPLAY DEBUG STATUS -----");
  Serial.print("Pins ready: ");
  Serial.println(pinsReady ? "YES" : "NO");

  Serial.print("Display ready: ");
  Serial.println(displayReady ? "YES" : "NO");

  Serial.print("Mux enabled: ");
  Serial.println(muxEnabled ? "YES" : "NO");

  Serial.print("Mode: ");
  switch (currentMode) {
    case MODE_STOP: Serial.println("MODE_STOP"); break;
    case MODE_SHOW_BUFFER: Serial.println("MODE_SHOW_BUFFER"); break;
    case MODE_SINGLE_DIGIT: Serial.println("MODE_SINGLE_DIGIT"); break;
    case MODE_SCAN_DIGITS: Serial.println("MODE_SCAN_DIGITS"); break;
    case MODE_SCAN_SEGMENTS: Serial.println("MODE_SCAN_SEGMENTS"); break;
    case MODE_COUNT: Serial.println("MODE_COUNT"); break;
  }

  Serial.print("commonAnode: ");
  Serial.println(commonAnode ? "true" : "false");

  Serial.print("digitActiveLow: ");
  Serial.println(digitActiveLow ? "true" : "false");

  Serial.print("segmentByteFirst: ");
  Serial.println(segmentByteFirst ? "true" : "false");

  Serial.print("reverseDigitOrder: ");
  Serial.println(reverseDigitOrder ? "true" : "false");

  Serial.print("selectedDigit: ");
  Serial.println(selectedDigit);

  Serial.print("selectedValue: ");
  Serial.println(selectedValue);

  Serial.print("selectedSegment: ");
  Serial.print(selectedSegment);
  Serial.print(" (");
  Serial.print(segmentNames[selectedSegment]);
  Serial.println(")");

  Serial.print("Buffer: ");
  Serial.print(displayBuffer[0]);
  Serial.print(displayBuffer[1]);
  Serial.print(displayBuffer[2]);
  Serial.println(displayBuffer[3]);
}

void printHelp() {
  Serial.println();
  Serial.println("Available commands:");
  Serial.println("help");
  Serial.println("status");
  Serial.println("off");
  Serial.println("show1234");
  Serial.println("digit <0-3> <0-9>");
  Serial.println("scan_digits");
  Serial.println("scan_segments");
  Serial.println("count");
  Serial.println("stop");
  Serial.println("set_digit <0-3>");
  Serial.println("set_value <0-9>");
  Serial.println("set_segment <0-7>");
  Serial.println("next_digit");
  Serial.println("next_value");
  Serial.println("next_segment");
  Serial.println("toggle_common");
  Serial.println("toggle_digitpol");
  Serial.println("toggle_byteorder");
  Serial.println("toggle_reverse");
  Serial.println("refresh");
}

void enterMode(DebugMode mode) {
  currentMode = mode;

  if (mode == MODE_SHOW_BUFFER || mode == MODE_COUNT) {
    muxEnabled = true;
  } else {
    muxEnabled = false;
  }

  lastManualStepMs = millis();

  Serial.print("Entered mode: ");
  switch (currentMode) {
    case MODE_STOP: Serial.println("MODE_STOP"); break;
    case MODE_SHOW_BUFFER: Serial.println("MODE_SHOW_BUFFER"); break;
    case MODE_SINGLE_DIGIT: Serial.println("MODE_SINGLE_DIGIT"); break;
    case MODE_SCAN_DIGITS: Serial.println("MODE_SCAN_DIGITS"); break;
    case MODE_SCAN_SEGMENTS: Serial.println("MODE_SCAN_SEGMENTS"); break;
    case MODE_COUNT: Serial.println("MODE_COUNT"); break;
  }
}

void applyImmediateView() {
  if (!displayReady) {
    Serial.println("Display not ready");
    return;
  }

  if (currentMode == MODE_STOP) {
    allOff();
    Serial.println("All digits OFF");
    return;
  }

  if (currentMode == MODE_SINGLE_DIGIT) {
    showOneDigit(selectedDigit, selectedValue, false);
    Serial.print("Showing digit ");
    Serial.print(selectedDigit);
    Serial.print(" value ");
    Serial.println(selectedValue);
    return;
  }

  if (currentMode == MODE_SCAN_SEGMENTS) {
    showOneSegment(selectedDigit, selectedSegment);
    Serial.print("Showing segment ");
    Serial.print(selectedSegment);
    Serial.print(" (");
    Serial.print(segmentNames[selectedSegment]);
    Serial.print(") on digit ");
    Serial.println(selectedDigit);
    return;
  }

  if (currentMode == MODE_SHOW_BUFFER) {
    Serial.println("Buffer mode active, multiplex refresh running");
    return;
  }

  if (currentMode == MODE_SCAN_DIGITS) {
    showOneDigit(selectedDigit, selectedValue, false);
    Serial.print("Scan digits view -> digit ");
    Serial.print(selectedDigit);
    Serial.print(" value ");
    Serial.println(selectedValue);
    return;
  }

  if (currentMode == MODE_COUNT) {
    Serial.println("Count mode active, multiplex refresh running");
    return;
  }
}

void processCommand(String cmd) {
  cmd.trim();

  if (cmd.length() == 0) {
    return;
  }

  if (cmd == "help") {
    printHelp();
    return;
  }

  if (cmd == "status") {
    printStatus();
    return;
  }

  if (cmd == "off") {
    enterMode(MODE_STOP);
    applyImmediateView();
    return;
  }

  if (cmd == "stop") {
    enterMode(MODE_STOP);
    applyImmediateView();
    return;
  }

  if (cmd == "show1234") {
    displayBuffer[0] = 1;
    displayBuffer[1] = 2;
    displayBuffer[2] = 3;
    displayBuffer[3] = 4;
    enterMode(MODE_SHOW_BUFFER);
    applyImmediateView();
    return;
  }

  if (cmd == "scan_digits") {
    enterMode(MODE_SCAN_DIGITS);
    applyImmediateView();
    return;
  }

  if (cmd == "scan_segments") {
    enterMode(MODE_SCAN_SEGMENTS);
    applyImmediateView();
    return;
  }

  if (cmd == "count") {
    counterValue = 0;
    setBufferFromNumber(counterValue);
    enterMode(MODE_COUNT);
    applyImmediateView();
    return;
  }

  if (cmd == "refresh") {
    applyImmediateView();
    return;
  }

  if (cmd == "next_digit") {
    selectedDigit++;
    if (selectedDigit > 3) {
      selectedDigit = 0;
    }
    applyImmediateView();
    return;
  }

  if (cmd == "next_value") {
    selectedValue++;
    if (selectedValue > 9) {
      selectedValue = 0;
    }
    applyImmediateView();
    return;
  }

  if (cmd == "next_segment") {
    selectedSegment++;
    if (selectedSegment > 7) {
      selectedSegment = 0;
    }
    applyImmediateView();
    return;
  }

  if (cmd == "toggle_common") {
    commonAnode = !commonAnode;
    Serial.print("commonAnode = ");
    Serial.println(commonAnode ? "true" : "false");
    applyImmediateView();
    return;
  }

  if (cmd == "toggle_digitpol") {
    digitActiveLow = !digitActiveLow;
    Serial.print("digitActiveLow = ");
    Serial.println(digitActiveLow ? "true" : "false");
    applyImmediateView();
    return;
  }

  if (cmd == "toggle_byteorder") {
    segmentByteFirst = !segmentByteFirst;
    Serial.print("segmentByteFirst = ");
    Serial.println(segmentByteFirst ? "true" : "false");
    applyImmediateView();
    return;
  }

  if (cmd == "toggle_reverse") {
    reverseDigitOrder = !reverseDigitOrder;
    Serial.print("reverseDigitOrder = ");
    Serial.println(reverseDigitOrder ? "true" : "false");
    applyImmediateView();
    return;
  }

  if (cmd.startsWith("set_digit ")) {
    int value = cmd.substring(10).toInt();
    if (value >= 0 && value <= 3) {
      selectedDigit = (uint8_t)value;
      Serial.print("selectedDigit = ");
      Serial.println(selectedDigit);
      applyImmediateView();
    } else {
      Serial.println("Invalid digit, use 0-3");
    }
    return;
  }

  if (cmd.startsWith("set_value ")) {
    int value = cmd.substring(10).toInt();
    if (value >= 0 && value <= 9) {
      selectedValue = (uint8_t)value;
      Serial.print("selectedValue = ");
      Serial.println(selectedValue);
      applyImmediateView();
    } else {
      Serial.println("Invalid value, use 0-9");
    }
    return;
  }

  if (cmd.startsWith("set_segment ")) {
    int value = cmd.substring(12).toInt();
    if (value >= 0 && value <= 7) {
      selectedSegment = (uint8_t)value;
      Serial.print("selectedSegment = ");
      Serial.print(selectedSegment);
      Serial.print(" (");
      Serial.print(segmentNames[selectedSegment]);
      Serial.println(")");
      applyImmediateView();
    } else {
      Serial.println("Invalid segment, use 0-7");
    }
    return;
  }

  if (cmd.startsWith("digit ")) {
    int d = -1;
    int v = -1;
    sscanf(cmd.c_str(), "digit %d %d", &d, &v);

    if (d >= 0 && d <= 3 && v >= 0 && v <= 9) {
      selectedDigit = (uint8_t)d;
      selectedValue = (uint8_t)v;
      enterMode(MODE_SINGLE_DIGIT);
      applyImmediateView();
    } else {
      Serial.println("Invalid format. Use: digit <0-3> <0-9>");
    }
    return;
  }

  Serial.println("Unknown command. Type: help");
}

// ============================================================
// Setup
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(300);
  serialReady = true;

  Serial.println();
  Serial.println("Starting interactive 74HC595 display debug...");

  pinMode(DATA_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  digitalWrite(DATA_PIN, LOW);
  digitalWrite(LATCH_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);

  pinsReady = true;
  displayReady = true;

  allOff();

  Serial.println("Pins initialized");
  Serial.println("Interactive control enabled");
  Serial.println("Type 'help' to see commands");

  enterMode(MODE_SHOW_BUFFER);
  printHelp();
  printStatus();
}

// ============================================================
// Loop
// ============================================================
void loop() {
  unsigned long nowUs = micros();
  unsigned long nowMs = millis();

  // -------------------------
  // Serial input handling
  // -------------------------
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (inputLine.length() > 0) {
        processCommand(inputLine);
        inputLine = "";
      }
    } else {
      inputLine += c;
    }
  }

  // -------------------------
  // Multiplex refresh
  // -------------------------
  if (displayReady && muxEnabled) {
    if (nowUs - lastMuxMicros >= muxIntervalUs) {
      lastMuxMicros = nowUs;
      refreshDisplay();
    }
  }

  // -------------------------
  // Manual slow scan
  // -------------------------
  if (displayReady && !muxEnabled) {
    if (currentMode == MODE_SCAN_DIGITS) {
      if (nowMs - lastManualStepMs >= manualStepIntervalMs) {
        lastManualStepMs = nowMs;

        showOneDigit(selectedDigit, selectedValue, false);

        Serial.print("Digit scan step -> digit ");
        Serial.print(selectedDigit);
        Serial.print(" value ");
        Serial.println(selectedValue);

        selectedDigit++;
        if (selectedDigit > 3) {
          selectedDigit = 0;
        }
      }
    }

    if (currentMode == MODE_SCAN_SEGMENTS) {
      if (nowMs - lastManualStepMs >= manualStepIntervalMs) {
        lastManualStepMs = nowMs;

        showOneSegment(selectedDigit, selectedSegment);

        Serial.print("Segment scan step -> digit ");
        Serial.print(selectedDigit);
        Serial.print(", segment ");
        Serial.print(selectedSegment);
        Serial.print(" (");
        Serial.print(segmentNames[selectedSegment]);
        Serial.println(")");

        selectedSegment++;
        if (selectedSegment > 7) {
          selectedSegment = 0;
        }
      }
    }
  }

  // -------------------------
  // Counter update
  // -------------------------
  if (displayReady && currentMode == MODE_COUNT) {
    static unsigned long lastCounterStepMs = 0;

    if (nowMs - lastCounterStepMs >= 1000) {
      lastCounterStepMs = nowMs;

      setBufferFromNumber(counterValue);

      Serial.print("Counter value: ");
      Serial.println(counterValue);

      counterValue++;
      if (counterValue > 9999) {
        counterValue = 0;
      }
    }
  }
}