#include "../include/RedisServer.h"
#include "../include/RedisCommandHandler.h"
#include "../include/RedisDatabase.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <signal.h>
#include <thread>

// Global pointer for signal handling
static RedisServer* globalServer = nullptr;

void signalHandler(int signum) {
    if (globalServer) {
        std::cout << "\nCaught signal " << signum
                  << ", shutting down...\n";
        globalServer->shutdown();
    }

    exit(signum);
}

void RedisServer::setupSignalHandler() {
    signal(SIGINT, signalHandler);
}

RedisServer::RedisServer(int port)
    : port(port), server_socket(-1), running(true) {
        globalServer = this;
        setupSignalHandler();
    }

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

    std::vector<std::thread> threads;
    RedisCommandHandler cmdHandler;

    while (running) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            std::cerr << "Error accepting client connection\n";
            break;
        }
        threads.emplace_back([client_socket, &cmdHandler]() {
            char buffer[1024];

            while (true) {
                memset(buffer, 0, sizeof(buffer));
            
                int bytes = recv(client_socket,
                                 buffer,
                                 sizeof(buffer) - 1,
                                 0);
                
                if (bytes <= 0)
                    break;
                
                std::string request(buffer, bytes);
                
                std::string response =
                    cmdHandler.processCommand(request);
                
                send(client_socket,response.c_str(),response.size(),0);
            }
        
            close(client_socket);
        });
    }

    for (auto& t : threads) {
        if (t.joinable())t.join();
    }

    close(server_socket);
    
    if (RedisDatabase::getInstance().dump("dump.my_rdb"))
        std::cout << "Database Dumped to dump.my_rdb\n";
    else 
        std::cerr << "Error dumping database\n";
}

void RedisServer::shutdown() {
    running = false;

    if (server_socket != -1) {

        close(server_socket);

        if (RedisDatabase::getInstance().dump("dump.my_rdb"))
            std::cout << "Database Dumped to dump.my_rdb\n";
        else
            std::cerr << "Error dumping database\n";
    }

    std::cout << "Server Shutdown Complete!\n";
}