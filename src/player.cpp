#include "player.h"

const float Player::SIZE = 16.0f;

Player::Player() : position({100, 100}), velocity({0, 0}), speed(150.0f), color({0, 200, 0, 255}) {}

Player::Player(Vector2 startPos) : position(startPos), velocity({0, 0}), speed(150.0f), color({0, 200, 0, 255}) {}

void Player::Update() {
    velocity = {0, 0};
    
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) velocity.y = -speed;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) velocity.y = speed;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) velocity.x = -speed;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) velocity.x = speed;
    
    Vector2 newPos = {
        position.x + velocity.x * GetFrameTime(),
        position.y + velocity.y * GetFrameTime()
    };
    
    position = newPos;
}

void Player::Draw() {
    DrawRectangleV(position, {SIZE, SIZE}, color);
    DrawRectangleLinesEx({position.x, position.y, SIZE, SIZE}, 2, DARKGREEN);
}

bool Player::CheckCollision(Rectangle rect) {
    Rectangle playerRect = GetBounds();
    if (CheckCollisionRecs(playerRect, rect)) {
        position.x -= velocity.x * GetFrameTime();
        position.y -= velocity.y * GetFrameTime();
        return true;
    }
    return false;
}

Rectangle Player::GetBounds() const {
    return {position.x, position.y, SIZE, SIZE};
}