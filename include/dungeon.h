#pragma once
#include "raylib.h"
#include <vector>

class Dungeon {
public:
    Dungeon();
    
    void Generate();
    void Draw();
    bool IsWall(int x, int y);
    Rectangle GetWallRect(int x, int y);
    
    static const int MAP_WIDTH = 25;
    static const int MAP_HEIGHT = 20;
    static const int TILE_SIZE = 32;
    
private:
    std::vector<std::vector<int>> map;
    Color wallColor;
    Color floorColor;
};