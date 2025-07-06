#include "game.h"

Game::Game() : player(Vector2{100, 100}) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Retro Dungeon");
    SetTargetFPS(60);
    
    camera.target = player.GetPosition();
    camera.offset = Vector2{SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    dungeon.Generate();
    
    enemies.push_back(Enemy(Vector2{300, 200}));
    enemies.push_back(Enemy(Vector2{500, 400}));
    enemies.push_back(Enemy(Vector2{200, 300}));
}

Game::~Game() {
    CloseWindow();
}

void Game::Run() {
    while (!WindowShouldClose()) {
        Update();
        Draw();
    }
}

void Game::Update() {
    player.Update();
    
    Vector2 playerPos = player.GetPosition();
    int playerTileX = (int)(playerPos.x / Dungeon::TILE_SIZE);
    int playerTileY = (int)(playerPos.y / Dungeon::TILE_SIZE);
    
    for (int x = playerTileX - 1; x <= playerTileX + 1; x++) {
        for (int y = playerTileY - 1; y <= playerTileY + 1; y++) {
            if (dungeon.IsWall(x, y)) {
                Rectangle wallRect = dungeon.GetWallRect(x, y);
                player.CheckCollision(wallRect);
            }
        }
    }
    
    for (auto& enemy : enemies) {
        if (enemy.IsAlive()) {
            enemy.Update(player.GetPosition(), {});
            
            Vector2 enemyPos = enemy.GetBounds().x < 0 ? Vector2{0, 0} : 
                              Vector2{enemy.GetBounds().x, enemy.GetBounds().y};
            int enemyTileX = (int)(enemyPos.x / Dungeon::TILE_SIZE);
            int enemyTileY = (int)(enemyPos.y / Dungeon::TILE_SIZE);
            
            for (int x = enemyTileX - 1; x <= enemyTileX + 1; x++) {
                for (int y = enemyTileY - 1; y <= enemyTileY + 1; y++) {
                    if (dungeon.IsWall(x, y)) {
                        Rectangle wallRect = dungeon.GetWallRect(x, y);
                        enemy.CheckCollision(wallRect);
                    }
                }
            }
        }
    }
    
    camera.target = player.GetPosition();
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground({20, 20, 30, 255});
    
    BeginMode2D(camera);
    
    dungeon.Draw();
    player.Draw();
    
    for (auto& enemy : enemies) {
        if (enemy.IsAlive()) {
            enemy.Draw();
        }
    }
    
    EndMode2D();
    
    DrawText("WASD to move", 10, 10, 20, {200, 200, 200, 255});
    DrawText("ESC to exit", 10, 40, 20, {200, 200, 200, 255});
    DrawText("RETRO DUNGEON", 10, SCREEN_HEIGHT - 30, 20, {150, 150, 150, 255});
    
    EndDrawing();
}