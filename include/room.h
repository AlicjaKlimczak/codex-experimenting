#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

enum class ItemType {
    MISC,
    WEAPON,
    ARMOR
};

struct Item {
    std::string name;
    std::string description;
    bool takeable;
    ItemType type;
    int damageBonus;  // For weapons
    int armorBonus;   // For armor
    
    Item(const std::string& n, const std::string& desc, bool take = true, 
         ItemType t = ItemType::MISC, int dmg = 0, int armor = 0)
        : name(n), description(desc), takeable(take), type(t), 
          damageBonus(dmg), armorBonus(armor) {}
};

struct Monster {
    std::string name;
    std::string description;
    int health;
    int attack;
    bool alive;
    float x, y;
    float targetX, targetY;
    float moveTimer;
    float aggroRange;
    bool isAggro;
    
    Monster(const std::string& n, const std::string& desc, int hp, int atk, float startX = 10.0f, float startY = 10.0f)
        : name(n), description(desc), health(hp), attack(atk), alive(true), 
          x(startX), y(startY), targetX(startX), targetY(startY), moveTimer(0.0f), 
          aggroRange(4.0f), isAggro(false) {}
};

class Room {
public:
    Room(const std::string& name, const std::string& description);
    
    void SetExit(const std::string& direction, Room* room);
    Room* GetExit(const std::string& direction);
    
    void AddItem(const Item& item);
    void AddMonster(const Monster& monster);
    
    bool RemoveItem(const std::string& itemName);
    
    std::string GetName() const { return name; }
    std::string GetDescription() const;
    std::vector<std::string> GetExits() const;
    std::vector<Item> GetItems() const { return items; }
    std::vector<Monster>& GetMonsters() { return monsters; }
    const std::vector<Monster>& GetMonsters() const { return monsters; }
    
    bool IsVisited() const { return visited; }
    void SetVisited(bool v) { visited = v; }
    
private:
    std::string name;
    std::string description;
    std::map<std::string, Room*> exits;
    std::vector<Item> items;
    std::vector<Monster> monsters;
    bool visited;
};