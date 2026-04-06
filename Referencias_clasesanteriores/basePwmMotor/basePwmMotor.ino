// ============================================================
// TET2026 — Ejemplo simple: control de velocidad de un motor DC con PWM
// Usa L298N - solo movimiento hacia adelante
// ============================================================

// Definición de pines
const int ENA = 10;  // Pin PWM que controla la velocidad del motor
const int IN1 = 11;  // Pin de dirección 1
const int IN2 = 12;  // Pin de dirección 2

void setup() {
  Serial.begin(115200); // Inicializa comunicación serial para monitoreo

  pinMode(ENA, OUTPUT); // Configura ENA como salida (PWM)
  pinMode(IN1, OUTPUT); // Configura IN1 como salida
  pinMode(IN2, OUTPUT); // Configura IN2 como salida

  // Configuración de dirección: siempre hacia adelante
  digitalWrite(IN1, HIGH); // IN1 en alto
  digitalWrite(IN2, LOW);  // IN2 en bajo
}

void loop() {

  // --- Velocidad baja ---
  Serial.println("Velocidad baja"); // Mensaje en monitor serial
  analogWrite(ENA, 50);             // PWM bajo (motor gira lento)
  delay(3000);                      // Espera 3 segundos

  // --- Velocidad media ---
  Serial.println("Velocidad media"); 
  analogWrite(ENA, 100);            // PWM medio
  delay(3000);                      // Espera 3 segundos

  // --- Velocidad alta ---
  Serial.println("Velocidad alta");
  analogWrite(ENA, 180);            // PWM alto
  delay(3000);                      // Espera 3 segundos

  // --- Velocidad máxima ---
  Serial.println("Velocidad maxima");
  analogWrite(ENA, 255);            // PWM máximo (100% duty cycle)
  delay(3000);                      // Espera 3 segundos

  // --- Freno (detención del motor) ---
  Serial.println("Freno");
  analogWrite(ENA, 0);              // PWM en 0 → motor detenido

  delay(10000);                     // Espera 10 segundos antes de repetir
}