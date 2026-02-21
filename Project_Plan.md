# Proyecto: AstroControl Sim 📡
**Objetivo**: Simular un sistema de control de radio-telescopios distribuido (estilo ALMA) para aprender C++, Sistemas Distribuidos y Tiempo Real.

## Arquitectura del Sistema

El proyecto tendrá 3 módulos principales que se comunican entre sí:

### 1. El "Antenna Simulator" (C++ | Backend)
Simula el hardware de una antena.
- **Conceptos Clave**: Clases, Herencia, State Machines (Máquinas de Estado), Multithreading.
- **Funcionalidad**:
    - Tiene estados: IDLE (Reposo), SLEWING (Moviéndose), TRACKING (Siguiendo objeto), ERROR.
    - Simula sensores: Temperatura, Azimut, Elevación.
    - Recibe comandos por red (TCP/UDP) para moverse a coordenadas (x, y).

### 2. El "Correlator / Central Control" (C++ | Middleware)
El cerebro que coordina las antenas.
- **Conceptos Clave**: Sockets (Networking), Punteros, Gestión de Memoria, Concurrencia.
- **Funcionalidad**:
    - Se conecta a múltiples instancias de "Antenas".
    - Envía comandos sincronizados ("¡Moverse todas a la estrella X!").
    - Recibe telemetría y detecta fallos.

### 3. El "SciOps Dashboard" (Python | Frontend)
La interfaz para los astrónomos.
- **Conceptos Clave**: Interoperabilidad (C++ <-> Python), Visualización de Datos, Sockets.
- **Funcionalidad**:
    - Panel gráfico que muestra la posición de las antenas en tiempo real.
    - Gráficos de temperatura y estado (librerías `matplotlib` o `dash`).

---

## Roadmap de Aprendizaje

### Fase 1: El Motor en C++ (La Antena)
- [ ] Configurar entorno C++ (Compilador).
- [ ] Crear la clase `Antenna` con propiedades físicas simuladas.
- [ ] Implementar un "Thread" (hilo) que actualice la posición cada 100ms (simulando movimiento físico).

### Fase 2: Comunicación (Networking)
- [ ] Implementar un servidor TCP simple en la Antena.
- [ ] Crear un protocolo binario simple (ej: byte de inicio + ID comando + datos).

### Fase 3: El Controlador
- [ ] Crear el cliente que se conecta a la antena y le manda órdenes.

### Fase 4: Integración Python
- [ ] Crear un script Python que escuche los datos y los grafique.

## ¿Por qué este proyecto?
1.  **C++**: Es el lenguaje de la infraestructura científica.
2.  **Tiempo Real**: Simular el movimiento requiere pensar en "tiempos" y "estados", no solo en "request-response" como en la web.
3.  **Sistemas Distribuidos**: Entenderás cómo comunicarse entre procesos, esencial para ALMA/ACS.
