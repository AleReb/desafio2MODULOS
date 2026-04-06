const int PIN_IR = 34;
const int ENA = 10;  // Pin PWM controla la velocidad del motor
const int IN1 = 11;  // Pin de control dirección 1
const int IN2 = 12;  // Pin de control dirección 2

unsigned long tMotorInicio = 0;
const long duracionTransporte = 3000; // 3 segundos encendido

enum Estado { ESPERA, ACTIVO };
Estado estadoActual = ESPERA;

void setup() {
  pinMode(PIN_IR, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  // Estado inicial seguro: motor detenido
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

void loop() {
  bool hayBolita = (digitalRead(PIN_IR) == LOW); // Lógica invertida TCRT5000

  switch (estadoActual) {
    case ESPERA:
      if (hayBolita) {
        // Mover motor hacia adelante
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, 255); // Ajustar velocidad de 0 a 255
        
        tMotorInicio = millis();
        estadoActual = ACTIVO;
      }
      break;

    case ACTIVO:
      // El motor sigue encendido mientras no pasen los 3 segundos
      if (millis() - tMotorInicio >= duracionTransporte) {
        // Detener motor
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, 0);
        
        estadoActual = ESPERA;
      }
      break;
  }
}
