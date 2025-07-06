#include "room.h"

Room::Room(const std::string& name, const std::string& description) 
    : name(name), description(description), visited(false) {}

void Room::SetExit(const std::string& direction, Room* room) {
    exits[direction] = room;
}

Room* Room::GetExit(const std::string& direction) {
    auto it = exits.find(direction);
    if (it != exits.end()) {
        return it->second;
    }
    return nullptr;
}

void Room::AddItem(const Item& item) {
    items.push_back(item);
}

void Room::AddMonster(const Monster& monster) {
    monsters.push_back(monster);
}

bool Room::RemoveItem(const std::string& itemName) {
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (it->name == itemName && it->takeable) {
            items.erase(it);
            return true;
        }
    }
    return false;
}


std::string Room::GetDescription() const {
    std::string fullDesc = description;
    
    if (!items.empty()) {
        fullDesc += "\n\nYou see: ";
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) fullDesc += ", ";
            fullDesc += items[i].name;
        }
    }
    
    if (!monsters.empty()) {
        fullDesc += "\n\nCreatures: ";
        for (size_t i = 0; i < monsters.size(); ++i) {
            if (monsters[i].alive) {
                if (i > 0) fullDesc += ", ";
                fullDesc += monsters[i].name;
            }
        }
    }
    
    return fullDesc;
}

std::vector<std::string> Room::GetExits() const {
    std::vector<std::string> exitList;
    for (const auto& exit : exits) {
        exitList.push_back(exit.first);
    }
    return exitList;
}