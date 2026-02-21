#include "Antenna.h"
#include <cmath>
#include <algorithm>
#include <iostream>

Antenna::Antenna(int id) : id(id), currentAzimuth(0.0), currentElevation(0.0), 
                           targetAzimuth(0.0), targetElevation(0.0), state(AntennaState::IDLE),
                           currentVelAz(0.0), currentVelEl(0.0) {}

void Antenna::setTarget(double az, double el) {
    targetAzimuth = az;
    targetElevation = el;
    state = AntennaState::SLEWING;
    std::cout << "[Antenna " << id << "] New Target: Az=" << az << " El=" << el << std::endl;
}

void Antenna::reset() {
    if (state == AntennaState::FAULT) {
        state = AntennaState::IDLE;
        std::cout << "[Antenna " << id << "] RESET command received. Fault cleared." << std::endl;
    }
}

void Antenna::update(double dt) {
    if (state == AntennaState::FAULT) return;

    // 1. Random Fault Injection (0.01% chance per tick ~ 1% per 1000 ticks)
    // Using a simple static random generator for demonstration
    if (std::rand() % 10000 < 5) { // 0.05% chance
        state = AntennaState::FAULT;
        currentVelAz = 0.0;
        currentVelEl = 0.0;
        std::cout << "[Antenna " << id << "] CRITICAL FAULT! Motors jammed." << std::endl;
        return;
    }

    if (state == AntennaState::IDLE) {
        // Even in IDLE, check if we need to move (if target changed but state didn't switch?)
        // In our logic SetTarget switches to SLEWING.
        if (std::abs(targetAzimuth - currentAzimuth) > 0.1 || std::abs(targetElevation - currentElevation) > 0.1) {
             state = AntennaState::SLEWING;
        } else {
             return;
        }
    }

    // 2. Simulación Física (Physics Engine)
    // Usamos un perfil de velocidad trapezoidal: Acelerar -> Mantener -> Frenar.
    auto updateAxis = [&](double &current, double target, double &velocity) {
        double error = target - current; // Distancia al objetivo
        
        // Calcular frenado: V^2 = 2*a*d
        // Si vamos muy rápido y estamos cerca, ¡hay que frenar ya!
        double brakingDist = (velocity * velocity) / (2 * MAX_ACCEL);
        double direction = (error > 0) ? 1.0 : -1.0;
        
        double targetVel = 0.0;
        if (std::abs(error) > 0.1) {
            // Si estamos lejos (más allá de la distancia de frenado), Aceleramos (MAX_SPEED)
            if (std::abs(error) > brakingDist + 0.1) {
                 targetVel = MAX_SPEED * direction;
            } else {
                 // Estamos cerca: Reducir velocidad suavemente (Raíz cuadrada = curva suave)
                 targetVel = sqrt(2 * MAX_ACCEL * std::abs(error)) * direction;
            }
        } else {
            // Llegamos! (Detener)
            targetVel = 0.0;
            current = target; // Forzar posición exacta para evitar "vibración"
            velocity = 0.0;
            return; 
        }

        // Aplicar Aceleración (Inercia)
        // No podemos cambiar de velocidad instantáneamente (F=ma)
        double velError = targetVel - velocity;
        double accelStep = MAX_ACCEL * dt; // Cuánto podemos cambiar la velocidad en este tick
        
        if (std::abs(velError) > accelStep) {
            velocity += (velError > 0 ? accelStep : -accelStep);
        } else {
            velocity = targetVel;
        }
        
        // Aplicar Velocidad a la Posición (Integración)
        current += velocity * dt;
    };

    updateAxis(currentAzimuth, targetAzimuth, currentVelAz);
    updateAxis(currentElevation, targetElevation, currentVelEl);

    // Check state transition
    if (std::abs(targetAzimuth - currentAzimuth) < 0.1 && std::abs(targetElevation - currentElevation) < 0.1 &&
        std::abs(currentVelAz) < 0.01 && std::abs(currentVelEl) < 0.01) {
        state = AntennaState::TRACKING;
    } else {
        state = AntennaState::SLEWING;
    }
}

double Antenna::getAzimuth() const { return currentAzimuth; }
double Antenna::getElevation() const { return currentElevation; }
int Antenna::getId() const { return id; }
AntennaState Antenna::getState() const { return state; }

std::string Antenna::getStateString() const {
    switch (state) {
        case AntennaState::IDLE: return "IDLE";
        case AntennaState::SLEWING: return "SLEWING";
        case AntennaState::TRACKING: return "TRACKING";
        case AntennaState::FAULT: return "FAULT";
        default: return "UNKNOWN";
    }
}
