#include "lobby.h"

Lobby::Lobby() : state(LobbyState::Idle) {
    network.setOnConnectedCallback([this]() {
        state = LobbyState::Connected;
    });
    
    network.setOnDisconnectedCallback([this]() {
        state = LobbyState::Idle;
    });
}

Lobby::~Lobby() {
    disconnect();
}

bool Lobby::hostGame(int port) {
    if (state != LobbyState::Idle) {
        return false;
    }
    
    if (network.startListening(port)) {
        state = LobbyState::Hosting;
        return true;
    }
    
    state = LobbyState::ErrorState;
    return false;
}

bool Lobby::joinGame(const std::string& ip, int port) {
    if (state != LobbyState::Idle) {
        return false;
    }
    
    if (network.connectToPeer(ip, port)) {
        state = LobbyState::Joining;
        return true;
    }
    
    state = LobbyState::ErrorState;
    return false;
}

void Lobby::disconnect() {
    network.disconnect();
    state = LobbyState::Idle;
}

void Lobby::sendMessage(const std::string& message) {
    network.sendMessage(message);
}

bool Lobby::hasMessages() {
    return network.hasMessages();
}

std::string Lobby::popMessage() {
    return network.popMessage();
}

Lobby::LobbyState Lobby::getState() const {
    return state;
}

std::string Lobby::getPeerInfo() const {
    return network.getPeerInfo();
}

std::string Lobby::getLocalInfo() const {
    return network.getLocalInfo();
}

void Lobby::setOnConnectedCallback(std::function<void()> callback) {
    network.setOnConnectedCallback(callback);
}

void Lobby::setOnDisconnectedCallback(std::function<void()> callback) {
    network.setOnDisconnectedCallback(callback);
}

void Lobby::setOnMessageReceivedCallback(std::function<void(const std::string&)> callback) {
    network.setOnMessageReceivedCallback(callback);
}