#ifndef SHIP_H
#define SHIP_H

#include <vector>
#include <utility>

class Ship {
public:
    enum Direction { HORIZONTAL, VERTICAL };

    Ship(int size, int x, int y, Direction dir);
    
    int getSize() const;
    std::pair<int, int> getPosition() const;
    Direction getDirection() const;
    bool isSunk() const;
    void hit();
    bool contains(int x, int y) const;
    const std::vector<bool>& getHits() const;

private:
    int size;
    int posX, posY;
    Direction direction;
    std::vector<bool> hits;
};

#endif