#include <Servo.h>
#include <tcs3200.h>

int red, green, blue, white;

// S0, S1, S2, S3, output pin
tcs3200 tcs(4, 5, 6, 7, 8); 

// Pin Servo
const int SERVO_PIN = 15;
Servo desviador;

unsigned long tPrevia = 0;
const long intervaloLectura = 500; // Leer cada 500ms

void setup() {
  Serial.begin(115200);

  desviador.attach(SERVO_PIN);
  desviador.write(90); // Posición neutral
  
  Serial.println("Iniciando Módulo 01: Sorting RGB");
}

void loop() {
  unsigned long tActual = millis();

  if (tActual - tPrevia >= intervaloLectura) {
    tPrevia = tActual;
    
    // Usar la librería para leer los canales RGB
    red = tcs.colorRead('r');
    green = tcs.colorRead('g');
    blue = tcs.colorRead('b');

    Serial.print("Rojo: "); Serial.print(red);
    Serial.print(" | Verde: "); Serial.print(green);
    Serial.print(" | Azul: "); Serial.println(blue);

    // Ajusta el umbral dependiendo de la respuesta de la librería en la prueba física
    // (A menor valor, mayor intensidad detectada del color correspondiente)
    if (red < 100 && red < green && red < blue) { // Umbral detectado
      Serial.println("¡Bolita Roja detectada! Desviando...");
      desviador.write(45); // Desviar bolita roja
    } else {
      desviador.write(90); // Mantener camino
    }
  }
}
