#include "dungeon.h"
#include <cstdlib>
#include <ctime>

Dungeon::Dungeon() : wallColor({80, 60, 40, 255}), floorColor({120, 100, 80, 255}) {
    map.resize(MAP_HEIGHT, std::vector<int>(MAP_WIDTH, 0));
    srand(time(nullptr));
}

void Dungeon::Generate() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1) {
                map[y][x] = 1;
            } else if (rand() % 100 < 15) {
                map[y][x] = 1;
            } else {
                map[y][x] = 0;
            }
        }
    }
    
    map[3][3] = 0;
    map[3][4] = 0;
    map[4][3] = 0;
    map[4][4] = 0;
}

void Dungeon::Draw() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            Rectangle tileRect = {
                x * TILE_SIZE,
                y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };
            
            if (map[y][x] == 1) {
                DrawRectangleRec(tileRect, wallColor);
                DrawRectangleLinesEx(tileRect, 1, BLACK);
            } else {
                DrawRectangleRec(tileRect, floorColor);
            }
        }
    }
}

bool Dungeon::IsWall(int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return true;
    }
    return map[y][x] == 1;
}

Rectangle Dungeon::GetWallRect(int x, int y) {
    return {
        x * TILE_SIZE,
        y * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    };
}