#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include "Antenna/Antenna.h"
#include "Network/AntennaServer.h"

int main() {
    std::cout << "AstroControlSim: Starting LARGE ARRAY Simulation (50 Antennas)..." << std::endl;

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
        for (auto& ant : array) {
            ant.update(0.1); 
        }

        // Print status of Antenna 1 to see if it moves
        // Only print if moving to avoid spam? 
        // Or just print periodically
        static int tick = 0;
        if (tick++ % 10 == 0) { // Every 1 second
             std::cout << "ANT-01: " << array[0].getStateString() << " Az:" << array[0].getAzimuth() 
                       << " | ANT-50: " << array[49].getStateString() << " Az:" << array[49].getAzimuth() << "\r";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Unreachable in this infinite loop design, but for completeness:
    server.stop();
    return 0;
}
