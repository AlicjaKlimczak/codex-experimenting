#pragma once
#include "raylib.h"
#include "room.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

class TextAdventure {
public:
    TextAdventure();
    ~TextAdventure();
    
    void Run();
    
private:
    void Update();
    void Draw();
    void ProcessInput();
    void ExecuteCommand(const std::string& command);
    
    void InitializeDungeon();
    void DrawMap();
    void DrawTextPanel();
    void AddMessage(const std::string& message);
    
    std::vector<std::string> SplitString(const std::string& str, char delimiter);
    std::string ToLower(const std::string& str);
    std::vector<std::string> WrapText(const std::string& text, int maxWidth, int fontSize);
    
    // Game state
    Room* currentRoom;
    std::vector<std::unique_ptr<Room>> rooms;
    std::vector<Item> inventory;
    std::vector<std::string> messages;
    std::string currentInput;
    int playerHealth;
    int basePlayerAttack;
    int basePlayerArmor;
    
    // Equipment slots - use indices instead of pointers to avoid vector reallocation issues
    int equippedWeaponIndex;  // -1 if no weapon equipped
    int equippedArmorIndex;   // -1 if no armor equipped
    
    // Display constants
    static const int SCREEN_WIDTH = 1800;
    static const int SCREEN_HEIGHT = 1200;
    static const int MAP_WIDTH = 900;
    static const int TEXT_WIDTH = 880;
    static const int MAX_MESSAGES = 35;
    
    // Room view constants
    static const int ROOM_GRID_WIDTH = 24;
    static const int ROOM_GRID_HEIGHT = 18;
    static const int TILE_SIZE = 32;
    
    // Player position within current room
    float playerRoomX, playerRoomY;
    float playerSpeed;
    bool isFemale;
    float moveTimer;
    
    // Animation state
    int walkAnimFrame;
    float animTimer;
    bool isWalking;
    
    // Chat scrolling
    int chatScrollOffset;
    
    // Special discoveries
    bool bookTaken;
    bool scrollTaken;
    bool mapUnlocked;
    bool infirmaryRevealed;
    bool inMapView;
    bool hasKey;
    bool gemUsed;
    bool hasTeleport;
    bool strangeMet;
    
    void DrawCurrentRoom();
    void DrawPlayer();
    void DrawRoomLayout(Room* room);
    bool IsWalkable(int tileX, int tileY, Room* room);
    void MovePlayer(float deltaX, float deltaY);
    void UpdateMonsters();
    void CheckMonsterCollisions();
    float GetDistance(float x1, float y1, float x2, float y2);
    void AttackNearestMonster();
    void DrawPlayerStats();
    int GetTotalAttack() const;
    int GetTotalArmor() const;
    Item* FindItemInInventory(const std::string& itemName);
    int FindItemIndexInInventory(const std::string& itemName);
    void EquipItem(const std::string& itemName);
    void DropItem(const std::string& itemName);
    void ShowItemStats(const std::string& itemName);
    void DrawDungeonMap();
    void UseItem(const std::string& itemName);
    void TeleportToRoom(const std::string& roomName);
};