#include "ship.h"

Ship::Ship(int size, int x, int y, Direction dir) 
    : size(size), posX(x), posY(y), direction(dir), hits(size, false) {}

int Ship::getSize() const {
    return size;
}

std::pair<int, int> Ship::getPosition() const {
    return {posX, posY};
}

Ship::Direction Ship::getDirection() const {
    return direction;
}

bool Ship::isSunk() const {
    for (bool h : hits) {
        if (!h) return false;
    }
    return true;
}

void Ship::hit() {
    for (std::size_t i = 0; i < hits.size(); ++i) {
        if (!hits[i]) {
            hits[i] = true;
            break;
        }
    }
}

bool Ship::contains(int x, int y) const {
    if (direction == HORIZONTAL) {
        return y == posY && x >= posX && x < posX + size;
    } else {
        return x == posX && y >= posY && y < posY + size;
    }
}

const std::vector<bool>& Ship::getHits() const {
    return hits;
}