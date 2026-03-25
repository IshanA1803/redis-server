#include "../include/RedisCommandHandler.h"

RedisCommandHandler::RedisCommandHandler() {}

std::string RedisCommandHandler::processCommand(const std::string& commandLine) {
    // For now: ignore input and return static response
    return "+OK\r\n";
}