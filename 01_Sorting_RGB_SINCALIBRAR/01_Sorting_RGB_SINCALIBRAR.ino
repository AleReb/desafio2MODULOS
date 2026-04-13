#include <ESP32Servo.h>
#include <tcs3200.h>

// --- Configuración de Pines ---
const int PIN_S0  = 8;
const int PIN_S1  = 9;
const int PIN_S2  = 11;
const int PIN_S3  = 10;
const int PIN_OUT = 12;

const int SERVO_PIN = 15;

// Instanciar el sensor de color y el servo
tcs3200 tcs(PIN_S0, PIN_S1, PIN_S2, PIN_S3, PIN_OUT); 
Servo desviador;

// --- Estructura para tener una lectura más estable ---
struct RawSample {
  float r;
  float g;
  float b;
  float c;
};

// --- Variables de Control y Tiempo ---
unsigned long tPrevia = 0;
const long intervaloLectura = 500; // Validar color cada 500ms

// --- Umbral de Detección (ajustar con la luz de la sala) ---
// Valores MÁS BAJOS indican MÁS intensidad de color detectada
const int UMBRAL_PRESENCIA = 150; 

// --------------------------------------------------
// Función para tomar múltiples muestras y promediarlas (mejora estabilidad)
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
  }

  s.r /= samples;
  s.g /= samples;
  s.b /= samples;
  s.c /= samples;

  return s;
}

void setup() {
  Serial.begin(115200);

  // Configuración de los temporizadores de ESP32Servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  desviador.setPeriodHertz(50); // Servo estándar de 50Hz

  desviador.attach(SERVO_PIN, 1000, 2000);
  desviador.write(90); // Posición neutral por defecto
  
  Serial.println("Iniciando Módulo 01: Sorting RGB (Promediado sin calibrar)");
}

void loop() {
  unsigned long tActual = millis();

  // Se evalúa de manera asíncrona mediante millis
  if (tActual - tPrevia >= intervaloLectura) {
    tPrevia = tActual;
    
    // Obtenemos una lectura promediando 5 muestras al instante
    RawSample lecturaPromedio = readAveragedRawSample(5);

    Serial.print("Rojo: "); Serial.print(lecturaPromedio.r, 1);
    Serial.print(" | Verde: "); Serial.print(lecturaPromedio.g, 1);
    Serial.print(" | Azul: "); Serial.println(lecturaPromedio.b, 1);

    // LÓGICA DE CLASIFICACIÓN
    // Comprobamos si al menos un color cruzó el umbral de presencia, 
    // lo que significa que el objeto está suficientemente cerca.
    if (lecturaPromedio.r < UMBRAL_PRESENCIA || 
        lecturaPromedio.g < UMBRAL_PRESENCIA || 
        lecturaPromedio.b < UMBRAL_PRESENCIA) {

      // Es ROJO si el valor promedio de rojo es el menor de los tres
      if (lecturaPromedio.r < lecturaPromedio.g && lecturaPromedio.r < lecturaPromedio.b) {
        Serial.println("¡Bolita ROJA detectada! Desviando (Angular: 45°)");
        desviador.write(45);
      } 
      // Es VERDE si el valor promedio verde es el menor de los tres
      else if (lecturaPromedio.g < lecturaPromedio.r && lecturaPromedio.g < lecturaPromedio.b) {
        Serial.println("¡Bolita VERDE detectada! Desviando (Angular: 135°)");
        desviador.write(135);
      } 
      // Es AZUL si el valor promedio azul es el menor de los tres
      else if (lecturaPromedio.b < lecturaPromedio.r && lecturaPromedio.b < lecturaPromedio.g) {
        Serial.println("¡Bolita AZUL detectada! Desviando (Angular: 175°)");
        desviador.write(175);
      }
      
    } else {
      // Los valores de todos los colores son altos (baja intensidad de lectura reflejada) 
      // => No hay ninguna bolita frente al sensor
      desviador.write(90); // Mantener el camino neutral abierto
    }
  }
}
