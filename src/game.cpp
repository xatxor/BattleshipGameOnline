#include "game.h"
#include <iostream>
#include <limits>
#include <sstream>

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

Game::Game(bool isNetworkGame, std::shared_ptr<Lobby> lobby) 
    : player1(nullptr), player2(nullptr), player1Turn(true), 
      isNetworkGame(isNetworkGame), lobby(lobby) {}

void Game::start() {
    std::cout << "Welcome to Battleship!\n\n";
    setupPlayers();
    
    if (isNetworkGame) {
        networkGameLoop();
    } else {
        gameLoop();
    }
}

void Game::setupPlayers() {
    std::string name;
    std::cout << "Enter first player's name: ";
    std::cin >> name;
    player1 = new Player(name);
    
    if (isNetworkGame) {
        player2 = new Player("Opponent");
        std::cout << "\n" << player1->getName() << ", ship placement:\n";
        char choice;
        do {
            std::cout << "Auto-place ships? (y/n): ";
            std::cin >> choice;
        } while (choice != 'y' && choice != 'n');
        
        if (choice == 'y') {
            player1->autoPlaceShips();
            sendShipPlacement(*player1);
        } else {
            manualPlacement(*player1);
            sendShipPlacement(*player1);
        }
        
        // Wait for opponent's ships
        std::cout << "\nWaiting for opponent to place ships...\n";
        receiveShipPlacement(*player2);
    } else {
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
}

void Game::sendShipPlacement(const Player& player) {
    // Send ship placement to opponent
    std::ostringstream oss;
    for (const auto& ship : player.getShips()) {
        auto pos = ship.getPosition();
        oss << "SHIP " << ship.getSize() << " " 
            << pos.first << " " << pos.second << " "
            << (ship.getDirection() == Ship::HORIZONTAL ? "H" : "V") << ";";
    }
    lobby->sendMessage(oss.str());
}

void Game::receiveShipPlacement(Player& player) {
    while (true) {
        if (lobby->hasMessages()) {
            std::string msg = lobby->popMessage();
            if (msg.find("SHIP") == 0) {
                std::istringstream iss(msg);
                std::string cmd;
                while (iss >> cmd) {
                    if (cmd == "SHIP") {
                        int size, x, y;
                        std::string dirStr;
                        iss >> size >> x >> y >> dirStr;
                        Ship::Direction dir = (dirStr == "H") ? Ship::HORIZONTAL : Ship::VERTICAL;
                        Ship ship(size, x, y, dir);
                        player.addShip(ship);
                    }
                }
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

void Game::networkGameLoop() {
    // Determine who goes first (host goes first)
    player1Turn = (lobby->getState() == Lobby::LobbyState::Hosting);
    
    while (true) {
        clearScreen();
        printBoards();
        
        if (player1Turn) {
            // Our turn to attack
            Player& currentPlayer = *player1;
            Player& opponent = *player2;
            
            std::cout << "\nYour turn to attack!\n";
            
            bool hit = processNetworkAttack(currentPlayer, opponent);
            
            if (opponent.isDefeated()) {
                clearScreen();
                printBoards();
                std::cout << "\nYou win!\n";
                lobby->sendMessage("WIN");
                break;
            }
            
            if (!hit) {
                player1Turn = !player1Turn;
                lobby->sendMessage("MISS");
                std::cout << "\nMiss! Waiting for opponent's turn...\n";
                waitForOpponentAttack();
            } else {
                lobby->sendMessage("HIT");
                std::cout << "\nHit! You get another turn.\n";
                std::cin.get();
                std::cin.get();
            }
        } else {
            // Opponent's turn to attack
            std::cout << "\nWaiting for opponent's attack...\n";
            waitForOpponentAttack();
        }
    }
}

void Game::printBoards() const {
    std::cout << "Your board:\n";
    player1->printBoard(true);
    std::cout << "\nEnemy board:\n";
    player2->printBoard(false);
}

bool Game::processAttack(Player& attacker, Player& defender) {
    auto coords = getAttackCoordinates();
    int x = coords.first;
    int y = coords.second;
    
    bool hit = defender.attack(x, y);
    
    std::cout << "\n" << attacker.getName() << " attacks " 
              << static_cast<char>('A' + x) << y + 1 << " - "
              << (hit ? "HIT!" : "miss.") << "\n";
    
    if (hit) {
        const Ship* ship = defender.getShipAtPoint(x, y);
        if (ship && ship->isSunk()) {
            std::cout << "You sunk a " << ship->getSize() << "-deck ship!\n";
        }
    }
    
    return hit;
}

bool Game::processNetworkAttack(Player& attacker, Player& defender) {
    auto coords = getAttackCoordinates();
    int x = coords.first;
    int y = coords.second;
    
    sendAttackToOpponent(x, y);
    bool hit = defender.attack(x, y);
    
    std::cout << "\nYou attack " 
              << static_cast<char>('A' + x) << y + 1 << " - "
              << (hit ? "HIT!" : "miss.") << "\n";
    
    if (hit) {
        const Ship* ship = defender.getShipAtPoint(x, y);
        if (ship && ship->isSunk()) {
            std::cout << "You sunk a " << ship->getSize() << "-deck ship!\n";
        }
    }
    
    return hit;
}

void Game::sendAttackToOpponent(int x, int y) {
    std::ostringstream oss;
    oss << "ATTACK " << x << " " << y;
    lobby->sendMessage(oss.str());
}

void Game::waitForOpponentAttack() {
    while (true) {
        if (lobby->hasMessages()) {
            std::string msg = lobby->popMessage();
            
            if (msg.find("ATTACK") == 0) {
                // Parse attack coordinates
                std::istringstream iss(msg);
                std::string cmd;
                int x, y;
                iss >> cmd >> x >> y;
                
                Player& currentPlayer = *player1;
                Player& opponent = *player2;
                
                bool hit = currentPlayer.attack(x, y);
                
                clearScreen();
                printBoards();
                std::cout << "\nOpponent attacks " 
                          << static_cast<char>('A' + x) << y + 1 << " - "
                          << (hit ? "HIT!" : "miss.") << "\n";
                
                if (hit) {
                    const Ship* ship = currentPlayer.getShipAtPoint(x, y);
                    if (ship && ship->isSunk()) {
                        std::cout << "Your " << ship->getSize() << "-deck ship was sunk!\n";
                    }
                }
                
                if (currentPlayer.isDefeated()) {
                    std::cout << "\nYou lost! Opponent wins.\n";
                    exit(0);
                }
                
                if (msg.find("MISS") == 0) {
                    player1Turn = !player1Turn;
                }
                
                std::cin.get();
                std::cin.get();
                return;
            }
            else if (msg == "WIN") {
                clearScreen();
                printBoards();
                std::cout << "\nYou lost! Opponent wins.\n";
                exit(0);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
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