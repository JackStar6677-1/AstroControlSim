# AstroControlSim: Resumen del Proyecto 🚀🔭

Hemos construido un **Simulador de Observatorio Profesional** desde cero. Aquí está el detalle técnico y conceptual de lo que lograse:

## 1. El "Cerebro" Físico (C++) 🧠
- **Qué es**: Un servidor de alto rendimiento escrito en C++.
- **Logro**: Simula 50 antenas de radio trabajando en paralelo.
- **Física Realista**:
    - Implementamos **Inercia**: Las antenas aceleran y frenan suavemente (perfil trapezoidal de velocidad), no se mueven instantáneamente. Esto es crítico en máquinas de 100 toneladas.
    - **Estados de Control**: `IDLE` (Quieto) -> `SLEWING` (Moviéndose) -> `TRACKING` (Siguiendo objetivo).

## 2. La Red Neuronal (TCP/IP) 🌐
- **Qué es**: Un protocolo de comunicación binaria personalizado.
- **Logro**: Permite controlar el observatorio desde cualquier lenguaje (Python en tu caso, pero podría ser Java o C#).
- **Protocolo**: Diseñamos paquetes de datos (`struct Packet`) para enviar comandos (`CMD_MOVE`, `CMD_RESET`) y recibir telemetría (posición, estado) en tiempo real.

## 3. El Panel de Control (Python + Matplotlib) 📊
- **Qué es**: `dashboard.py` y `spiral_scan.py`.
- **Logro**: Visualización en vivo de todo el array.
- **Datos Reales**: Conectamos el simulador al cielo real usando **Astropy**.
    - Cargamos una imagen FITS real (Nebulosa Cabeza de Caballo).
    - Mapeamos las coordenadas de las antenas sobre la imagen.
    - **Simulador de Señal**: Si una antena "apunta" a una zona brillante de la imagen, el dashboard detecta "señal". ¡Es un telescopio virtual funcional!

## 4. Ingeniería de Fiabilidad (Chaos Engineering) 💥
- **Qué es**: "Modo Caos" y `fault_handler.py`.
- **Logro**: Simulamos fallas de hardware aleatorias.
    - El servidor genera errores (`FAULT`) al azar.
    - Creamos un **Operador Automático** (`fault_handler.py`) que monitorea la red, detecta las fallas y las repara automáticamente enviando comandos de reinicio (`CMD_RESET`). Así funcionan los observatorios modernos: auto-reparación.

---

## Cómo Ejecutar tu Observatorio 🎮

Necesitas 4 terminales abiertas:

1.  **Simulación Física (Server)**:
    ```powershell
    .\build\AstroControlSim.exe
    ```
2.  **Visualización (Dashboard)**:
    ```powershell
    python scripts/dashboard.py
    ```
3.  **Controlador (Patrón de Búsqueda)**:
    ```powershell
    python scripts/spiral_scan.py
    ```
4.  **Operador Automático (Reparación)**:
    ```powershell
    python scripts/fault_handler.py
    ```

¡Felicidades! Has pasado de "Hello World" a un sistema distribuido complejo con física, redes y datos astronómicos reales. 🌟
