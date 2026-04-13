# ⚙️ Ejemplos Prácticos - Clase 12: Sensores y Lógica Reactiva

Este repositorio presenta códigos de ejemplo para el curso TET2026, enfocados en la integración de módulos autónomos que aplican conceptos de Lógica Reactiva y Máquinas de Estado en la placa de desarrollo ESP32 (incluyendo S2/S3).

---

## 🎯 Objetivo

Ayudar a los equipos a lograr que sus módulos detecten bolitas y actúen de forma **autónoma**, realizando múltiples tareas simultáneas de forma eficiente sin interrumpir la lectura de los sensores.

---

## 🧠 Contexto Pedagógico y Conceptos Clave

Para que los módulos funcionen correctamente (como leer un sensor mientras un motor está girando o esperando un tiempo de vaciado), este código aplica dos principios fundamentales enseñados en la Clase 12:

1. **Adiós al `delay()`:** El uso de `delay()` bloquea el microcontrolador de forma temporal. En su lugar, utilizamos la función `millis()` para medir el tiempo transcurrido sin detener la ejecución global del código.
2. **Máquinas de Estado:** Organizamos el comportamiento de los módulos en fases lógicas (por ejemplo, *ESPERA*, *ACTIVO*, *RECOLECTANDO*). El sistema evalúa constantemente en qué estado se encuentra y determina qué condición debe cumplirse para llevarlo transicionalmente al siguiente.

---

## 📂 Estructura del Repositorio y Configuración por Módulos

Los códigos están divididos según el tipo de módulo en el que estés trabajando y qué tareas deba realizar.

### 🧩 `00_Maquina_de_Estados` (Estructura Base)
Es la plantilla estructural ideal para comprender el funcionamiento asíncrono y comenzar a programar un módulo inteligente desde cero.
* **Sensores/Actuadores:** Sensor de Proximidad Infrarrojo (TCRT5000) y conectividad a módulo controlador de Motor DC (L298N).
* **Pines definidos:** 
  * `PIN_IR = 34` (Entrada del sensor)
  * Puente H: `PIN_ENA = 10` (Velocidad PWM), `PIN_IN1 = 11`, `PIN_IN2 = 12` (Dirección).
* **Resumen y Uso:** Demuestra una máquina de 3 estados limpia (`ESPERA`, `ACTIVO` y `ENFRIANDO`). Utiliza `millis()` para hacer transiciones luego de accionar el motor por 2 segundos y mantener un tiempo seguro de espera o enfriamiento por 1 segundo, sirviendo de base fundamental para cualquier módulo de reacción inmediata. Deberás ajustar estos tiempos y estados para tu caso.

### 🔴🟢🔵 `01_Sorting_RGB_SINCALIBRAR` (Módulo 01 - Clasificación)
Diseñado para el módulo encargado de separar y distribuir las bolitas tras detectar su color. Toma inspiración de técnicas avanzadas de promediado de hardware pero con una lógica condicional simplificada y directa de utilizar, usando la librería `ESP32Servo`.
* **Sensores/Actuadores:** Sensor de Color (TCS3200) y Servo Motor de desviación (MG90S/SG90).
* **Pines definidos:** 
  * TCS3200: `S0 = 8`, `S1 = 9`, `S2 = 11`, `S3 = 10`, `OUT = 12`
  * Servo Motor: `SERVO_PIN = 15`
* **Resumen y Uso:** Obtiene y promedia rápidamente cinco lecturas directas del sensor TCS3200 de los canales R, G, B y C. Mediante comprobaciones `if / else`, analiza cuál de los canales dominantes tiene el pico de mayor rebote de luz (es decir, el valor numérico más bajo). En función del color predominante percibido (Rojo, Verde o Azul), posiciona automáticamente el servo a un ángulo asignado (45°, 135°, 175° respectivamente) o vuelve a 90° si no percibe ningún cuerpo cerca. Úsalo como punto de partida en tu estación de clasificación calibrando primero los umbrales de detección generales según la luz de tu salón para encauzar el motor a la pista correcta.

### 🏃‍♂️💨 `02_Transportador_IR` (Módulos 02 al 04 - Transporte)
Ideal para los módulos intermedios que requieren activarse cuando reciben la bolita, transladarla y entregarla al siguiente equipo.
* **Sensores/Actuadores:** Sensor Infrarrojo (TCRT5000) y Motor DC (Vía Driver L298N).
* **Pines definidos:** 
  * `PIN_IR = 7`
  * L298N: `ENA = 10` (PWM), `IN1 = 11`, `IN2 = 12`
