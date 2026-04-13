#include <Arduino.h>

// ============================================================
// 8-digit display driver + test sketch
// Hardware: 2x 3461BG + 74HC595
// Board: ESP32-S3
//
// Fixed for this specific display:
// - Common anode
// - Digit active HIGH
// - Segment byte first
// - Physical digit order already corrected
//
// Visual order confirmed:
// left to right = digit 0,1,2,3,4,5,6,7 for fixed patterns
//
// Numeric counter behavior:
// units on the RIGHTMOST visible digit
// tens to its left
// hundreds to its left
// ...
// ============================================================

// -------------------------
// Pin definition
// -------------------------
const int DATA_PIN  = 21;
const int LATCH_PIN = 19;
const int CLOCK_PIN = 20;

// -------------------------
// Global status flags
// -------------------------
bool serialReady = false;
bool pinsReady = false;
bool displayReady = false;
bool muxReady = false;

// -------------------------
// Fixed hardware configuration
// -------------------------
const bool COMMON_ANODE       = true;
const bool DIGIT_ACTIVE_LOW   = false;
const bool SEGMENT_BYTE_FIRST = true;

// -------------------------
// Display configuration
// -------------------------
const uint8_t DISPLAY_DIGITS = 8;

// ------------------------------------------------------------
// Fixed physical digit map for this display
// logical display position -> physical digit select line
//
// This map is already corrected from your real hardware test.
// With setFixed01234567() it should show visually:
// 0 1 2 3 4 5 6 7
// ------------------------------------------------------------
const uint8_t DIGIT_SELECT_MAP[DISPLAY_DIGITS] = {
  4, 5, 6, 7, 0, 1, 2, 3
};

// -------------------------
// Timing
// -------------------------
unsigned long lastMuxMicros = 0;
unsigned long lastStatusMs = 0;
unsigned long lastDemoMs = 0;

const unsigned long muxIntervalUs = 1200;
const unsigned long statusIntervalMs = 3000;
const unsigned long demoStepMs = 600;

// -------------------------
// Display buffer
// segmentBuffer[0] = leftmost visible digit
// segmentBuffer[7] = rightmost visible digit
// -------------------------
volatile uint8_t segmentBuffer[DISPLAY_DIGITS];

// -------------------------
// Demo mode
// -------------------------
enum DemoMode {
  MODE_FIXED_01234567 = 0,
  MODE_FIXED_12345678,
  MODE_ONE_DIGIT_AT_A_TIME,
  MODE_COUNTER_111
};

DemoMode currentMode = MODE_COUNTER_111;

// -------------------------
// Demo state
// -------------------------
uint8_t activeTestDigit = 0;
unsigned long counterValue = 0;

// -------------------------
// Segment map
// Bit order: DP G F E D C B A
// Base map in common-cathode logic
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

const uint8_t blankMapCC = 0b00000000;
const uint8_t dashMapCC  = 0b01000000;

// ============================================================
// Low-level helpers
// ============================================================

uint8_t adaptSegmentPattern(uint8_t ccPattern) {
  if (COMMON_ANODE) {
    return ~ccPattern;
  }
  return ccPattern;
}

uint8_t getBlankSegments() {
  return COMMON_ANODE ? 0xFF : 0x00;
}

uint8_t getAllDigitsOffMask() {
  return DIGIT_ACTIVE_LOW ? 0xFF : 0x00;
}

uint8_t encodeDigit(uint8_t value, bool withDot = false) {
  uint8_t ccPattern = blankMapCC;

  if (value < 10) {
    ccPattern = digitMapCC[value];
  }

  if (withDot) {
    ccPattern |= 0b10000000;
  }

  return adaptSegmentPattern(ccPattern);
}

uint8_t encodeBlank() {
  return adaptSegmentPattern(blankMapCC);
}

uint8_t encodeDash() {
  return adaptSegmentPattern(dashMapCC);
}

uint8_t getDigitMask(uint8_t visibleDigitIndex) {
  if (visibleDigitIndex >= DISPLAY_DIGITS) {
    return getAllDigitsOffMask();
  }

  uint8_t physicalIndex = DIGIT_SELECT_MAP[visibleDigitIndex];

  if (physicalIndex >= DISPLAY_DIGITS) {
    return getAllDigitsOffMask();
  }

  if (DIGIT_ACTIVE_LOW) {
    return (uint8_t)~(1 << physicalIndex);
  } else {
    return (uint8_t)(1 << physicalIndex);
  }
}

