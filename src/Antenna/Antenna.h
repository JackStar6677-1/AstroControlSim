#ifndef ANTENNA_H
#define ANTENNA_H

#include <string>

enum class AntennaState {
    IDLE,
    SLEWING,
    TRACKING,
    FAULT
};

class Antenna {
private:
    int id;
    
    // --- COORDENADAS (Coordinates) ---
    // Azimut (Az): Ángulo horizontal (0-360 grados). 0=Norte, 90=Este.
    double currentAzimuth;
    
    // Elevación (El): Ángulo vertical (0-90 grados). 0=Horizonte, 90=Cénit (Arriba).
    double currentElevation;
    
    double targetAzimuth;
    double targetElevation;
    
    // --- ESTADO (State) ---
    // IDLE: Quieta, esperando comandos.
    // SLEWING: Moviéndose hacia el objetivo.
    // TRACKING: Siguiendo el objetivo (ej: una estrella) con precisión.
    // FAULT: Error crítico (motores trabados, sobrecalentamiento).
    AntennaState state;
    
    // Physical constraints (degrees per second)
    const double MAX_SPEED = 2.0; 
    const double MAX_ACCEL = 0.5;

    double currentVelAz;
    double currentVelEl; 

public:
    Antenna(int id);
    
    // Commands
    void setTarget(double az, double el);
    void reset();
    void update(double dt); // dt = delta time in seconds
    
    // Telemetry
    int getId() const;
    double getAzimuth() const;
    double getElevation() const;
    AntennaState getState() const;
    std::string getStateString() const;
};

#endif
