#ifndef GAME_H
#define GAME_H

#include "player.h"

class Game {
public:
    Game();
    ~Game();  
    void start();
    
private:
    Player* player1;  
    Player* player2;
    bool player1Turn;
    
    void setupPlayers();
    void manualPlacement(Player& player);
    void gameLoop();
    void printBoards() const;
    bool processAttack(Player& attacker, Player& defender);
    std::pair<int, int> getAttackCoordinates() const;
};

#endif