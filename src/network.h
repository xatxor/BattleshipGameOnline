#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <atomic>
#include <memory>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

class NetworkManager {
public:
    enum class NetworkState {
        Disconnected,
        Connecting,
        Connected,
        Listening,
        ErrorState
    };

    NetworkManager();
    ~NetworkManager();

    bool startListening(int port);
    bool connectToPeer(const std::string& ip, int port);
    void disconnect();
    void sendMessage(const std::string& message);
    bool hasMessages();
    std::string popMessage();
    NetworkState getState() const;
    std::string getPeerInfo() const;
    std::string getLocalInfo() const;

    void setOnConnectedCallback(std::function<void()> callback);
    void setOnDisconnectedCallback(std::function<void()> callback);
    void setOnMessageReceivedCallback(std::function<void(const std::string&)> callback);

private:
    void listeningThread();
    void receivingThread();
    void processConnection();

#ifdef _WIN32
    SOCKET listenSocket;
    SOCKET clientSocket;
#else
    int listenSocket;
    int clientSocket;
#endif

    std::atomic<NetworkState> state;
    std::thread listenerThread;
    std::thread receiverThread;
    std::mutex queueMutex;
    std::queue<std::string> messageQueue;
    std::string peerIp;
    int peerPort;
    int localPort;

    std::function<void()> onConnected;
    std::function<void()> onDisconnected;
    std::function<void(const std::string&)> onMessageReceived;

    void cleanup();
    void initializeNetwork();
    void shutdownNetwork();
};

#endif // NETWORK_H