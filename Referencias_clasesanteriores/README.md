# 🔧 Control de Motor DC con L298N — Versión Didáctica

Este proyecto presenta una versión simplificada de control de un motor DC usando un driver L298N, enfocada en el aprendizaje progresivo dentro del curso TET2026.

---

## 🎯 Objetivo

Comprender los conceptos fundamentales de:

- Control de dirección (IN1 / IN2)
- Control de velocidad mediante PWM
- Separación de lógica en funciones (`adelante`, `atras`, `detener`)
- Uso de `analogWrite` como aproximación simple a señales PWM

---

## 🧠 Contexto pedagógico

Este código proviene de una versión más compleja que incluía:

- Control vía WiFi (ESP32 en modo Access Point)
- Servidor web
- Interfaz HTML para control remoto

Para esta etapa del curso, se eliminó toda esa complejidad para centrarse exclusivamente en:

**Cómo se mueve realmente el motor**

---

## ⚙️ Conexión básica

| Pin ESP32 | L298N |
|----------|------|
| 10       | ENA  |
| 11       | IN1  |
| 12       | IN2  |

- ENA → controla velocidad (PWM)
- IN1 / IN2 → controlan dirección

⚠️ Importante:
- Fuente externa para el motor
- GND común entre ESP32 y L298N

---

## 🔄 Lógica de control

### Dirección

| IN1 | IN2 | Resultado |
|-----|-----|----------|
| 1   | 0   | Adelante |
| 0   | 1   | Atrás |
| 0   | 0   | Detenido |

### Velocidad

Se controla con:

```
analogWrite(ENA, valor);
```

- 0 → motor detenido  
- 255 → velocidad máxima  
- valores intermedios → velocidad proporcional  

---

## ⚡ PWM: qué está pasando realmente

El motor no recibe un voltaje analógico real.

En su lugar:
- Se enciende y apaga muy rápido (PWM)
- El motor promedia esa señal
- Se comporta como si tuviera menos voltaje

---

## 🧪 ¿Por qué usar analogWrite?

### Ventajas en docencia

- Simple
- Intuitivo
- Menos carga cognitiva
- Permite enfocarse en el fenómeno físico

---

## 🔬 analogWrite vs PWM real (ledc en ESP32)

### analogWrite (simplificado)

```
analogWrite(pin, 128);
```

- Fácil de usar
- No configurable
- Ideal para enseñanza inicial

---

### ledc (PWM real)

```
ledcSetup(channel, freq, resolution);
ledcAttachPin(pin, channel);
ledcWrite(channel, duty);
```

Ventajas:
- Control de frecuencia
- Control de resolución
- Mayor precisión

---

## 🧠 ¿Cuál es mejor?

👉 ledc es más eficiente porque usa hardware PWM real.

---

## 🎓 Enfoque del curso

1. analogWrite → entender PWM  
2. ledc → control preciso  
3. WiFi → control remoto  

---

## ⚠️ Consideraciones

- L298N no es eficiente
- Puede fallar a bajas velocidades
- Alimentación correcta es clave

---

## 👨‍🏫 Autor

Alejandro Rebolledo  
arebolledo@udd.cl  

Curso: TET2026 — Universidad del Desarrollo  

---

## 📄 Licencia

CC BY-NC 4.0
