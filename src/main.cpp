#include "game.h"
#include "lobby.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

void playLocalGame() {
    Game game;
    game.start();
}

void playNetworkGame() {
    auto lobby = std::make_shared<Lobby>();
    int choice;
    
    std::cout << "1. Host game\n";
    std::cout << "2. Join game\n";
    std::cout << "Choice: ";
    std::cin >> choice;
    
    if (choice == 1) {
        int port;
        std::cout << "Enter port to listen on: ";
        std::cin >> port;
        
        if (!lobby->hostGame(port)) {
            std::cout << "Failed to host game\n";
            return;
        }
        
        std::cout << "Waiting for connection... Your info: " << lobby->getLocalInfo() << "\n";
        
        while (lobby->getState() != Lobby::LobbyState::Connected) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (lobby->getState() == Lobby::LobbyState::ErrorState) {
                std::cout << "Error occurred while waiting for connection\n";
                return;
            }
        }
    } else if (choice == 2) {
        std::string ip;
        int port;
        
        std::cout << "Enter host IP: ";
        std::cin >> ip;
        std::cout << "Enter host port: ";
        std::cin >> port;
        
        if (!lobby->joinGame(ip, port)) {
            std::cout << "Failed to connect to host\n";
            return;
        }
        
        std::cout << "Connecting...\n";
        
        while (lobby->getState() == Lobby::LobbyState::Joining) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (lobby->getState() == Lobby::LobbyState::ErrorState) {
                std::cout << "Error occurred while connecting\n";
                return;
            }
        }
    } else {
        std::cout << "Invalid choice\n";
        return;
    }
    
    std::cout << "Connected to peer: " << lobby->getPeerInfo() << "\n";
    std::cout << "Starting game...\n";
    
    Game game(true, lobby);
    game.start();
}

int main() {
    int choice;
    
    std::cout << "1. Local game\n";
    std::cout << "2. Network game\n";
    std::cout << "Choice: ";
    std::cin >> choice;
    
    if (choice == 1) {
        playLocalGame();
    } else if (choice == 2) {
        playNetworkGame();
    } else {
        std::cout << "Invalid choice\n";
    }
    
    return 0;
}