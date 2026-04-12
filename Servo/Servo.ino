/* BÁSICO
 por BARRAGAN <http://barraganstudio.com>
 Este código de ejemplo es de dominio público.

 modificado el 8 Nov 2013
 por Scott Fitzgerald

 modificado para el ESP32 en Marzo 2017
 por John Bennett
 modificado para el ESP32 clase Explracion tecnologica Abril 2026
 por Alejandro rebolledo 

  

 * Diferentes servos requieren diferentes anchos de pulso para variar el ángulo del servo, pero el rango es 
 * un pulso de aproximadamente 500-2500 microsegundos cada 20ms (50Hz). En general, los servos básicos
 * hacen un barrido de 180 grados, por lo que el número más bajo en el rango publicado para un servo en particular
 * representa un ángulo de 0 grados, el medio del rango representa 90 grados, y la parte superior
 * del rango representa 180 grados. Así que por ejemplo, si el rango es de 1000us a 2000us,
 * 1000us igualaría un ángulo de 0, 1500us igualaría 90 grados, y 2000us igualaría 1800
 * grados.
 * 
 * Circuito: (usando un ESP32 Thing de Sparkfun)
 * Los servomotores tienen tres cables: alimentación, tierra y señal. El cable de alimentación es típicamente rojo,
 * el cable de tierra es típicamente negro o marrón, y el cable de señal es típicamente amarillo,
 * naranja o blanco. Dado que el ESP32 puede suministrar corriente limitada a solo 3.3V, y los servos extraen
 * una cantidad considerable de energía, conectaremos la energía del servo al pin VBat del ESP32 (ubicado
 * cerca del conector USB). ESTO ES SOLO APROPIADO PARA SERVOS PEQUEÑOS. 
 * 
 * También podríamos conectar la energía del servo a una fuente de energía externa
 * separada (siempre y cuando conectemos todas las tierras (ESP32, servo, y energía externa).
 * En este ejemplo, solo conectamos la tierra del ESP32 a la tierra del servo. Los pines de señal del servo
 * se conectan a cualquier pin GPIO disponible en el ESP32 (en este ejemplo, usamos el pin 18).
 * 
 * En este ejemplo, asumimos un servo grande Tower Pro MG995 conectado a una fuente de energía externa.
 * El min y max publicado para este servo es 1000 y 2000, respectivamente, por lo que los valores predeterminados están bien.
 * Estos valores realmente impulsan a los servos un poco más allá de 0 y 180, por lo que
 * si es muy particular, ajuste los valores min y max para que coincidan con sus necesidades.
 Es importante no llegar a los limites fisicos del servo para que no se dañen 
 */

#include <ESP32Servo.h>

Servo myservo;  // crea un objeto servo para controlar un servo
// Se pueden crear 16 objetos servo en el ESP32
// Posibles pines PWM GPIO en el ESP32-S3: 0(usado por el botón integrado),1-21,35-45,47,48(usado por el LED integrado)
int servoPin = 6;

void setup() {
  // Permite la asignación de todos los temporizadores
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);            // servo estándar de 50 hz
  myservo.attach(servoPin, 1000, 2000);  // conecta el servo en el pin 18 al objeto servo
                                         // usando un min/max por defecto de 1000us y 2000us
                                         // diferentes servos pueden requerir diferentes ajustes min/max
                                         // para un barrido preciso de 0 a 180
}

void loop() {
  myservo.write(5);  // le dice al servo llega al algulos 5
  delay(3000);       // espera para que llegue a esa posicion
  myservo.write(175);  //le dice al servo llega al algulos 5
  delay(3000);           // espera para que llegue a esa posicion
}
