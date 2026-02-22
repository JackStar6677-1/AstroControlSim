#include "Antenna.h"
#include <cmath>
#include <algorithm>
#include <iostream>

Antenna::Antenna(int id) : id(id), currentAzimuth(0.0), currentElevation(0.0), 
                           targetAzimuth(0.0), targetElevation(0.0), state(AntennaState::IDLE),
                           currentVelAz(0.0), currentVelEl(0.0), integralAz(0.0), integralEl(0.0),
                           lastErrorAz(0.0), lastErrorEl(0.0), externalForceAz(0.0), externalForceEl(0.0),
                           signalAmplitude(0.0), signalPhase(0.0), motorTemp(25.0), motorCurrent(0.0) {
    // Layout en espiral o grilla para las posiciones en tierra
    posX = (id % 7) * 15.0 - 45.0; 
    posY = (id / 7) * 15.0 - 45.0;
}

void Antenna::setTarget(double az, double el) {
    targetAzimuth = az;
    targetElevation = el;
    state = AntennaState::SLEWING;
    std::cout << "[Antenna " << id << "] New Target: Az=" << az << " El=" << el << std::endl;
}

void Antenna::reset() {
    if (state == AntennaState::FAULT) {
        state = AntennaState::IDLE;
        integralAz = 0.0;
        integralEl = 0.0;
        lastErrorAz = 0.0;
        lastErrorEl = 0.0;
        std::cout << "[Antenna " << id << "] RESET command received. Fault cleared." << std::endl;
    }
}

void Antenna::applyExternalForce(double azForce, double elForce) {
    externalForceAz = azForce;
    externalForceEl = elForce;
}

void Antenna::update(double dt) {
    if (state == AntennaState::FAULT) return;

    // 1. Random Fault Injection
    if (std::rand() % 10000 < 5) { // 0.05% chance
        state = AntennaState::FAULT;
        currentVelAz = 0.0;
        currentVelEl = 0.0;
        std::cout << "[Antenna " << id << "] CRITICAL FAULT! Motors jammed." << std::endl;
        return;
    }

    // 2. Control Logic
    auto updateAxisPID = [&](double &current, double target, double &velocity, double &integral, double &lastError, double force) {
        double error = target - current;
        
        // Anti-windup (limit integral)
        integral += error * dt;
        integral = std::max(std::min(integral, 5.0), -5.0);
        
        double derivative = (error - lastError) / dt;
        lastError = error;

        // PID Output (Desired acceleration/torque)
        double pidOutput = (Kp * error) + (Ki * integral) + (Kd * derivative);

        // Apply Forces (Disturbances like wind + Motor torque)
        double totalAccel = pidOutput + force;

        // Limit acceleration
        totalAccel = std::max(std::min(totalAccel, MAX_ACCEL), -MAX_ACCEL);

        // Update Velocity (V = V0 + a*t)
        velocity += totalAccel * dt;
        
        // Limit Speed
        velocity = std::max(std::min(velocity, MAX_SPEED), -MAX_SPEED);

        // Update Position (X = X0 + V*t)
        current += velocity * dt;
    };

    updateAxisPID(currentAzimuth, targetAzimuth, currentVelAz, integralAz, lastErrorAz, externalForceAz);
    updateAxisPID(currentElevation, targetElevation, currentVelEl, integralEl, lastErrorEl, externalForceEl);

    // Dynamic State Transition
    double totalError = std::abs(targetAzimuth - currentAzimuth) + std::abs(targetElevation - currentElevation);
    if (totalError < 0.05 && std::abs(currentVelAz) < 0.01 && std::abs(currentVelEl) < 0.01) {
        state = AntennaState::TRACKING;
    } else {
        state = AntennaState::SLEWING;
    }
}

void Antenna::updateSensors(double dt) {
    // 1. MOTOR TEMPERATURE
    // Heats up based on mechanical work (vel * torque/friction)
    double speedSq = currentVelAz * currentVelAz + currentVelEl * currentVelEl;
    motorTemp += speedSq * 0.5 * dt; 
    
    // Cooling (Newton's Law of Cooling towards ambient 25C)
    motorTemp -= (motorTemp - 25.0) * 0.05 * dt;

    // 2. MOTOR CURRENT
    // Proportional to speed and disturbances (wind)
    if (state == AntennaState::IDLE || state == AntennaState::TRACKING) {
        motorCurrent = 0.5 + (std::abs(externalForceAz) + std::abs(externalForceEl)) * 2.0;
    } else {
        motorCurrent = 2.0 + std::sqrt(speedSq) * 3.0; // SLEWING draw
    }

    // Add some sensor noise
    motorTemp += (std::rand() % 100) / 1000.0;
    motorCurrent += (std::rand() % 100) / 1000.0;
    
    // FAULT SIGNATURE: If FAULT, current spikes then drops (blown fuse style simulation)
    if (state == AntennaState::FAULT) {
        motorCurrent = 0.0; 
        motorTemp += 0.1; // Residual heat
    }
}

double Antenna::getAzimuth() const { return currentAzimuth; }
double Antenna::getElevation() const { return currentElevation; }
double Antenna::getTargetAzimuth() const { return targetAzimuth; }
double Antenna::getTargetElevation() const { return targetElevation; }
double Antenna::getPosX() const { return posX; }
double Antenna::getPosY() const { return posY; }
double Antenna::getSignalAmp() const { return signalAmplitude; }
double Antenna::getSignalPhase() const { return signalPhase; }
double Antenna::getMotorTemp() const { return motorTemp; }
double Antenna::getMotorCurrent() const { return motorCurrent; }
int Antenna::getId() const { return id; }
AntennaState Antenna::getState() const { return state; }

void Antenna::setSignal(double amp, double phase) {
    signalAmplitude = amp;
    signalPhase = phase;
}

std::string Antenna::getStateString() const {
    switch (state) {
        case AntennaState::IDLE: return "IDLE";
        case AntennaState::SLEWING: return "SLEWING";
        case AntennaState::TRACKING: return "TRACKING";
        case AntennaState::FAULT: return "FAULT";
        default: return "UNKNOWN";
    }
}
