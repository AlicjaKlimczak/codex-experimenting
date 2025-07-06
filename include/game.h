#pragma once
#include "raylib.h"
#include "player.h"
#include "dungeon.h"
#include "enemy.h"
#include <vector>

class Game {
public:
    Game();
    ~Game();
    
    void Run();
    
private:
    void Update();
    void Draw();
    
    static const int SCREEN_WIDTH = 800;
    static const int SCREEN_HEIGHT = 600;
    
    Player player;
    Dungeon dungeon;
    std::vector<Enemy> enemies;
    Camera2D camera;
};