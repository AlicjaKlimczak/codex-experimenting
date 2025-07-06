#include "room_factory.h"

std::vector<std::unique_ptr<Room>> RoomFactory::CreateAllRooms() {
    std::vector<std::unique_ptr<Room>> rooms;
    
    rooms.push_back(CreateEntranceHall());      // Index 0
    rooms.push_back(CreateArmory());            // Index 1
    rooms.push_back(CreateTreasureChamber());   // Index 2
    rooms.push_back(CreateDarkCorridor());      // Index 3
    rooms.push_back(CreateMonsterLair());       // Index 4
    rooms.push_back(CreateLibrary());           // Index 5
    rooms.push_back(CreateKitchen());           // Index 6
    rooms.push_back(CreateBasement());          // Index 7
    rooms.push_back(CreateThroneRoom());        // Index 8
    rooms.push_back(CreateGarden());            // Index 9
    rooms.push_back(CreateInfirmary());         // Index 10
    rooms.push_back(CreateSunlitMeadow());      // Index 11
    rooms.push_back(CreateDarkRoom());          // Index 12
    rooms.push_back(CreateChapel());            // Index 13
    rooms.push_back(CreateSleepingQuarters());  // Index 14
    
    return rooms;
}

void RoomFactory::ConnectRooms(std::vector<std::unique_ptr<Room>>& rooms) {
    // Entrance Hall connections
    rooms[0]->SetExit("north", rooms[3].get());    // to corridor
    rooms[0]->SetExit("east", rooms[1].get());     // to armory
    rooms[0]->SetExit("west", rooms[5].get());     // to library
    rooms[0]->SetExit("south", rooms[6].get());    // to kitchen
    
    // Armory connections
    rooms[1]->SetExit("west", rooms[0].get());     // to entrance
    rooms[1]->SetExit("south", rooms[9].get());    // to garden
    // Note: east exit to treasure chamber is added dynamically when key is picked up
    
    // Treasure Chamber connections
    rooms[2]->SetExit("west", rooms[1].get());     // to armory
    rooms[2]->SetExit("east", rooms[8].get());     // to throne
    
    // Dark Corridor connections
    rooms[3]->SetExit("south", rooms[0].get());    // to entrance
    rooms[3]->SetExit("north", rooms[4].get());    // to lair
    
    // Monster Lair connections
    rooms[4]->SetExit("south", rooms[3].get());    // to corridor
    rooms[4]->SetExit("west", rooms[7].get());     // to basement
    
    // Library connections
    rooms[5]->SetExit("east", rooms[0].get());     // to entrance
    rooms[5]->SetExit("north", rooms[7].get());    // to basement
    
    // Kitchen connections
    rooms[6]->SetExit("north", rooms[0].get());    // to entrance
    rooms[6]->SetExit("east", rooms[9].get());     // to garden
    
    // Basement connections
    rooms[7]->SetExit("south", rooms[5].get());    // to library
    rooms[7]->SetExit("east", rooms[4].get());     // to lair
    
    // Throne Room connections
    rooms[8]->SetExit("west", rooms[2].get());     // to treasure
    
    // Garden connections
    rooms[9]->SetExit("west", rooms[6].get());     // to kitchen
    rooms[9]->SetExit("north", rooms[1].get());    // to armory
    rooms[9]->SetExit("down", rooms[12].get());    // to dark room
    
    // Dark Room connections
    rooms[12]->SetExit("up", rooms[9].get());      // to garden
    
    // Note: Chapel and Sleeping Quarters connections are added dynamically after note is read:
    // - Sunlit Meadow (11) <-> Chapel (13) via east/west
    // - Throne Room (8) <-> Sleeping Quarters (14) via north/south
}

