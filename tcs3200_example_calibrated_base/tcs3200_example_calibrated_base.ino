/*
 *
 * TCS3200 calibrated classifier for ESP32-S2
 *
 * Features:
 * - Serial-based calibration
 * - Averaged readings
 * - Normalization using clear channel
 * - Optional white/black normalization
 * - Reference-based classification
 * - UNKNOWN state when confidence is low
 * - Global flags and serial diagnostics
 *
 */

#include <tcs3200.h>
#include <math.h>

// --------------------------------------------------
// Pin configuration
// --------------------------------------------------
const int PIN_S0  = 8;
const int PIN_S1  = 9;
const int PIN_S2  = 11;
const int PIN_S3  = 10;
const int PIN_OUT = 12;

// Sensor instance
tcs3200 tcs(PIN_S0, PIN_S1, PIN_S2, PIN_S3, PIN_OUT);

// --------------------------------------------------
// Global status flags
// --------------------------------------------------
bool serialReady = false;
bool sensorReady = false;

bool whiteCalibrated = false;
bool blackCalibrated = false;
bool redCalibrated = false;
bool greenCalibrated = false;
bool blueCalibrated = false;

// --------------------------------------------------
// Thresholds and timing
// --------------------------------------------------
float thresholdUnknown = 0.25f;   // Adjust experimentally
unsigned long lastSampleMs = 0;
unsigned long lastStatusMs = 0;

const unsigned long sampleIntervalMs = 400;
const unsigned long statusIntervalMs = 5000;

// --------------------------------------------------
// Data structures
// --------------------------------------------------
struct RawSample {
  float r;
  float g;
  float b;
  float c;
};

struct NormSample {
  float r;
  float g;
  float b;
};

// --------------------------------------------------
// Calibration references
// --------------------------------------------------
RawSample whiteRef = {0, 0, 0, 0};
RawSample blackRef = {0, 0, 0, 0};
RawSample redRef   = {0, 0, 0, 0};
RawSample greenRef = {0, 0, 0, 0};
RawSample blueRef  = {0, 0, 0, 0};

// --------------------------------------------------
// Function declarations
// --------------------------------------------------
RawSample readAveragedRawSample(int samples);
NormSample normalizeSample(const RawSample &in);
float colorDistance(const NormSample &a, const NormSample &b);
bool fullColorCalibrationReady();
void printRawSample(const char *label, const RawSample &s);
void printNormSample(const char *label, const NormSample &s);
void printCalibrationStatus();
void printCommands();
void handleSerialCommand(char cmd);
const char* classifyColor(const NormSample &current, float &dRed, float &dGreen, float &dBlue);

// --------------------------------------------------
// Setup
// --------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(800);

  serialReady = true;

  Serial.println();
  Serial.println("========================================");
  Serial.println("TCS3200 Calibrated Classifier");
  Serial.println("Board: ESP32-S2");
  Serial.println("========================================");

  sensorReady = true;

  if (sensorReady) {
    Serial.println("Sensor status: READY");
  } else {
    Serial.println("Sensor status: NOT READY");
  }

  printCommands();
}

// --------------------------------------------------
// Main loop
// --------------------------------------------------
void loop() {
  // -----------------------------
  // Handle incoming serial commands
  // -----------------------------
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd != '\n' && cmd != '\r') {
      handleSerialCommand(cmd);
    }
  }

  // -----------------------------
  // Sensor status monitoring
  // -----------------------------
  if (!sensorReady) {
    if (millis() - lastStatusMs >= statusIntervalMs) {
      lastStatusMs = millis();
      Serial.println("[WARN] Sensor not ready. Retrying logical state...");
      sensorReady = true;
      Serial.print("[INFO] Sensor status now: ");
      Serial.println(sensorReady ? "READY" : "NOT READY");
    }

    delay(50);
  } else {
    // -----------------------------
    // Periodic classification
    // -----------------------------
    if (millis() - lastSampleMs >= sampleIntervalMs) {
      lastSampleMs = millis();

      RawSample currentRaw = readAveragedRawSample(8);
      NormSample currentNorm = normalizeSample(currentRaw);

      float dRed = 0.0f;
      float dGreen = 0.0f;
      float dBlue = 0.0f;

      const char *detected = classifyColor(currentNorm, dRed, dGreen, dBlue);

      Serial.print("RAW  -> ");
      Serial.print("R=");
      Serial.print(currentRaw.r, 2);
      Serial.print(" G=");
      Serial.print(currentRaw.g, 2);
      Serial.print(" B=");
      Serial.print(currentRaw.b, 2);
      Serial.print(" C=");
      Serial.print(currentRaw.c, 2);
      Serial.print("    ");

      Serial.print("NORM -> ");
      Serial.print("Rn=");
      Serial.print(currentNorm.r, 4);
      Serial.print(" Gn=");
      Serial.print(currentNorm.g, 4);
      Serial.print(" Bn=");
      Serial.print(currentNorm.b, 4);
      Serial.print("    ");

      Serial.print("CLASS -> ");
      Serial.print(detected);

      if (fullColorCalibrationReady()) {
        Serial.print("    D[R,G,B]=");
        Serial.print(dRed, 4);
        Serial.print(", ");
        Serial.print(dGreen, 4);
        Serial.print(", ");
        Serial.print(dBlue, 4);
      } else {
        Serial.print("    Waiting calibration...");
      }

      Serial.println();
    }
  }
}

