#include "../include/RedisCommandHandler.h"

#include <vector>
#include <sstream>
#include <algorithm>

// RESP Parser
std::vector<std::string> parseRespCommand(const std::string &input) {
    std::vector<std::string> tokens;
    if (input.empty()) return tokens;

    // Inline command (fallback)
    if (input[0] != '*') {
        std::istringstream iss(input);
        std::string token;
        while (iss >> token) //used if the command is not in RESP format, we can parse it as a simple space-separated command
            tokens.push_back(token);
        return tokens;
    }

    size_t pos = 1;
    size_t crlf = input.find("\r\n", pos);
    if (crlf == std::string::npos) return tokens;

    int numElements = std::stoi(input.substr(pos, crlf - pos));
    pos = crlf + 2;

    for (int i = 0; i < numElements; i++) {
        if (input[pos] != '$') break;
        pos++;

        crlf = input.find("\r\n", pos);
        int len = std::stoi(input.substr(pos, crlf - pos));
        pos = crlf + 2;

        std::string token = input.substr(pos, len);
        tokens.push_back(token);
        pos += len + 2;
    }

    return tokens;
}

static std::string handlePing(const std::vector<std::string>&) {
    return "+PONG\r\n";
}

static std::string handleEcho(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2)
        return "-Error: ECHO requires a message\r\n";
    return "+" + tokens[1] + "\r\n";
}

RedisCommandHandler::RedisCommandHandler() {}

std::string RedisCommandHandler::processCommand(const std::string& commandLine) {
    auto tokens = parseRespCommand(commandLine);
    if (tokens.empty())
        return "-Error: Empty command\r\n";

    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    if (cmd == "PING")
        return handlePing(tokens);
    else if (cmd == "ECHO")
        return handleEcho(tokens);
    else
        return "-Error: Unknown command\r\n";
}