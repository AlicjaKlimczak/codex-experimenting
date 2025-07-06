#include "enemy.h"
#include <cmath>
#include <cstdlib>

const float Enemy::SIZE = 14.0f;

Enemy::Enemy() : position({0, 0}), velocity({0, 0}), speed(50.0f), color(RED), alive(true), moveTimer(0.0f) {}

Enemy::Enemy(Vector2 startPos) : position(startPos), velocity({0, 0}), speed(50.0f), color(RED), alive(true), moveTimer(0.0f) {}

void Enemy::Update(Vector2 playerPos, const std::vector<std::vector<int>>& map) {
    moveTimer += GetFrameTime();
    
    if (moveTimer > 1.0f) {
        float dx = playerPos.x - position.x;
        float dy = playerPos.y - position.y;
        float distance = sqrt(dx * dx + dy * dy);
        
        if (distance < 200.0f && distance > 0) {
            velocity.x = (dx / distance) * speed;
            velocity.y = (dy / distance) * speed;
        } else {
            velocity.x = (rand() % 3 - 1) * speed * 0.5f;
            velocity.y = (rand() % 3 - 1) * speed * 0.5f;
        }
        
        moveTimer = 0.0f;
    }
    
    Vector2 newPos = {
        position.x + velocity.x * GetFrameTime(),
        position.y + velocity.y * GetFrameTime()
    };
    
    position = newPos;
    
    if (position.x < 0) position.x = 0;
    if (position.y < 0) position.y = 0;
    if (position.x > 800 - SIZE) position.x = 800 - SIZE;
    if (position.y > 600 - SIZE) position.y = 600 - SIZE;
}

bool Enemy::CheckCollision(Rectangle rect) {
    Rectangle enemyRect = GetBounds();
    if (CheckCollisionRecs(enemyRect, rect)) {
        position.x -= velocity.x * GetFrameTime();
        position.y -= velocity.y * GetFrameTime();
        return true;
    }
    return false;
}

void Enemy::Draw() {
    if (alive) {
        DrawRectangleV(position, {SIZE, SIZE}, color);
        DrawRectangleLinesEx({position.x, position.y, SIZE, SIZE}, 2, {139, 0, 0, 255});
        
        DrawCircleV({position.x + SIZE/2, position.y + SIZE/2 - 2}, 2, WHITE);
        DrawCircleV({position.x + SIZE/2 + 4, position.y + SIZE/2 - 2}, 2, WHITE);
    }
}

Rectangle Enemy::GetBounds() const {
    return {position.x, position.y, SIZE, SIZE};
}