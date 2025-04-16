#include "player.h"
#include <iostream>
#include <random>
#include <algorithm>

Player::Player(const std::string& name) : name(name) {}

const std::string& Player::getName() const {
    return name;
}

void Player::addShip(const Ship& ship) {
    ships.push_back(ship);
}

bool Player::isDefeated() const {
    for (const auto& ship : ships) {
        if (!ship.isSunk()) {
            return false;
        }
    }
    return true;
}

bool Player::attack(int x, int y) {
    attacks.emplace_back(x, y);
    for (auto& ship : ships) {
        if (ship.contains(x, y)) {
            ship.hit();
            return true;
        }
    }
    return false;
}

void Player::printBoard(bool showShips) const {
    std::cout << "   A B C D E F G H I J\n";
    for (int y = 0; y < 10; ++y) {
        std::cout << (y < 9 ? " " : "") << y + 1 << " ";
        for (int x = 0; x < 10; ++x) {
            bool attacked = std::find(attacks.begin(), attacks.end(), std::make_pair(x, y)) != attacks.end();
            bool hasShip = false;
            bool shipHit = false;
            
            for (const auto& ship : ships) {
                if (ship.contains(x, y)) {
                    hasShip = true;
                    if (attacked) {
                        for (size_t i = 0; i < ship.getHits().size(); ++i) {
                            if (ship.getHits()[i]) {
                                auto pos = ship.getPosition();
                                int sx = pos.first, sy = pos.second;
                                if (ship.getDirection() == Ship::HORIZONTAL) {
                                    if (x == sx + i && y == sy) {
                                        shipHit = true;
                                        break;
                                    }
                                } else {
                                    if (x == sx && y == sy + i) {
                                        shipHit = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
            }
            
            if (attacked) {
                if (hasShip) {
                    std::cout << (shipHit ? "X " : "x ");
                } else {
                    std::cout << "o ";
                }
            } else if (showShips && hasShip) {
                std::cout << "# ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
}

bool Player::isValidPlacement(const Ship& newShip) const {
    auto pos = newShip.getPosition();
    int x = pos.first, y = pos.second;
    int size = newShip.getSize();
    Ship::Direction dir = newShip.getDirection();
    
    // Check boundaries
    if (dir == Ship::HORIZONTAL) {
        if (x < 0 || x + size > 10 || y < 0 || y >= 10) return false;
    } else {
        if (x < 0 || x >= 10 || y < 0 || y + size > 10) return false;
    }
    
    // Check overlap with existing ships
    for (int i = 0; i < size; ++i) {
        int cx = (dir == Ship::HORIZONTAL) ? x + i : x;
        int cy = (dir == Ship::HORIZONTAL) ? y : y + i;
        
        // Check current cell and surrounding cells
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                int nx = cx + dx;
                int ny = cy + dy;
                if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10) {
                    if (isCellOccupied(nx, ny)) {
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

bool Player::isCellOccupied(int x, int y) const {
    for (const auto& ship : ships) {
        if (ship.contains(x, y)) {
            return true;
        }
    }
    return false;
}

const Ship* Player::getShipAtPoint(int x, int y) const {
    for (const auto& ship : ships) {
        if (ship.contains(x, y)) {
            return &ship;
        }
    }
    return nullptr;
}

void Player::autoPlaceShips() {
    std::vector<int> shipSizes = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int size : shipSizes) {
        bool placed = false;
        int attempts = 0;
        
        while (!placed && attempts < 100) {
            attempts++;
            
            std::uniform_int_distribution<> dirDist(0, 1);
            Ship::Direction dir = dirDist(gen) == 0 ? Ship::HORIZONTAL : Ship::VERTICAL;
            
            int maxX = dir == Ship::HORIZONTAL ? 10 - size : 10;
            int maxY = dir == Ship::HORIZONTAL ? 10 : 10 - size;
            
            if (maxX <= 0 || maxY <= 0) continue;
            
            std::uniform_int_distribution<> xDist(0, maxX - 1);
            std::uniform_int_distribution<> yDist(0, maxY - 1);
            
            int x = xDist(gen);
            int y = yDist(gen);
            
            Ship newShip(size, x, y, dir);
            if (isValidPlacement(newShip)) {
                addShip(newShip);
                placed = true;
            }
        }
        
        if (!placed) {
            // If auto placement fails, clear and try again
            ships.clear();
            autoPlaceShips();
            return;
        }
    }
}