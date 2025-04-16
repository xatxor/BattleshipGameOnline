#include "game.h"
#include <iostream>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

void clearScreen() {
    #ifdef _WIN32
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {0, 0};
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    FillConsoleOutputCharacter(hStdOut, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
    SetConsoleCursorPosition(hStdOut, coord);
    #else
    system("clear");
    #endif
}

Game::Game() : player1(nullptr), player2(nullptr), player1Turn(true) {}

void Game::start() {
    std::cout << "Welcome to Battleship!\n\n";
    setupPlayers();
    gameLoop();
}

void Game::setupPlayers() {
    std::string name;
    std::cout << "Enter first player's name: ";
    std::cin >> name;
    player1 = new Player(name);
    
    std::cout << "Enter second player's name (or 'AI' to play against computer): ";
    std::cin >> name;
    player2 = new Player(name);
    
    // Ship placement for player 1
    std::cout << "\n" << player1->getName() << ", ship placement:\n";
    char choice;
    do {
        std::cout << "Auto-place ships? (y/n): ";
        std::cin >> choice;
    } while (choice != 'y' && choice != 'n');
    
    if (choice == 'y') {
        player1->autoPlaceShips();
    } else {
        manualPlacement(*player1);
    }
    
    // Ship placement for player 2
    if (player2->getName() == "AI") {
        player2->autoPlaceShips();
        std::cout << "\nComputer ships placed automatically.\n";
    } else {
        std::cout << "\n" << player2->getName() << ", ship placement:\n";
        do {
            std::cout << "Auto-place ships? (y/n): ";
            std::cin >> choice;
        } while (choice != 'y' && choice != 'n');
        
        if (choice == 'y') {
            player2->autoPlaceShips();
        } else {
            manualPlacement(*player2);
        }
    }
}

void Game::manualPlacement(Player& player) {
    std::vector<int> shipSizes = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
    
    for (int size : shipSizes) {
        bool placed = false;
        
        while (!placed) {
            clearScreen();
            player.printBoard(true);
            
            std::cout << "\nPlacing " << size << "-deck ship\n";
            
            char dirChar;
            Ship::Direction dir;
            do {
                std::cout << "Direction (h - horizontal, v - vertical): ";
                std::cin >> dirChar;
            } while (dirChar != 'h' && dirChar != 'v');
            dir = dirChar == 'h' ? Ship::HORIZONTAL : Ship::VERTICAL;
            
            int x, y;
            bool validInput = false;
            while (!validInput) {
                std::cout << "Coordinates (e.g. A1): ";
                char col;
                std::cin >> col >> y;
                
                col = toupper(col);
                x = col - 'A';
                y--; // Convert to 0-based index
                
                if (x >= 0 && x < 10 && y >= 0 && y < 10) {
                    validInput = true;
                } else {
                    std::cout << "Invalid coordinates. Try again.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            }
            
            Ship newShip(size, x, y, dir);
            if (player.isValidPlacement(newShip)) {
                player.addShip(newShip);
                placed = true;
            } else {
                std::cout << "Invalid ship placement. Try again.\n";
                std::cin.get();
            }
        }
    }
}

void Game::gameLoop() {
    while (true) {
        clearScreen();
        printBoards();
        
        Player& currentPlayer = player1Turn ? *player1 : *player2;
        Player& opponent = player1Turn ? *player2 : *player1;
        
        std::cout << "\n" << currentPlayer.getName() << "'s turn\n";
        
        bool hit = processAttack(currentPlayer, opponent);
        
        if (opponent.isDefeated()) {
            clearScreen();
            printBoards();
            std::cout << "\nPlayer " << currentPlayer.getName() << " wins!\n";
            break;
        }
        
        if (!hit) {
            player1Turn = !player1Turn;
            std::cout << "\nMiss! Turn passes to the other player.\n";
            std::cin.get();
            std::cin.get();
        } else {
            std::cout << "\nHit! " << currentPlayer.getName() << " gets another turn.\n";
            std::cin.get();
            std::cin.get();
        }
    }
}

void Game::printBoards() const {
    std::cout << "Your board:\n";
    if (player1Turn){
        player1->printBoard(true);
        std::cout << "\nEnemy board:\n";
        player2->printBoard(false);
    }
    else{
        player2->printBoard(true);
        std::cout << "\nEnemy board:\n";
        player1->printBoard(false);
    }
}

bool Game::processAttack(Player& attacker, Player& defender) {
    auto coords = getAttackCoordinates();
    int x = coords.first;
    int y = coords.second;
    
    bool hit = defender.attack(x, y);
    
    std::cout << "\n" << attacker.getName() << " attacks " 
              << static_cast<char>('A' + x) << y + 1 << " - "
              << (hit ? "HIT!" : "miss.") << "\n";
    
    // Проверяем, был ли потоплен корабль
    if (hit) {
        const Ship* ship = defender.getShipAtPoint(x, y);
        if (ship && ship->isSunk()) {
            std::cout << "You sunk a " << ship->getSize() << "-deck ship!\n";
        }
    }
    
    return hit;
}

std::pair<int, int> Game::getAttackCoordinates() const {
    int x, y;
    bool validInput = false;
    
    while (!validInput) {
        std::cout << "Enter attack coordinates (e.g. A1): ";
        char col;
        std::cin >> col >> y;
        
        col = toupper(col);
        x = col - 'A';
        y--; // Convert to 0-based index
        
        if (x >= 0 && x < 10 && y >= 0 && y < 10) {
            validInput = true;
        } else {
            std::cout << "Invalid coordinates. Try again.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    
    return {x, y};
}

Game::~Game() {
    delete player1;
    delete player2;
}