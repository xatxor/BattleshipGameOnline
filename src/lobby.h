#ifndef LOBBY_H
#define LOBBY_H

#include "network.h"
#include <string>
#include <functional>

class Lobby {
public:
    enum class LobbyState {
        Idle,
        Hosting,
        Joining,
        Connected,
        ErrorState
    };

    Lobby();
    ~Lobby();

    bool hostGame(int port);
    bool joinGame(const std::string& ip, int port);
    void disconnect();
    void sendMessage(const std::string& message);
    bool hasMessages();
    std::string popMessage();

    LobbyState getState() const;
    std::string getPeerInfo() const;
    std::string getLocalInfo() const;

    void setOnConnectedCallback(std::function<void()> callback);
    void setOnDisconnectedCallback(std::function<void()> callback);
    void setOnMessageReceivedCallback(std::function<void(const std::string&)> callback);

private:
    NetworkManager network;
    LobbyState state;
};

#endif // LOBBY_H