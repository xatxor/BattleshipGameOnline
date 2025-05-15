#include "game.h"
#include <iostream>
#include <thread>
#include <chrono>

void playLocalGame() {
    Game game;
    game.start();
}

int main() {

    playLocalGame();

    return 0;
}