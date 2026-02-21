#include "AntennaServer.h"
#include "../../include/Protocol.h"
#include <iostream>
#include <ws2tcpip.h>

AntennaServer::AntennaServer(std::vector<Antenna>* antennaArray) 
    : antennas(antennaArray), serverSocket(INVALID_SOCKET), running(false) {
    
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
    }
}

AntennaServer::~AntennaServer() {
    stop();
    WSACleanup();
}

bool AntennaServer::start(int port) {
    struct addrinfo *result = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    std::string portStr = std::to_string(port);
    if (getaddrinfo(NULL, portStr.c_str(), &hints, &result) != 0) {
        std::cerr << "getaddrinfo failed" << std::endl;
        return false;
    }

    // Create a SOCKET for the server to listen for client connections
    serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        return false;
    }

    // Setup the TCP listening socket
    if (bind(serverSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(serverSocket);
        return false;
    }

    freeaddrinfo(result);

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        return false;
    }

    running = true;
    serverThread = std::thread(&AntennaServer::acceptConnections, this);
    
    std::cout << "[Server] Listening on port " << port << std::endl;
    return true;
}

void AntennaServer::stop() {
    running = false;
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

void AntennaServer::acceptConnections() {
    while (running) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            if (running) std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::cout << "[Server] Client connected!" << std::endl;
        std::thread(&AntennaServer::handleClient, this, clientSocket).detach();
    }
}

void AntennaServer::handleClient(SOCKET clientSocket) {
    Packet packet;
    int iResult;

    // Receive until the peer shuts down the connection
    do {
        iResult = recv(clientSocket, (char*)&packet, sizeof(Packet), 0);
        if (iResult > 0) {
            // Process Packet
            if (packet.type == PacketType::CMD_MOVE) {
                // Warning: No mutex here for simplicity (Race condition potential)
                // In production, protect shared 'antennas' vector access
                
                if (packet.antennaId == 0) {
                    // Broadcast to all
                     std::cout << "[Server] Broadcast CMD: Az=" << packet.azimuth << " El=" << packet.elevation << std::endl;
                    for (auto& ant : *antennas) {
                        ant.setTarget(packet.azimuth, packet.elevation);
                    }
                } else if (packet.antennaId <= antennas->size()) {
                    // Specific antenna (1-based ID)
                    std::cout << "[Server] CMD for ANT-" << packet.antennaId << std::endl;
                    (*antennas)[packet.antennaId - 1].setTarget(packet.azimuth, packet.elevation);
                }
            } else if (packet.type == PacketType::CMD_GET_TELEMETRY) {
                // Send status of ALL antennas
                // Ideally this should be a single bulk packet, but for simplicity we send N packets
                for (const auto& ant : *antennas) {
                     Packet response;
                     response.type = PacketType::TELEMETRY;
                     response.antennaId = ant.getId();
                     response.azimuth = ant.getAzimuth();
                     response.elevation = ant.getElevation();
                     response.state = (uint8_t)ant.getState();
                     
                     send(clientSocket, (const char*)&response, sizeof(Packet), 0);
                } 
            } else if (packet.type == PacketType::CMD_RESET) {
                 if (packet.antennaId == 0) {
                     for (auto& ant : *antennas) ant.reset();
                     std::cout << "[Server] Global RESET broadcasted." << std::endl;
                 } else if (packet.antennaId <= antennas->size()) {
                     (*antennas)[packet.antennaId - 1].reset();
                     std::cout << "[Server] RESET for ANT-" << packet.antennaId << std::endl;
                 }
            }
        } else if (iResult == 0) {
            std::cout << "Connection closing..." << std::endl;
        } else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            return;
        }

    } while (iResult > 0);

    closesocket(clientSocket);
}
