# ⚙️ Ejemplos Prácticos - Clase 12: Sensores y Lógica Reactiva

Este repositorio presenta códigos de ejemplo para el curso TET2026, enfocados en la integración de módulos autónomos que aplican conceptos de Lógica Reactiva y Máquinas de Estado.

---

## 🎯 Objetivo

Ayudar a los equipos a lograr que sus módulos detecten bolitas y actúen de forma **autónoma**, realizando múltiples tareas simultáneas de forma eficiente.

---

## 🧠 Contexto pedagógico y Conceptos Clave

Para que los módulos funcionen correctamente (como leer un sensor mientras un motor está girando o esperando un tiempo de vaciado), este código aplica dos principios fundamentales enseñados en la Clase 12:

1. **Adiós al `delay()`:** El uso de `delay()` bloquea el microcontrolador. En su lugar, utilizamos la función `millis()` para medir el tiempo transcurrido sin detener la ejecución del código.
2. **Máquinas de Estado:** Organizamos el comportamiento de los módulos en fases lógicas (ej. *ESPERA*, *ACTIVO*, *RECOLECTANDO*). El sistema evalúa constantemente en qué estado se encuentra y qué condiciones lo hacen cambiar al siguiente.

---

## 📂 Estructura del Repositorio

Los códigos están divididos según el tipo de módulo en el que estés trabajando:

### 🧩 `00_Maquina_de_Estados` (Estructura Base)
Es la plantilla estructural ideal para comprender el funcionamiento asíncrono y comenzar a programar de cero.
* **Sensores/Actuadores:** Sensor Infrarrojo (TCRT5000) y Motor DC (con Driver L298N).
* **Propósito:** Demuestra una máquina de 3 estados limpia (`ESPERA`, `ACTIVO` y `ENFRIANDO`). Utiliza `millis()` para transicionar luego de actuar el motor por 2 segundos y mantener un tiempo de seguridad de 1 segundo, sirviendo de base para cualquier módulo reactivo.

### 🔴🟢🔵 `01_Sorting_RGB` (Módulo 01 - Clasificación)
Diseñado para el módulo encargado de separar las bolitas por color.
* **Sensores/Actuadores:** Sensor de Color (TCS3200) y Servo Motor (MG90S).
* **Propósito:** Lee colores utilizando la librería oficial sin bloquear el sistema. Cuando detecta el umbral de un color específico (ej. Rojo), activa el servo para desviar la bolita hacia el carril correcto y luego regresa.

### 🏃‍♂️💨 `02_Transportador_IR` (Módulos 02 al 04 - Transporte)
Ideal para los módulos que reciben la bolita, la trasladan y la entregan al siguiente equipo.
* **Sensores/Actuadores:** Sensor Infrarrojo (TCRT5000) y Motor DC (con Driver L298N usando `ENA`, `IN1`, `IN2`).
* **Propósito:** Implementa una máquina de estados básica. Permanece en estado de `ESPERA` hasta detectar una bolita con el IR. Al hacerlo pasa a `ACTIVO`, moviendo el motor progresivamente regulado por PWM durante el tiempo configurado con `millis()`.

### 🔢📥 `03_Contador_Retorno` (Módulo 05 - Colector y Fin de Línea)
Pensado para el módulo final que debe agrupar las bolitas antes de devolverlas al inicio.
* **Sensores/Actuadores:** Sensor Infrarrojo (TCRT5000) y Servo Motor (como compuerta).
* **Propósito:** Implementa **control por flancos** para contar el ingreso de las bolitas evitando lecturas duplicadas. Al llegar al límite, acciona el servo para abrir la compuerta, se vacía mediante una máquina de estados asíncrona temporalizada y reinicia el ciclo.

---

## 🚀 Cómo utilizar estos códigos

1. Estudia los ejemplos provistos en la carpeta secundaria `Referencias_clasesanteriores` si tienes dudas sobre conexiones anteriores.
2. Abre el archivo `.ino` correspondiente a tu módulo en el IDE de Arduino.
3. ⚠️ **¡Importante!** Revisa la configuración al inicio de cada código y **ajusta los números de pines** para que coincidan con la forma en que armaste tu protoboard.
4. Sube el código y calibra umbrales de luz, tiempos y colores reales en el monitor serie.

---

## 👨‍🏫 Autor

Alejandro Rebolledo  
arebolledo@udd.cl  

Curso: TET2026 — Universidad del Desarrollo  

---

## 📄 Licencia

CC BY-NC 4.0