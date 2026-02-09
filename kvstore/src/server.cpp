#include "kvstore.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <cstring>

using namespace kvstore;

void HandleClient(int client_socket, KVStore& store) {
    char buffer[4096];
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_read <= 0) {
            break;
        }
        
        std::string request(buffer);
        std::string response;
        
        // Parse command: PUT key value, GET key, DELETE key
        if (request.substr(0, 3) == "PUT") {
            size_t pos1 = request.find(' ', 4);
            std::string key = request.substr(4, pos1 - 4);
            std::string value = request.substr(pos1 + 1);
            
            if (store.Put(key, value)) {
                response = "OK\n";
            } else {
                response = "ERROR\n";
            }
        }
        else if (request.substr(0, 3) == "GET") {
            std::string key = request.substr(4);
            key.erase(key.find_last_not_of(" \n\r\t") + 1);
            
            std::string value;
            if (store.Get(key, value)) {
                response = value + "\n";
            } else {
                response = "NOT_FOUND\n";
            }
        }
        else if (request.substr(0, 6) == "DELETE") {
            std::string key = request.substr(7);
            key.erase(key.find_last_not_of(" \n\r\t") + 1);
            
            if (store.Delete(key)) {
                response = "OK\n";
            } else {
                response = "ERROR\n";
            }
        }
        else {
            response = "UNKNOWN_COMMAND\n";
        }
        
        send(client_socket, response.c_str(), response.size(), 0);
    }
    
    close(client_socket);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    Config config;
    config.data_dir = "./data";
    config.memtable_size_mb = 64;
    config.compaction_threshold = 4;
    
    KVStore store(config);
    std::cout << "KVStore server starting on port " << port << std::endl;
    
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        return 1;
    }
    
    if (listen(server_socket, 10) < 0) {
        std::cerr << "Failed to listen" << std::endl;
        return 1;
    }
    
    std::cout << "Server listening..." << std::endl;
    
    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            continue;
        }
        
        std::thread(HandleClient, client_socket, std::ref(store)).detach();
    }
    
    close(server_socket);
    return 0;
}