void writeRegistersRaw(uint8_t segmentByte, uint8_t digitByte) {
  digitalWrite(LATCH_PIN, LOW);

  if (SEGMENT_BYTE_FIRST) {
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

void clearBuffer() {
  for (uint8_t i = 0; i < DISPLAY_DIGITS; i++) {
    segmentBuffer[i] = encodeBlank();
  }
}

void refreshDisplay() {
  static uint8_t muxDigit = 0;

  uint8_t digitByte = getDigitMask(muxDigit);
  uint8_t segmentByte = segmentBuffer[muxDigit];

  allOff();
  writeRegistersRaw(segmentByte, digitByte);

  muxDigit++;
  if (muxDigit >= DISPLAY_DIGITS) {
    muxDigit = 0;
  }
}

// ============================================================
// Buffer helpers
// ============================================================

void setFixed01234567() {
  segmentBuffer[0] = encodeDigit(0);
  segmentBuffer[1] = encodeDigit(1);
  segmentBuffer[2] = encodeDigit(2);
  segmentBuffer[3] = encodeDigit(3);
  segmentBuffer[4] = encodeDigit(4);
  segmentBuffer[5] = encodeDigit(5);
  segmentBuffer[6] = encodeDigit(6);
  segmentBuffer[7] = encodeDigit(7);
}

void setFixed12345678() {
  segmentBuffer[0] = encodeDigit(1);
  segmentBuffer[1] = encodeDigit(2);
  segmentBuffer[2] = encodeDigit(3);
  segmentBuffer[3] = encodeDigit(4);
  segmentBuffer[4] = encodeDigit(5);
  segmentBuffer[5] = encodeDigit(6);
  segmentBuffer[6] = encodeDigit(7);
  segmentBuffer[7] = encodeDigit(8);
}

void setOneDigitOnly(uint8_t visibleDigitIndex) {
  clearBuffer();

  if (visibleDigitIndex < DISPLAY_DIGITS) {
    segmentBuffer[visibleDigitIndex] = encodeDigit(8);
  }
}

// ------------------------------------------------------------
// Numeric counter display
// RIGHT-aligned like a normal counter:
//
// value = 308
// visible result = "     308"
//
// segmentBuffer[7] = units
// segmentBuffer[6] = tens
// segmentBuffer[5] = hundreds
// ...
// ------------------------------------------------------------
void setUnsignedCounterRightAligned(unsigned long value) {
  clearBuffer();

  if (value > 99999999UL) {
    for (uint8_t i = 0; i < DISPLAY_DIGITS; i++) {
      segmentBuffer[i] = encodeDash();
    }
    return;
  }

  if (value == 0) {
    segmentBuffer[DISPLAY_DIGITS - 1] = encodeDigit(0);
    return;
  }

  int writeIndex = DISPLAY_DIGITS - 1;

  while (value > 0 && writeIndex >= 0) {
    uint8_t digit = value % 10;
    value /= 10;

    segmentBuffer[writeIndex] = encodeDigit(digit);
    writeIndex--;
  }
}

// ============================================================
// Debug helpers
// ============================================================

void printStatus() {
  if (!serialReady) {
    return;
  }

  Serial.println();
  Serial.println("----- DISPLAY DEBUG STATUS -----");
  Serial.print("Pins ready: ");
  Serial.println(pinsReady ? "YES" : "NO");

  Serial.print("Display ready: ");
  Serial.println(displayReady ? "YES" : "NO");

  Serial.print("Mux ready: ");
  Serial.println(muxReady ? "YES" : "NO");

  Serial.print("Current mode: ");
  switch (currentMode) {
    case MODE_FIXED_01234567:
      Serial.println("MODE_FIXED_01234567");
      break;
    case MODE_FIXED_12345678:
      Serial.println("MODE_FIXED_12345678");
      break;
    case MODE_ONE_DIGIT_AT_A_TIME:
      Serial.println("MODE_ONE_DIGIT_AT_A_TIME");
      break;
    case MODE_COUNTER_111:
      Serial.println("MODE_COUNTER_111");
      break;
  }

  Serial.println("Digit select map:");
  for (uint8_t i = 0; i < DISPLAY_DIGITS; i++) {
    Serial.print("visible ");
    Serial.print(i);
    Serial.print(" -> physical ");
    Serial.println(DIGIT_SELECT_MAP[i]);
  }
}

void printHelp() {
  if (!serialReady) {
    return;
  }

  Serial.println();
  Serial.println("Commands:");
  Serial.println("0 -> show fixed 0 1 2 3 4 5 6 7");
  Serial.println("1 -> show fixed 1 2 3 4 5 6 7 8");
  Serial.println("2 -> show one digit at a time with 8");
  Serial.println("3 -> run counter in steps of 111");
  Serial.println("s -> print status");
  Serial.println("h -> print help");
}

// ============================================================
// Mode control
// ============================================================

void applyCurrentMode() {
  if (currentMode == MODE_FIXED_01234567) {
    setFixed01234567();
  } else if (currentMode == MODE_FIXED_12345678) {
    setFixed12345678();
  } else if (currentMode == MODE_ONE_DIGIT_AT_A_TIME) {
    setOneDigitOnly(activeTestDigit);
  } else if (currentMode == MODE_COUNTER_111) {
    setUnsignedCounterRightAligned(counterValue);
  }
}

void handleSerialInput() {
  if (!serialReady) {
    return;
  }

  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '0') {
      currentMode = MODE_FIXED_01234567;
      applyCurrentMode();
      Serial.println("Mode set to FIXED 01234567");
    } else if (c == '1') {
      currentMode = MODE_FIXED_12345678;
      applyCurrentMode();
      Serial.println("Mode set to FIXED 12345678");
    } else if (c == '2') {
      currentMode = MODE_ONE_DIGIT_AT_A_TIME;
      activeTestDigit = 0;
      applyCurrentMode();
      Serial.println("Mode set to ONE DIGIT AT A TIME");
    } else if (c == '3') {
      currentMode = MODE_COUNTER_111;
      counterValue = 0;
      applyCurrentMode();
      Serial.println("Mode set to COUNTER_111");
    } else if (c == 's' || c == 'S') {
      printStatus();
    } else if (c == 'h' || c == 'H') {
      printHelp();
    }
  }
}

