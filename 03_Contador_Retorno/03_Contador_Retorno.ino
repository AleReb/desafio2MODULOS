#include <Servo.h>

// --- Pines y Constantes ---
const int PIN_IR_CONTADOR = 34; // Pin del sensor infrarrojo (ajustar a la protoboard)
const int SERVO_PIN = 15;       // Pin del Servo motor
const int LIMITE_BOLITAS = 9;   // Cantidad para vaciar el contenedor

// --- Variables del Servo ---
Servo servoCompuerta;
const int POS_DEFECTO = 90; // Compuerta cerrada
const int POS_ABIERTA = 45; // Compuerta abierta

// --- Variables de Control y Tiempo ---
unsigned long tApertura = 0;
const long tiempoVaciado = 2000; // Tiempo en mseg que la compuerta se mantendrá abierta

// --- Control de Flancos para Conteo ---
int conteoBolitas = 0;
bool estadoAnteriorIR = true; // El sensor IR TCRT5000 suele entregar HIGH cuando NO hay objeto

// --- Máquina de Estados ---
enum EstadoSistema { RECOLECTANDO, VACIANDO_CONTENEDOR };
EstadoSistema estadoActual = RECOLECTANDO;

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_IR_CONTADOR, INPUT);
  
  servoCompuerta.attach(SERVO_PIN);
  servoCompuerta.write(POS_DEFECTO); // Iniciar guardando en posición neutral
  
  Serial.println("Iniciando Módulo 05: Contador Retorno");
}

void loop() {
  // Leer el sensor IR (Lógica invertida: LOW significa objeto detectado)
  bool estadoActualIR = digitalRead(PIN_IR_CONTADOR); 
  
  switch (estadoActual) {
    case RECOLECTANDO:
      // Control de Flanco descendente: El estado anterior era HIGH (sin objeto) 
      // y el nuevo es LOW (objeto detectado)
      if (estadoAnteriorIR == HIGH && estadoActualIR == LOW) {
        conteoBolitas++;
        Serial.print("Bolita detectada! Total: ");
        Serial.println(conteoBolitas);
        
        // Si llegamos al límite establecido, pasamos de estado
        if (conteoBolitas >= LIMITE_BOLITAS) {
          Serial.println("Límite alcanzado, abriendo compuerta...");
          servoCompuerta.write(POS_ABIERTA);
          tApertura = millis(); // Registrar tiempo de inicio de apertura
          estadoActual = VACIANDO_CONTENEDOR;
        }
      }
      
      // Actualizar el estado anterior para la próxima iteración
      estadoAnteriorIR = estadoActualIR; 
      break;
      
    case VACIANDO_CONTENEDOR:
      // Esperar hasta que se cumpla el tiempo de vaciado, utilizando millis()
      if (millis() - tApertura >= tiempoVaciado) {
        Serial.println("Vaciado finalizado, cerrando compuerta y reiniciando...");
        servoCompuerta.write(POS_DEFECTO); // Volver el servo a posición cerrada
        conteoBolitas = 0;                 // Reiniciar el contador
        
        // Registrar nuevamente la lectura actual para no contabilizar erróneamente
        estadoAnteriorIR = digitalRead(PIN_IR_CONTADOR);
        
        estadoActual = RECOLECTANDO;       // Volver al estado inicial
      }
      break;
  }
}
