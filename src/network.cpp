#include "network.h"
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <ws2tcpip.h>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

NetworkManager::NetworkManager() : 
    listenSocket(INVALID_SOCKET), 
    clientSocket(INVALID_SOCKET),
    state(NetworkState::Disconnected),
    peerPort(0),
    localPort(0) {
    initializeNetwork();
}

NetworkManager::~NetworkManager() {
    disconnect();
    shutdownNetwork();
}

void NetworkManager::initializeNetwork() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        state = NetworkState::ErrorState;
        throw std::runtime_error("WSAStartup failed");
    }
#endif
}

void NetworkManager::shutdownNetwork() {
#ifdef _WIN32
    WSACleanup();
#endif
}

bool NetworkManager::startListening(int port) {
    if (state != NetworkState::Disconnected) {
        return false;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        state = NetworkState::ErrorState;
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
#ifdef _WIN32
        closesocket(listenSocket);
#else
        close(listenSocket);
#endif
        state = NetworkState::ErrorState;
        return false;
    }

    if (listen(listenSocket, 1) == SOCKET_ERROR) {
#ifdef _WIN32
        closesocket(listenSocket);
#else
        close(listenSocket);
#endif
        state = NetworkState::ErrorState;
        return false;
    }

    localPort = port;
    state = NetworkState::Listening;
    listenerThread = std::thread(&NetworkManager::listeningThread, this);
    return true;
}

bool NetworkManager::connectToPeer(const std::string& ip, int port) {
    if (state != NetworkState::Disconnected) {
        return false;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        state = NetworkState::ErrorState;
        return false;
    }

    sockaddr_in peerAddr;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &peerAddr.sin_addr);

    state = NetworkState::Connecting;
    if (connect(clientSocket, (sockaddr*)&peerAddr, sizeof(peerAddr)) == SOCKET_ERROR) {
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
        clientSocket = INVALID_SOCKET;
        state = NetworkState::ErrorState;
        return false;
    }

    peerIp = ip;
    peerPort = port;
    state = NetworkState::Connected;

    if (onConnected) {
        onConnected();
    }

    receiverThread = std::thread(&NetworkManager::receivingThread, this);
    return true;
}

void NetworkManager::disconnect() {
    if (state == NetworkState::Disconnected) {
        return;
    }

    state = NetworkState::Disconnected;

#ifdef _WIN32
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    if (clientSocket != INVALID_SOCKET) {
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
#else
    if (listenSocket != -1) {
        close(listenSocket);
        listenSocket = -1;
    }
    if (clientSocket != -1) {
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        clientSocket = -1;
    }
#endif

    if (listenerThread.joinable()) {
        listenerThread.join();
    }
    if (receiverThread.joinable()) {
        receiverThread.join();
    }

    if (onDisconnected) {
        onDisconnected();
    }

    cleanup();
}

void NetworkManager::cleanup() {
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!messageQueue.empty()) {
        messageQueue.pop();
    }
    peerIp.clear();
    peerPort = 0;
}

void NetworkManager::sendMessage(const std::string& message) {
    if (state != NetworkState::Connected || clientSocket == INVALID_SOCKET) {
        return;
    }

    std::string msg = message + "\n";
    send(clientSocket, msg.c_str(), msg.size(), 0);
}

bool NetworkManager::hasMessages() {
    std::lock_guard<std::mutex> lock(queueMutex);
    return !messageQueue.empty();
}

std::string NetworkManager::popMessage() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (messageQueue.empty()) {
        return "";
    }
    std::string msg = messageQueue.front();
    messageQueue.pop();
    return msg;
}

NetworkManager::NetworkState NetworkManager::getState() const {
    return state;
}

std::string NetworkManager::getPeerInfo() const {
    return peerIp + ":" + std::to_string(peerPort);
}

std::string NetworkManager::getLocalInfo() const {
    char host[256];
    gethostname(host, sizeof(host));
    return std::string(host) + ":" + std::to_string(localPort);
}

void NetworkManager::setOnConnectedCallback(std::function<void()> callback) {
    onConnected = callback;
}

void NetworkManager::setOnDisconnectedCallback(std::function<void()> callback) {
    onDisconnected = callback;
}

void NetworkManager::setOnMessageReceivedCallback(std::function<void(const std::string&)> callback) {
    onMessageReceived = callback;
}

void NetworkManager::listeningThread() {
    sockaddr_in clientAddr;
#ifdef _WIN32
    int addrLen = sizeof(clientAddr);
#else
    socklen_t addrLen = sizeof(clientAddr);
#endif

    clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
    if (clientSocket == INVALID_SOCKET) {
        state = NetworkState::ErrorState;
        return;
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
    peerIp = ipStr;
    peerPort = ntohs(clientAddr.sin_port);

    state = NetworkState::Connected;

    if (onConnected) {
        onConnected();
    }

#ifdef _WIN32
    closesocket(listenSocket);
#else
    close(listenSocket);
#endif
    listenSocket = INVALID_SOCKET;

    receivingThread();
}

void NetworkManager::receivingThread() {
    char buffer[1024];
    std::string partialMessage;

    while (state == NetworkState::Connected) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            disconnect();
            return;
        }

        buffer[bytesReceived] = '\0';
        partialMessage += buffer;

        size_t pos;
        while ((pos = partialMessage.find('\n')) != std::string::npos) {
            std::string message = partialMessage.substr(0, pos);
            partialMessage.erase(0, pos + 1);

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                messageQueue.push(message);
            }

            if (onMessageReceived) {
                onMessageReceived(message);
            }
        }
    }
}