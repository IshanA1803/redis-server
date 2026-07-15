#include "../include/RedisServer.h"
#include "../include/RedisDatabase.h"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    int port = 6379;

    if (argc >= 2)port = std::stoi(argv[1]);

    if (RedisDatabase::getInstance().load("dump.my_rdb"))
        std::cout << "Database Loaded From dump.my_rdb\n";
    else
        std::cout << "No dump found or load failed; Starting with an empty database.\n";

    RedisServer server(port);

    std::thread persistenceThread([]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(300));

            if (!RedisDatabase::getInstance().dump("dump.my_rdb"))
                std::cerr << "Error Dumping Database\n";
            else
                std::cout << "Database Dumped to dump.my_rdb\n";
        }
    });

    persistenceThread.detach();

    server.run();

    return 0;
}