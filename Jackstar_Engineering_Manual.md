# Jackstar Observatory: Engineering Manual 🛠️⚡

Este manual documenta el conocimiento técnico crítico invertido en el desarrollo de **Jackstar Observatory**, un simulador de antenas de grado profesional.

## 1. Control de Movimiento: El Ciclo PID
A diferencia de las animaciones simples, las antenas de Jackstar utilizan un **Controlador Proporcional-Integral-Derivativo (PID)**.

- **Proporcional (P)**: Corrige el error actual. Si la antena está lejos, acelera fuerte.
- **Integral (I)**: Corrige el error acumulado. Si hay un viento constante que empuja la antena, el término 'I' aumenta la fuerza hasta vencer la resistencia.
- **Derivativo (D)**: Predice el futuro. Evita que la antena "rebote" (overshoot) al llegar al objetivo, frenando justo a tiempo.

### Implementación en Jackstar Engine:
```cpp
double pidOutput = (Kp * error) + (Ki * integral) + (Kd * derivative);
totalAccel = pidOutput + externalForce; // El PID lucha contra el viento
```

## 2. Protocolo de Red: Binario vs Texto
Para lograr la latencia mínima necesaria en la telemetría de 50 antenas, Jackstar evita JSON o XML. Usamos **Binary Packets** de 70 bytes.

- **Eficiencia**: Un paquete binario es ~10 veces más pequeño que su equivalente en JSON.
- **Alineación**: Usamos `#pragma pack(push, 1)` para asegurar que C++ no añada "padding" extra, permitiendo que Python lea los bytes directamente.

## 3. Concurrencia y Paralelismo
El servidor C++ utiliza un modelo de **Multi-threading Asíncrono**:
- Un hilo principal gestiona la física de las 50 antenas.
- Un hilo de red acepta conexiones de clientes (operadores).
- Cada cliente nuevo recibe su propio hilo para no bloquear la simulación.

## 4. El Plano UV (Interferometría)
La resolución de Jackstar no viene de una antena grande, sino del **Plano UV**. Calculamos la diferencia de fase entre pares de antenas:
$$ \Delta \Phi = \frac{2\pi}{\lambda} (B \cdot S) $$
Esto permite que Jackstar funcione como un solo telescopio gigante de cientos de metros de diámetro.

---
*Documento propiedad de Jackstar Observatory Engineering.*
