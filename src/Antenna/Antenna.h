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
    
    // --- COORDENADAS ---
    double currentAzimuth;
    double currentElevation;
    double targetAzimuth;
    double targetElevation;
    
    // --- ESTADO ---
    AntennaState state;
    
    const double MAX_SPEED = 2.0; 
    const double MAX_ACCEL = 0.5;

    double currentVelAz;
    double currentVelEl;

    // --- CONTROL PID ---
    const double Kp = 1.2;
    const double Ki = 0.05;
    const double Kd = 0.3;

    double integralAz = 0.0;
    double integralEl = 0.0;
    double lastErrorAz = 0.0;
    double lastErrorEl = 0.0;

    double externalForceAz = 0.0;
    double externalForceEl = 0.0;

    // --- POSICIÓN EN TIERRA ---
    double posX; 
    double posY;

    // --- SEÑAL CIENTÍFICA ---
    double signalAmplitude;
    double signalPhase; 

    // --- SENSORES DE SALUD ---
    double motorTemp;
    double motorCurrent;

public:
    Antenna(int id);
    
    void setTarget(double az, double el);
    void reset();
    void applyExternalForce(double azForce, double elForce);
    void update(double dt); 
    void updateSensors(double dt);
    
    int getId() const;
    double getAzimuth() const;
    double getElevation() const;
    double getTargetAzimuth() const;
    double getTargetElevation() const;
    double getPosX() const;
    double getPosY() const;
    double getSignalAmp() const;
    double getSignalPhase() const;
    double getMotorTemp() const;
    double getMotorCurrent() const;
    AntennaState getState() const;
    std::string getStateString() const;

    void setSignal(double amp, double phase);
};

#endif
