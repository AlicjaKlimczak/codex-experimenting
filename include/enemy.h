#pragma once
#include "raylib.h"
#include <vector>

class Enemy {
public:
    Enemy();
    Enemy(Vector2 startPos);
    
    void Update(Vector2 playerPos, const std::vector<std::vector<int>>& map);
    bool CheckCollision(Rectangle rect);
    void Draw();
    bool IsAlive() const { return alive; }
    Rectangle GetBounds() const;
    
private:
    Vector2 position;
    Vector2 velocity;
    float speed;
    Color color;
    bool alive;
    float moveTimer;
    static const float SIZE;
};