* **Resumen y Uso:** Permanece silente en el estado de `ESPERA` revisando el sensor infrarrojo. Al detectar que la bolita ya ha entrado a su trayecto, avanza hacia la fase `ACTIVO`, encendiendo rotatoriamente el motor e impulsando el recorrido por el tiempo configurado (`duracionTransporte`).

### 🔢📥 `03_Contador_Retorno` (Módulo 05 - Colector y Fin de Línea)
Programado como modelo para el componente final o de conteo. Ideal para agrupar cierto número de unidades y luego vaciar o abrir una barrera.
* **Sensores/Actuadores:** Sensor Infrarrojo (TCRT5000) y compuerta operada por Servo Motor.
* **Pines definidos:** 
  * `PIN_IR_CONTADOR = 34`
  * Compuerta / Servo Motor: `SERVO_PIN = 15`
* **Resumen y Uso:** Hace uso también de `ESP32Servo` para gobernar el barrido. El elemento clave de este script es el **control por flancos**, el cual no solo lee si el sensor tiene una bolita en frente o no, sino que registra *cuándo cambia el estado*, evitando así falsos conteos o lecturas dobles cuando la bolita cruza algo lento. Al llegar al límite acumulado, desplaza el servo permitiendo un vaciado físico y acciona un temporizador asíncrono para retornar la compuerta a su inicio.

---

## 🚀 Cómo empezar

1. Estudia los ejemplos simples de control provistos dentro de la carpeta `Referencias_clasesanteriores` si tienes dudas sobre conexiones de un solo componente.
2. Abre el archivo `.ino` correspondiente a la misión primaria de tu módulo en el IDE de Arduino.
3. ⚠️ **¡Importante!** Revisa la configuración en las primeras líneas de cada proyecto y **ajusta los números de pines** para que se alineen idénticamente con la forma en que cableaste y armaste tu protoboard a la ESP32.
4. Sube el código y compueba el Monitor Serie (en 115200 baudios) para poder leer los umbrales de color y tiempos en vivo.

---

## ⚠️ Disclaimer y Errores Comunes (¡LEER ANTES DE CONECTAR!)

Al ensamblar este tipo de circuitos con electrónica reactiva, motores, servos y placas ESP32, es normal encontrar detalles técnicos. Mantengan estas consideraciones cruciales presentes en sus proyectos:

1. **Validen todos sus Pines:** Los ejemplos proponen pines de manera ilustrativa y general, pero cada equipo tiene cableados diferentes. Asegúrese de que su pinout real y los pines del esquema en su IDE de Arduino concuerden y de que el pin que usa sí sirva (por ejemplo, algunos pines en ESP32 solo sirven de entrada, como el 34, 35, 36, 39).
2. **⚠️ Salidas de Voltaje en los ESP32-S3:** Tengan extremo cuidado en la versión ESP32-S3 que se entrega en clases. En estas placas **el pin de 5V por defecto a veces no recibe retroalimentación útil desde el USB / suele ser de solo salida** en algunos puentes; o en la mayoría de implementaciones no dispone de la corriente requerida para tolerar directamente las demandas de un servomotor. 
3. **La alimentación del ESP32 NUNCA debe mezclarse directamente al servo:** Los servos consumen grandes ráfagas y corren a 5V. La alimentación y la tierra de los actuadores y los motores DC provendrán obligatoriamente **del Puente H (o batería externa 5V)**, NO utilicen la salida de 3.3V ni colapsen el 5V exclusivo del microcontrolador. Recuerden enlazar todas las señales de Ground (Tierras) de sus componentes en común para que la lógica de 0 y 1 funcione.
4. **Lógica TTL y sensores:** Al igual que en la lectura anterior con TCRT5000, recuerden trabajar en el programa bajo "lógica inversa" si su sensor óptico así lo dicta. (Casi siempre es LOW = Detectado y HIGH = Vacío).
5. **No hay Servidor ni AP WiFi:** Los códigos de ejemplo entregados son de control físico autónomo. Todo código que integre APs o conectividad IP es un bloque de tareas adicional.

---

## 👨‍🏫 Autor

Alejandro Rebolledo  
arebolledo@udd.cl  

Curso: TET2026 — Universidad del Desarrollo  

## 📄 Licencia

CC BY-NC 4.0