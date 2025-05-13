#ifndef GAME_H
#define GAME_H

#include "player.h"
#include "lobby.h"
#include <memory>

class Game {
public:
    Game(bool isNetworkGame = false, std::shared_ptr<Lobby> lobby = nullptr);
    ~Game();  
    void start();
    
private:
    Player* player1;  
    Player* player2;
    bool player1Turn;
    bool isNetworkGame;
    std::shared_ptr<Lobby> lobby;
    
    void setupPlayers();
    void manualPlacement(Player& player);
    void gameLoop();
    void networkGameLoop();
    void printBoards() const;
    bool processAttack(Player& attacker, Player& defender);
    bool processNetworkAttack(Player& attacker, Player& defender);
    std::pair<int, int> getAttackCoordinates() const;
    void sendAttackToOpponent(int x, int y);
    void waitForOpponentAttack();
    
};

#endif