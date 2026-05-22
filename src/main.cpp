#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <cmath>
#include <atomic>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Antenna/Antenna.h"
#include "Network/AntennaServer.h"

// Struct for the binary Visibility Stream (Port 9002)
#pragma pack(push, 1)
struct VisibilityRecord {
    uint32_t ant1;
    uint32_t ant2;
    double u;
    double v;
    double w;
    double realVis;
    double imagVis;
};
#pragma pack(pop)

// Worker thread for the visibility streamer on port 9002
void startVisibilityStreamer(std::vector<Antenna>* antennas, std::atomic<bool>& running) {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "[VisStream] Failed to create socket: " << WSAGetLastError() << std::endl;
        return;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(9002);

    char value = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

    if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cerr << "[VisStream] Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[VisStream] Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        return;
    }

    std::cout << "[VisStream] Visibility Stream server listening on 127.0.0.1:9002..." << std::endl;

    std::vector<SOCKET> clients;
    std::mutex clientMutex;

    // Thread to accept visibility connections
    std::thread acceptThread([serverSocket, &clients, &clientMutex, &running]() {
        while (running) {
            fd_set readFDs;
            FD_ZERO(&readFDs);
            FD_SET(serverSocket, &readFDs);
            timeval tv = { 1, 0 }; // 1 second timeout
            int selectRes = select(0, &readFDs, NULL, NULL, &tv);
            if (selectRes > 0) {
                SOCKET clientSocket = accept(serverSocket, NULL, NULL);
                if (clientSocket != INVALID_SOCKET) {
                    std::lock_guard<std::mutex> lock(clientMutex);
                    clients.push_back(clientSocket);
                    std::cout << "[VisStream] Client connected to Visibility Stream." << std::endl;
                }
            }
        }
    });

    const double lambda = 0.001; // 1mm wavelength (ALMA band 9 style)

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 5Hz streaming rate

        std::lock_guard<std::mutex> lock(clientMutex);
        if (clients.empty()) continue;

        std::vector<VisibilityRecord> records;
        records.reserve(1225); // 50 * 49 / 2 baselines

        for (size_t i = 0; i < antennas->size(); ++i) {
            for (size_t j = i + 1; j < antennas->size(); ++j) {
                const auto& ant1 = (*antennas)[i];
                const auto& ant2 = (*antennas)[j];

                double x1 = ant1.getPosX();
                double y1 = ant1.getPosY();
                double x2 = ant2.getPosX();
                double y2 = ant2.getPosY();

                double u = (x1 - x2) / lambda;
                double v = (y1 - y2) / lambda;

                // Project Az/El to 2D unit vector
                double azRad = ant1.getAzimuth() * (3.141592653589793 / 180.0);
                double elRad = ant1.getElevation() * (3.141592653589793 / 180.0);
                double sx = cos(elRad) * sin(azRad);
                double sy = cos(elRad) * cos(azRad);

                double w = (x1 - x2) * sx + (y1 - y2) * sy;

                double phase1 = ant1.getSignalPhase();
                double phase2 = ant2.getSignalPhase();
                double amp1 = ant1.getSignalAmp();
                double amp2 = ant2.getSignalAmp();

                double realVis = amp1 * amp2 * cos(phase1 - phase2);
                double imagVis = amp1 * amp2 * sin(phase1 - phase2);

                VisibilityRecord rec;
                rec.ant1 = static_cast<uint32_t>(ant1.getId());
                rec.ant2 = static_cast<uint32_t>(ant2.getId());
                rec.u = u;
                rec.v = v;
                rec.w = w;
                rec.realVis = realVis;
                rec.imagVis = imagVis;

                records.push_back(rec);
            }
        }

        // Broadcast visibilities to all clients
        for (auto it = clients.begin(); it != clients.end(); ) {
            int sent = send(*it, reinterpret_cast<const char*>(records.data()), static_cast<int>(records.size() * sizeof(VisibilityRecord)), 0);
            if (sent == SOCKET_ERROR) {
                std::cout << "[VisStream] Client disconnected from Visibility Stream." << std::endl;
                closesocket(*it);
                it = clients.erase(it);
            } else {
                ++it;
            }
        }
    }

    if (acceptThread.joinable()) {
        acceptThread.join();
    }
    closesocket(serverSocket);
}

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

    // Start Visibility Streamer Thread
    std::atomic<bool> running(true);
    std::thread visThread(startVisibilityStreamer, &array, std::ref(running));

    std::cout << "System Ready. Listening on Port 9000 (Ctrl+C to stop)..." << std::endl;
    
    double globalTime = 0.0;
    const double windSpeed = 20.0; // 20 m/s wind propagation velocity
    const double lambda = 0.001; // 1mm wavelength
    const double waveK = 2.0 * 3.141592653589793 / lambda;

    while (true) {
        double dt = 0.1;
        globalTime += dt;

        // 1. Calculate base wind phase and direction
        double windPhase = globalTime * 0.5;
        double windAngle = windPhase * 0.1; // Rotating wind vector

        for (auto& ant : array) {
            // 2. Spatial Wind Propagation: delay based on antenna ground coordinates projected on wind vector
            double distance = ant.getPosX() * cos(windAngle) + ant.getPosY() * sin(windAngle);
            double delay = distance / windSpeed;
            double antTime = globalTime - delay;

            // Delayed local wind intensity calculation
            double antWindPhase = antTime * 0.5;
            double antWindIntensity = (sin(antWindPhase) * 0.2) + ((std::rand() % 100) / 500.0);

            double windForceAz = cos(windAngle) * antWindIntensity;
            double windForceEl = sin(windAngle) * antWindIntensity;

            ant.applyExternalForce(windForceAz, windForceEl);
            ant.update(dt);
            ant.updateSensors(dt);

            // 3. SCIENCE SIMULATION: Update Complex Signal
            double azRad = ant.getAzimuth() * (3.141592653589793 / 180.0);
            double elRad = ant.getElevation() * (3.141592653589793 / 180.0);
            double sx = cos(elRad) * sin(azRad);
            double sy = cos(elRad) * cos(azRad);

            // Geometric Phase Delay: phi = k * (B dot S)
            double phase = waveK * (ant.getPosX() * sx + ant.getPosY() * sy);
            
            // Set amplitude as constant (1.0) and phase modulated by geometry
            ant.setSignal(1.0, fmod(phase, 2.0 * 3.141592653589793));
        }

        static int tick = 0;
        if (tick++ % 10 == 0) { 
             std::cout << "[Jackstar] ANT-01: " << array[0].getStateString() 
                       << " Az:" << array[0].getAzimuth() 
                       << " | Kp:" << array[0].getKp()
                       << " | Phase:" << array[0].getSignalPhase() << "\r" << std::flush;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    running = false;
    if (visThread.joinable()) {
        visThread.join();
    }
    server.stop();
    return 0;
}
