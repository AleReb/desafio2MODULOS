// ============================================================
// TET2026 — Control básico de motor DC con L298N (versión simplificada)
// ============================================================
//
// ORIGEN DEL CÓDIGO:
// Este código es una versión simplificada de un sistema más complejo
// que originalmente controlaba el motor mediante WiFi (ESP32 en modo AP).
//
// Para fines de aprendizaje, se eliminó completamente:
// - WiFi
// - Servidor web
// - Interfaz HTML
//
// Se mantiene únicamente:
// - La lógica de control del motor
// - Uso de PWM para velocidad
// - Funciones separadas (adelante, atrás, detener)
//
// OBJETIVO:
// Entender cómo controlar dirección y velocidad de un motor DC
// de forma directa, clara y sin distracciones.
//
// ============================================================


// -------------------------
// Definición de pines
// -------------------------

const int ENA = 10;  // Pin PWM → controla la velocidad del motor
const int IN1 = 11;  // Pin de control → dirección
const int IN2 = 12;  // Pin de control → dirección


// ============================================================
// Funciones de control del motor
// ============================================================

void adelante(int velocidad) {

  // Asegura que la velocidad esté dentro del rango válido (0 a 255)
  velocidad = constrain(velocidad, 0, 255);

  // Configura dirección: adelante
  // IN1 = HIGH, IN2 = LOW
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // Aplica PWM al pin ENA → controla velocidad
  analogWrite(ENA, velocidad);

  // Mensaje para monitoreo
  Serial.print("Adelante | Velocidad: ");
  Serial.println(velocidad);
}

void atras(int velocidad) {

  // Asegura rango válido
  velocidad = constrain(velocidad, 0, 255);

  // Configura dirección: atrás
  // IN1 = LOW, IN2 = HIGH
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // Aplica PWM
  analogWrite(ENA, velocidad);

  // Mensaje para monitoreo
  Serial.print("Atras | Velocidad: ");
  Serial.println(velocidad);
}

void detener() {

  // Ambos pines en LOW → motor sin movimiento
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  // PWM en 0 → sin energía al motor
  analogWrite(ENA, 0);

  Serial.println("Motor detenido");
}


// ============================================================
// Configuración inicial
// ============================================================

void setup() {

  // Inicializa comunicación serial
  Serial.begin(115200);

  // Configura pines como salida
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Estado inicial seguro
  detener();
}


// ============================================================
// Loop de prueba
// ============================================================

void loop() {

  // Movimiento hacia adelante a baja velocidad
  adelante(80);
  delay(3000);

  // Detención
  detener();
  delay(2000);

  // Movimiento hacia adelante a mayor velocidad
  adelante(180);
  delay(3000);

  // Detención
  detener();
  delay(2000);

  // Movimiento en reversa
  atras(150);
  delay(3000);

  // Detención final
  detener();
  delay(4000);
}