// --------------------------------------------------
// Read averaged raw sample
// --------------------------------------------------
RawSample readAveragedRawSample(int samples) {
  RawSample s = {0, 0, 0, 0};

  if (samples <= 0) {
    samples = 1;
  }

  for (int i = 0; i < samples; i++) {
    s.r += tcs.colorRead('r');
    s.g += tcs.colorRead('g');
    s.b += tcs.colorRead('b');
    s.c += tcs.colorRead('c');
    delay(20);
  }

  s.r /= samples;
  s.g /= samples;
  s.b /= samples;
  s.c /= samples;

  return s;
}

// --------------------------------------------------
// Normalize sample
// --------------------------------------------------
NormSample normalizeSample(const RawSample &in) {
  NormSample out = {0, 0, 0};

  float clearValue = in.c;
  if (clearValue < 1.0f) {
    clearValue = 1.0f;
  }

  // Ratio against clear channel
  float rr = in.r / clearValue;
  float gg = in.g / clearValue;
  float bb = in.b / clearValue;

  // Inversion because lower measured values often
  // correspond to stronger reflected color in this setup
  rr = 1.0f / max(rr, 0.0001f);
  gg = 1.0f / max(gg, 0.0001f);
  bb = 1.0f / max(bb, 0.0001f);

  // Optional normalization using white and black references
  if (whiteCalibrated && blackCalibrated) {
    float wr = whiteRef.r / max(whiteRef.c, 1.0f);
    float wg = whiteRef.g / max(whiteRef.c, 1.0f);
    float wb = whiteRef.b / max(whiteRef.c, 1.0f);

    float kr = blackRef.r / max(blackRef.c, 1.0f);
    float kg = blackRef.g / max(blackRef.c, 1.0f);
    float kb = blackRef.b / max(blackRef.c, 1.0f);

    wr = 1.0f / max(wr, 0.0001f);
    wg = 1.0f / max(wg, 0.0001f);
    wb = 1.0f / max(wb, 0.0001f);

    kr = 1.0f / max(kr, 0.0001f);
    kg = 1.0f / max(kg, 0.0001f);
    kb = 1.0f / max(kb, 0.0001f);

    if (fabs(wr - kr) > 0.0001f) {
      rr = (rr - kr) / (wr - kr);
    }

    if (fabs(wg - kg) > 0.0001f) {
      gg = (gg - kg) / (wg - kg);
    }

    if (fabs(wb - kb) > 0.0001f) {
      bb = (bb - kb) / (wb - kb);
    }
  }

  // Clamp values to a reasonable range
  if (rr < 0.0f) rr = 0.0f;
  if (gg < 0.0f) gg = 0.0f;
  if (bb < 0.0f) bb = 0.0f;

  if (rr > 2.0f) rr = 2.0f;
  if (gg > 2.0f) gg = 2.0f;
  if (bb > 2.0f) bb = 2.0f;

  out.r = rr;
  out.g = gg;
  out.b = bb;

  return out;
}

// --------------------------------------------------
// Euclidean distance in normalized space
// --------------------------------------------------
float colorDistance(const NormSample &a, const NormSample &b) {
  float dr = a.r - b.r;
  float dg = a.g - b.g;
  float db = a.b - b.b;

  return sqrtf(dr * dr + dg * dg + db * db);
}

// --------------------------------------------------
// Check if RGB references are complete
// --------------------------------------------------
bool fullColorCalibrationReady() {
  return redCalibrated && greenCalibrated && blueCalibrated;
}

// --------------------------------------------------
// Print helpers
// --------------------------------------------------
void printRawSample(const char *label, const RawSample &s) {
  Serial.print(label);
  Serial.print(" -> ");
  Serial.print("R=");
  Serial.print(s.r, 2);
  Serial.print(" G=");
  Serial.print(s.g, 2);
  Serial.print(" B=");
  Serial.print(s.b, 2);
  Serial.print(" C=");
  Serial.println(s.c, 2);
}

void printNormSample(const char *label, const NormSample &s) {
  Serial.print(label);
  Serial.print(" -> ");
  Serial.print("Rn=");
  Serial.print(s.r, 4);
  Serial.print(" Gn=");
  Serial.print(s.g, 4);
  Serial.print(" Bn=");
  Serial.println(s.b, 4);
}

void printCommands() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  w = capture white reference");
  Serial.println("  k = capture black reference");
  Serial.println("  r = capture red reference");
  Serial.println("  g = capture green reference");
  Serial.println("  b = capture blue reference");
  Serial.println("  p = print calibration status");
  Serial.println("  t = print current UNKNOWN threshold");
  Serial.println();
  Serial.println("Recommended order: w -> k -> r -> g -> b");
  Serial.println();
}

