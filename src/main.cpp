#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include "Antenna/Antenna.h"
#include "Network/AntennaServer.h"

int main() {
    std::cout << "Jackstar Observatory: Starting LARGE ARRAY Simulation (50 Antennas)..." << std::endl;

    std::vector<Antenna> array;
    const int NUM_ANTENNAS = 50;

    // Initialize the array
    for (int i = 0; i < NUM_ANTENNAS; ++i) {
        array.emplace_back(i + 1); // IDs 1 to 50
    }

    // Start Network Server
    AntennaServer server(&array);
    if (!server.start(9000)) {
        std::cerr << "Failed to start server!" << std::endl;
        return 1;
    }

    std::cout << "[Controller] dispatching coordinates..." << std::endl;
    // ... random init logic removed to let the user control it via network, 
    // or keep it for initial visual? Let's keep it for "alive" look but minimal move.
    
    // Simulation Loop
    std::cout << "System Ready. Listening on Port 9000..." << std::endl;
    
    while (true) { // Infinite loop, Ctrl+C to stop
        double dt = 0.1;

        // 2. Global Wind Simulation (Disturbance)
        static double windPhase = 0.0;
        windPhase += dt * 0.5;
        double windIntensity = (sin(windPhase) * 0.2) + (rand() % 100 / 500.0); // Sine wave + jitter
        double windAngle = windPhase * 0.1; // Slowly rotating wind
        
        double windForceAz = cos(windAngle) * windIntensity;
        double windForceEl = sin(windAngle) * windIntensity;

        for (auto& ant : array) {
            ant.applyExternalForce(windForceAz, windForceEl);
            ant.update(dt);
            ant.updateSensors(dt);

            // 3. SCIENCE SIMULATION: Update Complex Signal
            // Phase is relative to Az/El and antenna ground position
            double lambda = 0.001; // Simulating 1mm wavelength (ALMA band)
            double waveK = 2 * 3.14159 / lambda;
            
            // Project Az/El to 2D unit vector
            double azRad = ant.getAzimuth() * (3.14 / 180.0);
            double elRad = ant.getElevation() * (3.14 / 180.0);
            double sx = cos(elRad) * sin(azRad);
            double sy = cos(elRad) * cos(azRad);

            // Geometric Phase Delay: phi = k * (B dot S)
            double phase = waveK * (ant.getPosX() * sx + ant.getPosY() * sy);
            
            // Amplitude from FITS (Placeholder for now, or just use a constant for testing)
            ant.setSignal(1.0, fmod(phase, 2 * 3.14159));
        }

        static int tick = 0;
        if (tick++ % 10 == 0) { 
             std::cout << "[Jackstar] ANT-01: " << array[0].getStateString() << " Az:" << array[0].getAzimuth() 
                       << " | Phase:" << array[0].getSignalPhase() << "\r";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Unreachable in this infinite loop design, but for completeness:
    server.stop();
    return 0;
}
