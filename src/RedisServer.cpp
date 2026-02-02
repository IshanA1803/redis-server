#include "../include/RedisServer.h"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

RedisServer::RedisServer(int port)
    : port(port), server_socket(-1), running(true) {}

void RedisServer::run() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating server socket\n";
        return;
    }

    int opt = 1;
    // Allow reuse of the address immediately after the program terminates
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding server socket\n";
        close(server_socket);
        return;
    }
    
    if (listen(server_socket, 10) < 0) {
        std::cerr << "Error listening on server socket\n";
        close(server_socket);
        return;
    }

    std::cout << "Server listening on port " << port << "\n";

    while (running) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            std::cerr << "Error accepting client connection\n";
            break;
        }

        // Immediately close client
        close(client_socket);
    }

    close(server_socket);
}
