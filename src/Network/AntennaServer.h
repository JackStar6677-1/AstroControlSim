#ifndef ANTENNA_SERVER_H
#define ANTENNA_SERVER_H

#include <winsock2.h>
#include <vector>
#include <thread>
#include <atomic>
#include "../Antenna/Antenna.h"

// Link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

class AntennaServer {
private:
    SOCKET serverSocket;
    std::vector<Antenna>* antennas; // Reference to the main antenna array
    std::atomic<bool> running;
    std::thread serverThread;

    void acceptConnections();
    void handleClient(SOCKET clientSocket);

public:
    AntennaServer(std::vector<Antenna>* antennaArray);
    ~AntennaServer();

    bool start(int port);
    void stop();
};

#endif