// ============================================================
// Setup
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(300);
  serialReady = true;

  Serial.println();
  Serial.println("Starting simplified 8-digit diagnostic sketch...");

  pinMode(DATA_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  digitalWrite(DATA_PIN, LOW);
  digitalWrite(LATCH_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);

  pinsReady = true;

  allOff();
  clearBuffer();

  displayReady = true;
  muxReady = true;

  currentMode = MODE_COUNTER_111;
  counterValue = 0;
  applyCurrentMode();

  printHelp();
  printStatus();
}

// ============================================================
// Loop
// ============================================================

void loop() {
  handleSerialInput();

  unsigned long nowUs = micros();
  unsigned long nowMs = millis();

  if (displayReady && muxReady) {
    if (nowUs - lastMuxMicros >= muxIntervalUs) {
      lastMuxMicros = nowUs;
      refreshDisplay();
    }
  }

  if (currentMode == MODE_ONE_DIGIT_AT_A_TIME) {
    if (nowMs - lastDemoMs >= demoStepMs) {
      lastDemoMs = nowMs;

      activeTestDigit++;
      if (activeTestDigit >= DISPLAY_DIGITS) {
        activeTestDigit = 0;
      }

      setOneDigitOnly(activeTestDigit);

      if (serialReady) {
        Serial.print("Testing visible digit: ");
        Serial.println(activeTestDigit);
      }
    }
  }

  if (currentMode == MODE_COUNTER_111) {
    if (nowMs - lastDemoMs >= demoStepMs) {
      lastDemoMs = nowMs;

      counterValue += 1111;
      if (counterValue > 99999999UL) {
        counterValue = 0;
      }

      setUnsignedCounterRightAligned(counterValue);

      if (serialReady) {
        Serial.print("Counter = ");
        Serial.println(counterValue);
      }
    }
  }

  if (serialReady && nowMs - lastStatusMs >= statusIntervalMs) {
    lastStatusMs = nowMs;
    printStatus();
  }
}