std::unique_ptr<Room> RoomFactory::CreateEntranceHall() {
    auto room = std::make_unique<Room>("Entrance Hall", 
        "You stand in a dimly lit stone hall. Ancient torches flicker on the walls, casting dancing shadows. The air smells of dust and age.");
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateArmory() {
    auto room = std::make_unique<Room>("Armory", 
        "This room once held weapons and armor. Rusted sword racks line the walls, and broken shields litter the floor.");
    
    room->AddItem(Item("sword", "A sharp steel sword with a leather-wrapped hilt.", true, ItemType::WEAPON, 5, 0));
    room->AddItem(Item("shield", "A sturdy wooden shield reinforced with metal bands.", true, ItemType::ARMOR, 0, 3));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateTreasureChamber() {
    auto room = std::make_unique<Room>("Treasure Chamber", 
        "Gold coins and precious gems are scattered across the floor. A massive chest sits in the center, slightly ajar.");
    
    room->AddItem(Item("gold", "A handful of gleaming gold coins.", true, ItemType::MISC, 0, 0));
    room->AddItem(Item("gem", "A sparkling ruby that seems to pulse with inner light.", true, ItemType::MISC, 0, 0));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateDarkCorridor() {
    auto room = std::make_unique<Room>("Dark Corridor", 
        "A narrow, winding passage stretches before you. The walls are damp and covered in strange moss that glows faintly.");
    
    room->AddMonster(Monster("skeleton", "A rattling skeleton wielding a rusty sword.", 20, 8, 15.0f, 8.0f));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateMonsterLair() {
    auto room = std::make_unique<Room>("Monster Lair", 
        "Bones and debris cover the floor of this foul-smelling chamber. Claw marks scar the stone walls.");
    
    room->AddMonster(Monster("goblin", "A small, green-skinned creature with sharp teeth and claws.", 15, 5, 8.0f, 10.0f));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateLibrary() {
    auto room = std::make_unique<Room>("Library", 
        "Ancient books and scrolls fill wooden shelves that reach to the ceiling. Dust particles dance in shafts of light from somewhere above.");
    
    room->AddItem(Item("book", "An ancient tome of forgotten lore.", true, ItemType::MISC, 0, 0));
    room->AddItem(Item("scroll", "A mysterious scroll with strange symbols.", true, ItemType::MISC, 0, 0));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateKitchen() {
    auto room = std::make_unique<Room>("Kitchen", 
        "A medieval kitchen with stone ovens and wooden tables. Cast iron pots and cooking utensils hang from hooks on the walls.");
    
    room->AddItem(Item("pot", "A cast iron cooking pot.", true, ItemType::MISC, 0, 0));
    room->AddItem(Item("knife", "A sharp kitchen knife with a wooden handle.", true, ItemType::WEAPON, 2, 0));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateBasement() {
    auto room = std::make_unique<Room>("Basement", 
        "A damp underground storage room filled with wooden barrels and crates. The smell of aged wine and preserved foods fills the air.");
    
    room->AddItem(Item("wine", "A bottle of aged wine.", true, ItemType::MISC, 0, 0));
    room->AddItem(Item("key", "A rusty old key.", true, ItemType::MISC, 0, 0));
    room->AddMonster(Monster("rat", "A large, mangy rat with glowing red eyes.", 8, 3, 6.0f, 12.0f));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateThroneRoom() {
    auto room = std::make_unique<Room>("Throne Room", 
        "A grand chamber with a massive stone throne decorated with purple cushions. Tapestries depicting ancient battles hang on the walls.");
    
    room->AddItem(Item("crown", "A golden crown encrusted with jewels.", true, ItemType::ARMOR, 0, 2));
    room->AddMonster(Monster("ghost", "A translucent figure in royal robes, floating above the throne.", 50, 12, 18.0f, 6.0f));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateGarden() {
    auto room = std::make_unique<Room>("Garden", 
        "A small indoor garden with colorful flowers and herbs growing in neat rows. Sunlight streams through a glass ceiling above.");
    
    room->AddItem(Item("herbs", "A bundle of healing herbs.", true, ItemType::MISC, 0, 0));
    room->AddItem(Item("seeds", "A pouch of rare flower seeds.", true, ItemType::MISC, 0, 0));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateInfirmary() {
    auto room = std::make_unique<Room>("Infirmary", 
        "A small medical room with stone shelves lined with bottles and bandages. A wooden cot sits in the corner, and healing herbs hang from the ceiling.");
    
    room->AddItem(Item("plaster", "A magical healing plaster that restores 20 health.", true, ItemType::MISC, 0, 0));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateSunlitMeadow() {
    auto room = std::make_unique<Room>("Sunlit Meadow", 
        "You emerge into a beautiful meadow filled with wildflowers and tall grass. The warm sun shines down on your face as a gentle breeze carries the scent of freedom. Birds chirp in the distance, and you can see rolling hills stretching to the horizon.");
    
    room->AddItem(Item("note", "A weathered piece of parchment, torn and yellowed with age.", true, ItemType::MISC, 0, 0));
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateDarkRoom() {
    auto room = std::make_unique<Room>("Dark Room", 
        "You find yourself in a pitch-black underground chamber. The only light comes from glowing crystals embedded in the walls, casting eerie shadows that dance and flicker. The air is thick with ancient magic, and you sense you are not alone here.");
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateChapel() {
    auto room = std::make_unique<Room>("Chapel", 
        "A small, sacred chamber with stone walls covered in ancient religious symbols. Candles flicker on a simple altar, casting dancing shadows across weathered religious carvings. The air is heavy with incense and reverence.");
    
    // Add guardian creature
    room->AddMonster(Monster("guardian spirit", "A translucent figure in ancient robes, protecting the sacred diamond.", 35, 10, 12.0f, 6.0f));
    
    // Diamond will be added dynamically after note is read
    
    return room;
}

std::unique_ptr<Room> RoomFactory::CreateSleepingQuarters() {
    auto room = std::make_unique<Room>("Sleeping Quarters", 
        "A modest room with simple stone beds arranged along the walls. Faded tapestries hang between the sleeping areas, and a few personal belongings are scattered about. Dust motes dance in the dim light filtering through a small window.");
    
    // Add nightmare creature
    room->AddMonster(Monster("nightmare wraith", "A dark, shadowy creature that haunts sleeping minds and guards the opal.", 30, 9, 8.0f, 10.0f));
    
    // Opal will be added dynamically after note is read
    
    return room;
}