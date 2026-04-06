// ---------------------------------------------------------
// ESTRUCTURA DE MÁQUINA DE ESTADOS
// ---------------------------------------------------------
enum EstadoModulo { ESPERA, ACTIVO, ENFRIANDO };
EstadoModulo estadoActual = ESPERA;

// ---------------------------------------------------------
// VARIABLES PARA LÓGICA REACTIVA (millis)
// ---------------------------------------------------------
unsigned long tAnterior = 0;
const unsigned long TIEMPO_ACTIVO = 2000;    // 2 segundos moviendo la bolita
const unsigned long TIEMPO_ENFRIANDO = 1000; // 1 segundo de pausa de seguridad

// ---------------------------------------------------------
// CONFIGURACIÓN DE PINES (ESP32)
// ---------------------------------------------------------
const int PIN_IR = 34; // Sensor de proximidad TCRT5000

// Pines para el control del motor 
const int PIN_ENA = 10;
const int PIN_IN1 = 11;
const int PIN_IN2 = 12;

void setup() {
  Serial.begin(115200);
  
  // Configuración de pines
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  
  // Asegurar que el motor inicie detenido
  detenerMotor();
  
  Serial.println("Sistema Iniciado - Estado: ESPERA");
}

void loop() {
  // 1. LECTURA CONTINUA (El programa nunca se bloquea con delays)
  // El TCRT5000 entrega LOW cuando la luz infrarroja rebota en un objeto
  bool bolitaDetectada = (digitalRead(PIN_IR) == LOW); 
  unsigned long tActual = millis();

  // 2. EVALUACIÓN DE LA MÁQUINA DE ESTADOS
  switch (estadoActual) {
    
    case ESPERA:
      // Condición para avanzar: El sensor detecta una bolita
      if (bolitaDetectada) {
        Serial.println("¡Bolita detectada! Pasando a estado ACTIVO.");
        activarMotor();
        tAnterior = tActual;           // Guardamos el "sello de tiempo" inicial
        estadoActual = ACTIVO;         // Transición al siguiente estado
      }
      break;

    case ACTIVO:
      // Condición para avanzar: Ya transcurrió el tiempo de transporte
      if (tActual - tAnterior >= TIEMPO_ACTIVO) {
        Serial.println("Transporte finalizado. Pasando a estado ENFRIANDO.");
        detenerMotor();
        tAnterior = tActual;           // Reiniciamos el cronómetro para la pausa
        estadoActual = ENFRIANDO;      // Transición al siguiente estado
      }
      break;

    case ENFRIANDO:
      // Condición para avanzar: Ya transcurrió el tiempo de seguridad
      // (Esto evita que el módulo se reactive inmediatamente si la bolita no ha salido del todo)
      if (tActual - tAnterior >= TIEMPO_ENFRIANDO) {
        Serial.println("Enfriamiento completo. Volviendo a ESPERA.");
        estadoActual = ESPERA;         // El ciclo vuelve al inicio
      }
      break;
  }

  // Al no haber delays, aquí podrías agregar otras funciones, 
  // como hacer parpadear un LED o leer un segundo sensor, y todo funcionaría en paralelo.
}

// ---------------------------------------------------------
// FUNCIONES AUXILIARES DE HARDWARE
// ---------------------------------------------------------
void activarMotor() {
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_ENA, HIGH); // Habilita la entrega de energía
}

void detenerMotor() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_ENA, LOW);  // Corta la energía del motor
}