void printCalibrationStatus() {
  Serial.println();
  Serial.println("=========== Calibration Status ===========");
  Serial.print("Serial ready: ");
  Serial.println(serialReady ? "YES" : "NO");

  Serial.print("Sensor ready: ");
  Serial.println(sensorReady ? "YES" : "NO");

  Serial.print("White calibrated: ");
  Serial.println(whiteCalibrated ? "YES" : "NO");

  Serial.print("Black calibrated: ");
  Serial.println(blackCalibrated ? "YES" : "NO");

  Serial.print("Red calibrated: ");
  Serial.println(redCalibrated ? "YES" : "NO");

  Serial.print("Green calibrated: ");
  Serial.println(greenCalibrated ? "YES" : "NO");

  Serial.print("Blue calibrated: ");
  Serial.println(blueCalibrated ? "YES" : "NO");

  Serial.print("Unknown threshold: ");
  Serial.println(thresholdUnknown, 4);

  if (whiteCalibrated) {
    printRawSample("White raw", whiteRef);
    printNormSample("White norm", normalizeSample(whiteRef));
  }

  if (blackCalibrated) {
    printRawSample("Black raw", blackRef);
    printNormSample("Black norm", normalizeSample(blackRef));
  }

  if (redCalibrated) {
    printRawSample("Red raw", redRef);
    printNormSample("Red norm", normalizeSample(redRef));
  }

  if (greenCalibrated) {
    printRawSample("Green raw", greenRef);
    printNormSample("Green norm", normalizeSample(greenRef));
  }

  if (blueCalibrated) {
    printRawSample("Blue raw", blueRef);
    printNormSample("Blue norm", normalizeSample(blueRef));
  }

  Serial.println("==========================================");
  Serial.println();
}

// --------------------------------------------------
// Handle serial calibration commands
// --------------------------------------------------
void handleSerialCommand(char cmd) {
  RawSample captured;

  switch (cmd) {
    case 'w':
      Serial.println("[CAL] Capturing WHITE reference...");
      captured = readAveragedRawSample(15);
      whiteRef = captured;
      whiteCalibrated = true;
      printRawSample("White captured", whiteRef);
      printNormSample("White normalized", normalizeSample(whiteRef));
      break;

    case 'k':
      Serial.println("[CAL] Capturing BLACK reference...");
      captured = readAveragedRawSample(15);
      blackRef = captured;
      blackCalibrated = true;
      printRawSample("Black captured", blackRef);
      printNormSample("Black normalized", normalizeSample(blackRef));
      break;

    case 'r':
      Serial.println("[CAL] Capturing RED reference...");
      captured = readAveragedRawSample(15);
      redRef = captured;
      redCalibrated = true;
      printRawSample("Red captured", redRef);
      printNormSample("Red normalized", normalizeSample(redRef));
      break;

    case 'g':
      Serial.println("[CAL] Capturing GREEN reference...");
      captured = readAveragedRawSample(15);
      greenRef = captured;
      greenCalibrated = true;
      printRawSample("Green captured", greenRef);
      printNormSample("Green normalized", normalizeSample(greenRef));
      break;

    case 'b':
      Serial.println("[CAL] Capturing BLUE reference...");
      captured = readAveragedRawSample(15);
      blueRef = captured;
      blueCalibrated = true;
      printRawSample("Blue captured", blueRef);
      printNormSample("Blue normalized", normalizeSample(blueRef));
      break;

    case 'p':
      printCalibrationStatus();
      break;

    case 't':
      Serial.print("[INFO] UNKNOWN threshold = ");
      Serial.println(thresholdUnknown, 4);
      break;

    default:
      Serial.print("[WARN] Unknown command: ");
      Serial.println(cmd);
      printCommands();
      break;
  }
}

// --------------------------------------------------
// Classification logic
// --------------------------------------------------
const char* classifyColor(const NormSample &current, float &dRed, float &dGreen, float &dBlue) {
  if (!fullColorCalibrationReady()) {
    dRed = -1.0f;
    dGreen = -1.0f;
    dBlue = -1.0f;
    return "NOT_CALIBRATED";
  }

  NormSample redNorm = normalizeSample(redRef);
  NormSample greenNorm = normalizeSample(greenRef);
  NormSample blueNorm = normalizeSample(blueRef);

  dRed = colorDistance(current, redNorm);
  dGreen = colorDistance(current, greenNorm);
  dBlue = colorDistance(current, blueNorm);

  float bestDistance = dRed;
  const char *bestLabel = "RED";

  if (dGreen < bestDistance) {
    bestDistance = dGreen;
    bestLabel = "GREEN";
  }

  if (dBlue < bestDistance) {
    bestDistance = dBlue;
    bestLabel = "BLUE";
  }

  if (bestDistance > thresholdUnknown) {
    return "UNKNOWN";
  }

  return bestLabel;
}