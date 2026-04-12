#include <ESP32Servo.h>

Servo myservo;  // crea un objeto servo para controlar un servo
// Se pueden crear 16 objetos servo en el ESP32
// Posibles pines PWM GPIO en el ESP32-S3: 0(usado por el botón integrado),1-21,35-45,47,48(usado por el LED integrado)
int servoPin = 6;//revisa la lista disponible
const int SensorPin = 7;  // numero del  pin del sensor TCRT5000 este sensor es de logica invertida


// variables que cambiarán:
int EstadoSensor = 0;  // variable para leer el estado del sensor

void setup() {
  Serial.begin(115200);
  // Permite la asignación de todos los temporizadores
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);            // servo estándar de 50 hz
  myservo.attach(servoPin, 1000, 2000);  // conecta el servo en el pin 6 al objeto servo
                                         // usando un min/max por defecto de 1000us y 2000us
                                         // diferentes servos pueden requerir diferentes ajustes min/max
                                         // para un barrido preciso de 0 a 180
  // inicializa el pin del sensor como entrada:
  pinMode(SensorPin, INPUT);
}

void loop() {
  // lee el estado del valor del sensor:
  EstadoSensor = digitalRead(SensorPin);
  Serial.println(EstadoSensor);
  // revisa si el sensor detecto algo o no si detecto es LOW:
  if (EstadoSensor == LOW) {
    // mueve el servor deja pasar algo por ejemplo
    myservo.write(5);  // vuelve a su posicion "abierta"
    delay(1500);   // espera para que llegue a esa posicion
  } else {
    myservo.write(155);  // vuelve a su posicion "cerrada"
  }
}
