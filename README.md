# Jackstar Observatory 📡✨

A high-performance radio telescope array simulator developed in C++ and Python. This project simulates the control systems, network protocols, and data visualization used in professional astronomical observatories like ALMA.

## Key Features 🚀

- **Jackstar Engine (C++)**: Realistic 50-antenna array simulation with inertia, acceleration, and trapezoidal velocity profiles.
- **Custom TCP/IP Protocol**: Binary network layer for high-efficiency command dispatch and telemetry retrieval.
- **Professional Dashboard**: Real-time visualization using Python (`matplotlib`) and astronomical data processing (`astropy`).
- **Control PID**: Proportional-Integral-Derivative control system to counteract environmental disturbances.
- **Interferometry Simulation**: Real-time complex signal generation (Phase/Amplitude) and UV-plane baseline visualization.
- **Chaos Engineering**: Built-in fault injection and an automated "Self-Healing" fault handler to simulate site operations.

## Architecture 🏛️

```mermaid
graph TD
    A[Jackstar Physics Engine] <-->|TCP/Binary| B[Antenna Server]
    B <-->|Telemetry/Commands| C[Python Dashboard]
    B <-->|Control Patterns| D[Spiral Scan Controller]
    B <-->|Automatic Reset| E[Fault Handler]
    C -->|FITS Imaging| F[Real Sky Data]
```

## Getting Started 🛠️

### Prerequisites
- **C++**: GCC/MinGW-w64 (support for C++17)
- **Python 3.12+**: `matplotlib`, `astropy`, `numpy`, `websockets`
- **Make**: To build the C++ server.

### Running the Simulation
1. **Build the Server**:
   ```powershell
   mingw32-make
   ```
2. **Launch the Components** (in separate terminals):
   - **Physics Engine**: `.\build\JackstarObs.exe`
   - **Dashboard**: `python scripts/dashboard.py`
   - **Spiral Scan**: `python scripts/spiral_scan.py`
   - **Fault Handler**: `python scripts/fault_handler.py`

## Learning Objectives 🎓
- **Systems Programming**: Low-level C++, PID control, and binary protocols.
- **Distributed Systems**: Client-server architecture and asynchronous telemetry.
- **Astronomy Operations**: Interferometry (UV plane), FITS visibility, and autonomous site management.

---
*Created as part of the Jackstar Observatory Engineering initiative.*
