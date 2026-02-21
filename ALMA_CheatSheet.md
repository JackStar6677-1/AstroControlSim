# Guía de Supervivencia: ALMA & Radioastronomía 📡✨

## 1. Conceptos Básicos de Coordenadas
En astronomía, necesitamos decirle a la antena "dónde mirar". Usamos coordenadas celestes, pero la antena física se mueve en **Azimut** y **Elevación**.

### Azimut (Az) - "La Brújula" 🧭
- **Qué es**: El ángulo horizontal.
- **Rango**: 0° a 360°.
- **Ejemplo**:
    - 0° = Norte
    - 90° = Este
    - 180° = Sur
    - 270° = Oeste
- **En código**: `currentAzimuth`

### Elevación (El) - "La Altura" 📐
- **Qué es**: El ángulo vertical desde el horizonte hacia el cielo.
- **Rango**: 0° a 90°.
- **Ejemplo**:
    - 0° = Horizonte (mirando al suelo)
    - 90° = Cénit (mirando directo arriba a tu cabeza)
- **En código**: `currentElevation`

> **Nota**: Las antenas reales tienen "límites de hardware" (ej: no pueden bajar de 2° para no chocar).

---

## 2. Polarización (Polarity) 〰️
La luz (ondas de radio) vibra.
- **Qué es**: La orientación de la onda eléctrica.
- **Tipos**: Horizontal (H), Vertical (V), o Circular (L/R).
- **En ALMA**: Los receptores captan **dos polarizaciones** simultáneamente (X e Y) para no perder datos.
- **Simulación**: Más adelante simularemos esto generando dos flujos de datos de ruido.

---

## 3. Comandos Linux / ALMA Esenciales
En la sala de control (OSF) usarás Linux. Aquí tu "traductor":

| Tu Comando (Windows/Sim) | Comando Real (Linux/ALMA) | Descripción |
| :--- | :--- | :--- |
| `dir` | `ls -l` | Listar archivos |
| `cd carpetas` | `cd carpetas` | Entrar a carpeta |
| `python script.py` | `./script.py` | Ejecutar script (si tiene permisos) |
| `taskkill ...` | `kill -9 <PID>` | Matar un proceso trabado |
| `AstroControlSim.exe` | `systemctl start antenna-service` | Iniciar servicio de antena |
| `type archivo.txt` | `cat archivo.txt` | Ver contenido de archivo |
| `ping localhost` | `ping antenna-42` | Ver si una antena responde |
| `ssh pablo@server` | `ssh operator@alma-osf` | Conectarse al servidor central |

---

## 4. "Big Data" y Filtrado 🌊
Las antenas de ALMA generan **Tetra-bytes** de datos por segundo.
- **Correlador**: Es una supercomputadora que recibe las señales de las 66 antenas y las *multiplica* entre sí.
- **Filtrado**:
    - **Paso 1 (Hardware)**: La antena solo escucha una frecuencia específica (ej: 230 GHz).
    - **Paso 2 (Software)**: Se promedia la señal (Integración). En vez de guardar 1 millón de datos por segundo, guardamos el promedio cada 1 segundo.
    - **Simulación**: Crearemos un "Generador de Ruido" que escupe números aleatorios (señal) y un script que los promedia.
