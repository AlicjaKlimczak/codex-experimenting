#pragma once
#include "raylib.h"

class Player {
public:
    Player();
    Player(Vector2 startPos);
    
    void Update();
    void Draw();
    bool CheckCollision(Rectangle rect);
    
    Vector2 GetPosition() const { return position; }
    Rectangle GetBounds() const;
    
private:
    Vector2 position;
    Vector2 velocity;
    float speed;
    Color color;
    static const float SIZE;
};