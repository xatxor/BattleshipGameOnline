#ifndef PLAYER_H
#define PLAYER_H

#include "ship.h"
#include <vector>
#include <string>

class Player {
public:
    Player(const std::string& name);
    
    const std::string& getName() const;
    void addShip(const Ship& ship);
    bool isDefeated() const;
    bool attack(int x, int y);
    void printBoard(bool showShips) const;
    bool isValidPlacement(const Ship& newShip) const;
    void autoPlaceShips();
    const Ship* getShipAtPoint(int x, int y) const;

private:
    std::string name;
    std::vector<Ship> ships;
    std::vector<std::pair<int, int>> attacks;
    
    bool isCellOccupied(int x, int y) const;
};

#endif