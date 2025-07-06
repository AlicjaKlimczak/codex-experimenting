#include "textadventure.h"
#include "room_factory.h"
#include <algorithm>
#include <iostream>
#include <cmath>

TextAdventure::TextAdventure() : currentRoom(nullptr), playerHealth(100), basePlayerAttack(3), basePlayerArmor(1), equippedWeaponIndex(-1), equippedArmorIndex(-1), playerRoomX(12.0f), playerRoomY(9.0f), playerSpeed(4.0f), isFemale(false), moveTimer(0.0f), walkAnimFrame(0), animTimer(0.0f), isWalking(false), chatScrollOffset(0), bookTaken(false), scrollTaken(false), mapUnlocked(false), infirmaryRevealed(false), inMapView(false), hasKey(false), gemUsed(false), hasTeleport(false), strangeMet(false), noteRead(false), hasStaff(false), hasDiamond(false), hasEmerald(false), hasOpal(false), staffComplete(false), gameEnding(false), shouldQuit(false), waitingForContinue(false), endingPhase(0), endingTimer(0.0f) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Retro Dungeon - Text Adventure");
    SetExitKey(-1); // Disable ESC key from closing the window
    SetTargetFPS(60);
    
    // Character selection
    AddMessage("Welcome to the Retro Dungeon!");
    AddMessage("Choose your character: Type 'male' or 'female'");
    AddMessage("");
    
    InitializeDungeon();
    AddMessage("Welcome to the Retro Dungeon!");
    AddMessage("Type 'help' for commands, 'look' to examine your surroundings.");
    AddMessage("Use 'go north', 'go south', 'go east', 'go west' to move.");
}

TextAdventure::~TextAdventure() {
    CloseWindow();
}

void TextAdventure::Run() {
    while (!shouldQuit && !WindowShouldClose()) {
        Update();
        Draw();
    }
}

void TextAdventure::Update() {
    ProcessInput();
    
    moveTimer += GetFrameTime();
    animTimer += GetFrameTime();
    
    // Handle ending sequence transitions
    if (endingPhase > 0) {
        endingTimer += GetFrameTime();
        
        // Transition every 1.5 seconds
        if (endingTimer >= 1.5f) {
            endingPhase++;
            endingTimer = 0.0f;
            
            if (endingPhase > 5) {
                endingPhase = 5; // Stay at game over
            }
        }
    }
    
    // Walking animation - cycle through frames (much slower, each pose held longer)
    if (isWalking && animTimer >= 0.4f) {
        walkAnimFrame = (walkAnimFrame + 1) % 4; // 4 frame walk cycle, each held longer
        animTimer = 0.0f;
    }
    
    // Retro movement - grid-based with animation (Arrow keys only)
    bool keyPressed = false;
    if (moveTimer >= 0.08f) { // Slightly faster for smoother animation
        if (IsKeyDown(KEY_UP)) {
            MovePlayer(0, -1);
            moveTimer = 0.0f;
            keyPressed = true;
        }
        else if (IsKeyDown(KEY_DOWN)) {
            MovePlayer(0, 1);
            moveTimer = 0.0f;
            keyPressed = true;
        }
        else if (IsKeyDown(KEY_LEFT)) {
            MovePlayer(-1, 0);
            moveTimer = 0.0f;
            keyPressed = true;
        }
        else if (IsKeyDown(KEY_RIGHT)) {
            MovePlayer(1, 0);
            moveTimer = 0.0f;
            keyPressed = true;
        }
    }
    
    // Update walking state
    isWalking = keyPressed || (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || 
                               IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT));
    
    if (!isWalking) {
        walkAnimFrame = 0; // Reset to idle frame
    }
    
    // Attack with Delete key or Spacebar
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_SPACE)) {
        AttackNearestMonster();
    }
    
    // Exit map view with Shift key
    if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) {
        if (inMapView) {
            inMapView = false;
            AddMessage("Closing map view.");
        }
    }
    
    // Handle teleport clicks on map
    if (inMapView && hasTeleport && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        // Check if click is within map area
        if (mousePos.x >= 20 && mousePos.x <= 20 + MAP_WIDTH && mousePos.y >= 20 && mousePos.y <= SCREEN_HEIGHT - 40) {
            // Will be implemented in DrawDungeonMap function
        }
    }
    
    // Chat scrolling with Page Up/Page Down and Mouse Wheel
    if (IsKeyPressed(KEY_PAGE_UP)) {
        chatScrollOffset = std::max(0, chatScrollOffset - 5);
    }
    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        chatScrollOffset += 5; // Will be clamped in DrawTextPanel
    }
    
    // Mouse wheel scrolling
    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0) {
        int scrollAmount = (int)(wheelMove * 3); // 3 lines per wheel step
        chatScrollOffset = std::max(0, chatScrollOffset - scrollAmount);
        // Max scroll will be handled in DrawTextPanel
    }
    
    UpdateMonsters();
    CheckMonsterCollisions();
}

void TextAdventure::ProcessInput() {
    int key = GetCharPressed();
    
    while (key > 0) {
        if (key >= 32 && key <= 126) {
            currentInput += (char)key;
        }
        key = GetCharPressed();
    }
    
    if (IsKeyPressed(KEY_ENTER)) {
        if (!currentInput.empty()) {
            AddMessage("> " + currentInput);
            ExecuteCommand(currentInput);
            currentInput.clear();
        }
    }
    
    if (IsKeyPressed(KEY_BACKSPACE) && !currentInput.empty()) {
        currentInput.pop_back();
    }
}

void TextAdventure::ExecuteCommand(const std::string& command) {
    std::vector<std::string> words = SplitString(ToLower(command), ' ');
    
    if (words.empty()) return;
    
    std::string verb = words[0];
    
    if (verb == "help") {
        AddMessage("=== COMMAND HELP ===");
        AddMessage("");
        AddMessage("MOVEMENT:");
        AddMessage("  go [direction] - Move to another room (north, south, east, west)");
        AddMessage("  Arrow Keys - Move character within room");
        AddMessage("");
        AddMessage("EXPLORATION:");
        AddMessage("  look - Examine current room and see exits");
        AddMessage("  take [item] - Pick up an item from the room");
        AddMessage("  inventory (or inv) - View your carried items");
        AddMessage("  use [item] - Use an item from your inventory");
        AddMessage("  map - View dungeon map (requires scroll)");
        AddMessage("");
        AddMessage("EQUIPMENT:");
        AddMessage("  equip [item] - Equip any item (weapons give attack, armor gives protection)");
        AddMessage("  drop [item] - Drop an item from inventory to current room");
        AddMessage("  stats [item] - View detailed item statistics and bonuses");
        AddMessage("");
        AddMessage("COMBAT:");
        AddMessage("  DELETE/SPACEBAR - Attack nearby monsters");
        AddMessage("  Get close to monsters (within 3 tiles) to attack them");
        AddMessage("");
        AddMessage("CHARACTER:");
        AddMessage("  male - Set character as male");
        AddMessage("  female - Set character as female");
        AddMessage("");
        AddMessage("INTERFACE:");
        AddMessage("  Page Up/Page Down - Scroll through chat history");
        AddMessage("  Mouse Wheel - Scroll through chat history");
        AddMessage("  help - Show this help screen");
        AddMessage("  quit - Exit the game");
    }
    else if (verb == "male") {
        isFemale = false;
        AddMessage("You are now a male character.");
    }
    else if (verb == "female") {
        isFemale = true;
        AddMessage("You are now a female character.");
    }
    else if (verb == "look") {
        AddMessage(currentRoom->GetName());
        
        // Get description with exits included
        std::string fullDesc = currentRoom->GetDescription();
        auto exits = currentRoom->GetExits();
        if (!exits.empty()) {
            fullDesc += "\n\nExits: ";
            for (size_t i = 0; i < exits.size(); ++i) {
                if (i > 0) fullDesc += ", ";
                fullDesc += exits[i];
            }
        }
        
        // Add locked exits for armory
        if (currentRoom->GetName() == "Armory" && !hasKey) {
            if (!exits.empty()) {
                fullDesc += ", east (locked)";
            } else {
                fullDesc += "\n\nExits: east (locked)";
            }
        }
        
        AddMessage(fullDesc);
    }
    else if (verb == "go" && words.size() > 1) {
        std::string direction = words[1];
        
        // Check for locked doors
        if (currentRoom->GetName() == "Armory" && direction == "east" && !hasKey) {
            AddMessage("The door to the east is locked with a heavy iron lock.");
            AddMessage("You need a key to open it.");
            return;
        }
        
        Room* nextRoom = currentRoom->GetExit(direction);
        
        if (nextRoom) {
            currentRoom = nextRoom;
            currentRoom->SetVisited(true);
            playerRoomX = 10.0f;
            playerRoomY = 8.0f;
            
            AddMessage("You go " + direction + ".");
            AddMessage(currentRoom->GetName());
            
            // Check for win condition
            if (currentRoom->GetName() == "Sunlit Meadow") {
                AddMessage("After what feels like an eternity in the dark dungeon, you finally breathe fresh air!");
                AddMessage("The nightmare is over. You have escaped the Retro Dungeon!");
                AddMessage("");
                AddMessage("=== GAME OVER. YOU WIN! ===");
                AddMessage("...or do you?");
                AddMessage("");
                AddMessage("Thank you for playing! Press any key to continue exploring...");
                return; // Don't show exits or description for ending
            }
            
            // Check for dark room stranger interaction
            if (currentRoom->GetName() == "Dark Room" && !strangeMet) {
                AddMessage("A mysterious figure emerges from the shadows...");
                AddMessage("'Welcome, traveler,' whispers a hooded stranger with glowing eyes.");
                AddMessage("'I seek gold... in exchange for something truly special.'");
                
                // Check if player has gold
                bool hasGold = false;
                for (const auto& item : inventory) {
                    if (item.name == "gold") {
                        hasGold = true;
                        break;
                    }
                }
                
                if (hasGold) {
                    AddMessage("'I see you carry gold... use it here if you wish to trade.'");
                } else if (hasKey) {
                    AddMessage("'You seek gold? Take a look in the treasure chamber...'");
                } else {
                    AddMessage("'You seek gold? Maybe you can find a key in the basement to unlock another room...'");
                }
            }
            
            // Get description with exits included
            std::string fullDesc = currentRoom->GetDescription();
            auto exits = currentRoom->GetExits();
            if (!exits.empty()) {
                fullDesc += "\n\nExits: ";
                for (size_t i = 0; i < exits.size(); ++i) {
                    if (i > 0) fullDesc += ", ";
                    fullDesc += exits[i];
                }
            }
            
            // Add locked exits for armory
            if (currentRoom->GetName() == "Armory" && !hasKey) {
                if (!exits.empty()) {
                    fullDesc += ", east (locked)";
                } else {
                    fullDesc += "\n\nExits: east (locked)";
                }
            }
            
            AddMessage(fullDesc);
        } else {
            AddMessage("You can't go that way.");
        }
    }
    else if (verb == "take" && words.size() > 1) {
        std::string itemName = words[1];
        
        // Find the item in the room first to get its full details
        const auto& roomItems = currentRoom->GetItems();
        Item* foundItem = nullptr;
        for (const auto& item : roomItems) {
            if (item.name == itemName && item.takeable) {
                foundItem = const_cast<Item*>(&item);
                break;
            }
        }
        
        if (foundItem && currentRoom->RemoveItem(itemName)) {
            inventory.push_back(*foundItem); // Copy the complete item with all its properties
            
            // Special interactions for specific items
            if (itemName == "book" && !bookTaken) {
                bookTaken = true;
                AddMessage("You take the ancient tome. As you lift it, you notice a hidden lever behind it!");
                AddMessage("You pull the lever and hear a rumbling sound from somewhere nearby...");
                AddMessage("A secret passage has opened in the library! You can now go 'south' to the Infirmary.");
                
                // Connect library to infirmary
                Room* library = rooms[5].get();
                Room* infirmary = rooms[10].get();
                library->SetExit("south", infirmary);
                infirmary->SetExit("north", library);
                infirmaryRevealed = true;
            }
            else if (itemName == "scroll" && !scrollTaken) {
                scrollTaken = true;
                AddMessage("You take the mysterious scroll. As you unroll it, ancient symbols glow briefly!");
                AddMessage("The scroll contains a map enchantment! You have learned the 'map' command.");
                AddMessage("Use 'map' to view the entire dungeon layout.");
                mapUnlocked = true;
            }
            else if (itemName == "key" && !hasKey) {
                hasKey = true;
                AddMessage("You take the rusty old key. It feels heavy and important in your hand.");
                AddMessage("This key looks like it might unlock something significant...");
                
                // Unlock the path from armory to treasure chamber
                Room* armory = rooms[1].get();
                Room* treasure = rooms[2].get();
                armory->SetExit("east", treasure);
                
                AddMessage("You hear a distant clicking sound from somewhere in the dungeon!");
                AddMessage("The armory door to the east has been unlocked!");
            }
            else if (itemName == "gem") {
                AddMessage("You take the sparkling ruby. Its inner light pulses mysteriously.");
                AddMessage("This gem seems special... perhaps it belongs somewhere significant like a throne room?");
            }
            else if (itemName == "staff") {
                AddMessage("You take the ancient wooden staff. The runes along its surface begin to glow faintly.");
                AddMessage("This feels like an incredibly powerful artifact. You sense it was once whole...");
                hasStaff = true;
            }
            else if (itemName == "diamond") {
                AddMessage("You take the flawless diamond. It resonates with pure, brilliant energy.");
                AddMessage("This diamond seems to be part of something greater...");
                hasDiamond = true;
            }
            else if (itemName == "emerald") {
                AddMessage("You take the brilliant emerald. It pulses with vibrant green light.");
                AddMessage("You feel nature's power flowing through this gem...");
                hasEmerald = true;
            }
            else if (itemName == "opal") {
                AddMessage("You take the shimmering opal. It shifts through all colors of the rainbow.");
                AddMessage("This opal seems to contain the essence of all elements...");
                hasOpal = true;
            }
            
            // Check if all staff parts are collected
            if (hasStaff && hasDiamond && hasEmerald && hasOpal && !staffComplete) {
                AddMessage("");
                AddMessage("The four artifacts resonate with each other in your inventory!");
                AddMessage("The staff parts seem to be calling out to be reunited...");
                AddMessage("You sense you can now 'combine' them to restore the ancient staff!");
            }
            else {
                AddMessage("You take the " + itemName + ".");
            }
        } else {
            AddMessage("You can't take that.");
        }
    }
    else if (verb == "inventory" || verb == "inv") {
        if (inventory.empty()) {
            AddMessage("Your inventory is empty.");
        } else {
            std::string invStr = "You are carrying: ";
            for (size_t i = 0; i < inventory.size(); ++i) {
                if (i > 0) invStr += ", ";
                invStr += inventory[i].name;
            }
            AddMessage(invStr);
        }
    }
    else if (verb == "equip" && words.size() > 1) {
        std::string itemName = words[1];
        EquipItem(itemName);
    }
    else if (verb == "drop" && words.size() > 1) {
        std::string itemName = words[1];
        DropItem(itemName);
    }
    else if (verb == "stats" && words.size() > 1) {
        std::string itemName = words[1];
        ShowItemStats(itemName);
    }
    else if (verb == "use" && words.size() > 1) {
        std::string itemName = words[1];
        UseItem(itemName);
    }
    else if (verb == "map") {
        if (mapUnlocked) {
            inMapView = true;
            if (hasTeleport) {
                AddMessage("Opening map view... Press SHIFT to exit. Click on any explored room to teleport there!");
            } else {
                AddMessage("Opening map view... Press SHIFT to exit.");
            }
        } else {
            AddMessage("You need to take a better look in the library.");
        }
    }
    else if (verb == "combine") {
        // Check if all staff parts are collected
        if (hasStaff && hasDiamond && hasEmerald && hasOpal && !staffComplete) {
            // Remove individual parts from inventory
            for (auto it = inventory.begin(); it != inventory.end(); ) {
                if (it->name == "staff" || it->name == "diamond" || it->name == "emerald" || it->name == "opal") {
                    it = inventory.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Add the complete staff to inventory
            inventory.push_back(Item("?????", "The Ancient Staff of Power, now fully restored with all three gems embedded in its head. It pulses with magical energy.", true, ItemType::WEAPON, 25, 0));
            
            // Set completion flag
            staffComplete = true;
            
            // Victory message
            AddMessage("The staff parts resonate with ancient power as you bring them together!");
            AddMessage("The diamond, emerald, and opal float from your hands and embed themselves into the staff head.");
            AddMessage("Light erupts from the completed staff as its true power is unleashed!");
            AddMessage("You now wield the Ancient Staff of Power! (+25 attack)");
            AddMessage("The way forward is now clear...");
            
        } else if (staffComplete) {
            AddMessage("The staff is already complete and pulsing with power.");
        } else {
            AddMessage("You need to collect all the staff parts first:");
            AddMessage("- The wooden staff");
            AddMessage("- The diamond gem");
            AddMessage("- The emerald gem");
            AddMessage("- The opal gem");
        }
    }
    else if (verb == "continue" && waitingForContinue) {
        AddMessage("The world begins to change...");
        waitingForContinue = false;
        endingPhase = 1; // Start visual transition
    }
    else if (verb == "quit") {
        AddMessage("Thanks for playing!");
        shouldQuit = true;
    }
    else {
        AddMessage("I don't understand that command.");
    }
}

void TextAdventure::Draw() {
    BeginDrawing();
    ClearBackground({20, 20, 30, 255});
    
    if (inMapView) {
        DrawDungeonMap();
    } else {
        DrawCurrentRoom();
    }
    DrawTextPanel();
    DrawPlayerStats();
    
    // Handle ending visual effects over the room area
    if (endingPhase > 0) {
        Color overlayColor = {0, 0, 0, 0};
        
        switch (endingPhase) {
            case 1: // White
                overlayColor = {255, 255, 255, 200};
                break;
            case 2: // Yellow
                overlayColor = {255, 255, 0, 220};
                break;
            case 3: // Red
                overlayColor = {255, 0, 0, 240};
                break;
            case 4: // Black
                overlayColor = {0, 0, 0, 255};
                break;
            case 5: // Game Over
                overlayColor = {0, 0, 0, 255};
                // Draw game over text in the room area
                DrawRectangle(0, 0, MAP_WIDTH, SCREEN_HEIGHT, {0, 0, 0, 255});
                
                int gameOverY = SCREEN_HEIGHT / 2 - 50;
                DrawText("GAME OVER", MAP_WIDTH / 2 - 120, gameOverY, 48, {255, 255, 255, 255});
                
                DrawText("You have discovered the terrible truth of the Ancient Staff.", 
                         50, gameOverY + 80, 20, {200, 200, 200, 255});
                DrawText("Its power was never meant to be unleashed upon the world.", 
                         50, gameOverY + 110, 20, {200, 200, 200, 255});
                DrawText("The wizard who broke it was trying to save everyone...", 
                         50, gameOverY + 140, 20, {200, 200, 200, 255});
                DrawText("But it's too late now.", 
                         50, gameOverY + 170, 20, {200, 200, 200, 255});
                
                DrawText("Type 'quit' to exit.", 
                         50, gameOverY + 220, 24, {255, 255, 100, 255});
                break;
        }
        
        // Draw overlay for phases 1-4
        if (endingPhase >= 1 && endingPhase <= 4) {
            DrawRectangle(0, 0, MAP_WIDTH, SCREEN_HEIGHT, overlayColor);
        }
    }
    
    EndDrawing();
}

void TextAdventure::DrawCurrentRoom() {
    DrawRectangle(20, 20, MAP_WIDTH, SCREEN_HEIGHT - 40, {30, 30, 40, 255});
    DrawRectangleLines(20, 20, MAP_WIDTH, SCREEN_HEIGHT - 40, {100, 100, 120, 255});
    
    std::string roomTitle = currentRoom ? currentRoom->GetName() : "Unknown Room";
    DrawText(roomTitle.c_str(), 40, 40, 32, {220, 220, 220, 255});
    
    if (currentRoom) {
        DrawRoomLayout(currentRoom);
    }
    
    DrawPlayer();
    
    DrawText("Arrow Keys to move", 40, SCREEN_HEIGHT - 90, 16, {200, 200, 200, 255});
    DrawText("Commands: look, take [item]", 40, SCREEN_HEIGHT - 65, 16, {150, 150, 150, 255});
    DrawText("DELETE/SPACEBAR to attack monsters", 40, SCREEN_HEIGHT - 45, 16, {150, 150, 150, 255});
}

void TextAdventure::DrawTextPanel() {
    int textX = MAP_WIDTH + 40;
    int textY = 40;
    
    // Make text panel shorter to leave room for stats and input
    int panelHeight = SCREEN_HEIGHT - 300; // Leave 300px at bottom
    DrawRectangle(textX, 20, TEXT_WIDTH, panelHeight, {25, 25, 35, 255});
    DrawRectangleLines(textX, 20, TEXT_WIDTH, panelHeight, {100, 100, 120, 255});
    
    DrawText("ADVENTURE LOG", textX + 20, textY, 32, {220, 220, 220, 255});
    
    int messageStartY = textY + 60;
    int lineHeight = 32; // Much larger line height to completely prevent overlap
    int maxWidth = TEXT_WIDTH - 100; // Even more conservative width
    int fontSize = 18; // Larger font size for better readability
    
    // Calculate available space for messages more conservatively
    int availableHeight = panelHeight - 120; // More space reserved for headers/scrolling
    int maxDisplayLines = availableHeight / lineHeight;
    
    // Process messages and count total display lines needed
    std::vector<std::string> displayLines;
    std::vector<Color> lineColors;
    
    for (const auto& message : messages) {
        Color textColor = {200, 200, 200, 255};
        if (message.find("> ") == 0) {
            textColor = {120, 255, 120, 255};
        }
        
        // Word wrap the message
        std::vector<std::string> wrappedLines = WrapText(message, maxWidth, fontSize);
        for (const auto& line : wrappedLines) {
            displayLines.push_back(line);
            lineColors.push_back(textColor);
        }
    }
    
    // Calculate scroll range
    int totalLines = displayLines.size();
    int maxScrollOffset = std::max(0, totalLines - maxDisplayLines);
    
    // Robust auto-scroll: handle rapid messages properly
    static bool manuallyScrolled = false;
    static int previousTotalLines = 0;
    static float lastAutoScrollTime = 0.0f;
    float currentTime = GetTime();
    
    // Detect manual scrolling
    if (IsKeyPressed(KEY_PAGE_UP) || IsKeyPressed(KEY_PAGE_DOWN) || GetMouseWheelMove() != 0) {
        manuallyScrolled = true;
    }
    
    // Reset manual scroll flag when new content arrives, but with a small delay to handle rapid messages
    if (totalLines > previousTotalLines) {
        // If it's been a short time since last auto-scroll, keep scrolling
        if (currentTime - lastAutoScrollTime < 0.5f || !manuallyScrolled) {
            manuallyScrolled = false;
        }
        previousTotalLines = totalLines;
        lastAutoScrollTime = currentTime;
    }
    
    // Auto-scroll to bottom unless manually scrolled
    if (!manuallyScrolled) {
        chatScrollOffset = maxScrollOffset;
    }
    
    // Clamp scroll offset
    chatScrollOffset = std::max(0, std::min(maxScrollOffset, chatScrollOffset));
    
    // Draw visible lines with very careful bounds checking
    int startLine = chatScrollOffset;
    int endLine = std::min(totalLines, startLine + maxDisplayLines);
    
    int currentY = messageStartY;
    int panelBottom = textY + panelHeight - 80; // Reserve more space at bottom
    
    for (int i = startLine; i < endLine; i++) {
        // Only draw if there's enough room for the full line
        if (currentY + fontSize + 5 <= panelBottom) { // 5px extra safety margin
            DrawText(displayLines[i].c_str(), textX + 50, currentY, fontSize, lineColors[i]);
            currentY += lineHeight;
        } else {
            break; // Stop drawing if we run out of room
        }
    }
    
    // Show scroll indicator at bottom without blocking messages
    if (totalLines > maxDisplayLines) {
        std::string scrollInfo = "(" + std::to_string(startLine + 1) + "-" + std::to_string(endLine) + 
                                "/" + std::to_string(totalLines) + ") PgUp/PgDn/Wheel to scroll";
        int scrollY = textY + panelHeight - 30; // Position at very bottom with more space
        DrawRectangle(textX + 10, scrollY - 5, TEXT_WIDTH - 20, 25, {20, 20, 30, 220}); // Darker background
        DrawText(scrollInfo.c_str(), textX + 20, scrollY, 14, {180, 180, 180, 255});
    }
    
    // Text input area at the very bottom
    std::string inputText = "> " + currentInput;
    int inputY = SCREEN_HEIGHT - 60;
    DrawRectangle(textX + 10, inputY, TEXT_WIDTH - 20, 40, {40, 40, 50, 255});
    DrawRectangleLines(textX + 10, inputY, TEXT_WIDTH - 20, 40, {100, 100, 120, 255});
    DrawText(inputText.c_str(), textX + 20, inputY + 12, 20, {255, 255, 120, 255});
}

void TextAdventure::DrawRoomLayout(Room* room) {
    int startX = 40;
    int startY = 80;
    
    std::string roomName = room->GetName();
    
    for (int y = 0; y < ROOM_GRID_HEIGHT; y++) {
        for (int x = 0; x < ROOM_GRID_WIDTH; x++) {
            int posX = startX + x * TILE_SIZE;
            int posY = startY + y * TILE_SIZE;
            
            bool isWall = (x == 0 || x == ROOM_GRID_WIDTH - 1 || y == 0 || y == ROOM_GRID_HEIGHT - 1);
            
            if (isWall) {
                DrawRectangle(posX, posY, TILE_SIZE, TILE_SIZE, {60, 40, 30, 255});
                
                for (int px = 0; px < TILE_SIZE; px += 4) {
                    for (int py = 0; py < TILE_SIZE; py += 4) {
                        if ((px + py) % 8 == 0) {
                            DrawRectangle(posX + px, posY + py, 4, 4, {80, 60, 40, 255});
                        } else {
                            DrawRectangle(posX + px, posY + py, 4, 4, {50, 30, 20, 255});
                        }
                    }
                }
            } else {
                Color floorColor;
                if (roomName == "Entrance Hall") {
                    floorColor = {96, 80, 64, 255};
                } else if (roomName == "Armory") {
                    floorColor = {80, 80, 96, 255};
                } else if (roomName == "Treasure Chamber") {
                    floorColor = {128, 112, 48, 255};
                } else if (roomName == "Dark Corridor") {
                    floorColor = {32, 32, 48, 255};
                } else if (roomName == "Monster Lair") {
                    floorColor = {64, 48, 48, 255};
                } else if (roomName == "Library") {
                    floorColor = {80, 64, 48, 255};
                } else if (roomName == "Kitchen") {
                    floorColor = {80, 64, 48, 255};
                } else if (roomName == "Basement") {
                    floorColor = {32, 32, 32, 255};
                } else if (roomName == "Throne Room") {
                    floorColor = {64, 48, 80, 255};
                } else if (roomName == "Garden") {
                    floorColor = {48, 80, 32, 255};
                } else if (roomName == "Infirmary") {
                    floorColor = {96, 96, 112, 255}; // Light blue-gray for medical room
                } else if (roomName == "Dark Room") {
                    floorColor = {32, 32, 48, 255}; // Very dark blue-gray for mysterious room
                } else if (roomName == "Sunlit Meadow") {
                    floorColor = {144, 238, 144, 255}; // Light green for meadow
                } else if (roomName == "Chapel") {
                    floorColor = {96, 96, 112, 255}; // Stone gray for sacred space
                } else if (roomName == "Sleeping Quarters") {
                    floorColor = {112, 96, 80, 255}; // Warm wood tone for bedrooms
                } else {
                    floorColor = {96, 80, 64, 255};
                }
                
                for (int px = 0; px < TILE_SIZE; px += 8) {
                    for (int py = 0; py < TILE_SIZE; py += 8) {
                        Color pixelColor = floorColor;
                        if ((px + py) % 16 == 0) {
                            pixelColor.r = (pixelColor.r > 16) ? pixelColor.r - 16 : 0;
                            pixelColor.g = (pixelColor.g > 16) ? pixelColor.g - 16 : 0;
                            pixelColor.b = (pixelColor.b > 16) ? pixelColor.b - 16 : 0;
                        }
                        DrawRectangle(posX + px, posY + py, 8, 8, pixelColor);
                    }
                }
            }
        }
    }
    
    if (roomName == "Entrance Hall") {
        // Stone pillars with bright torches - properly sized
        for (int i = 0; i < 3; i++) {
            int pillarX = startX + (3 + i * 6) * TILE_SIZE;
            int pillarY = startY + 3 * TILE_SIZE;
            
            // Stone pillar base - fits within room bounds
            for (int px = 0; px < TILE_SIZE; px += 8) {
                for (int py = 0; py < TILE_SIZE * 3; py += 8) {
                    DrawRectangle(pillarX + px, pillarY + py, 8, 8, {120, 100, 85, 255});
                }
            }
            // Pillar outline for clarity
            DrawRectangleLines(pillarX, pillarY, TILE_SIZE, TILE_SIZE * 3, {80, 60, 45, 255});
            
            // Bright torch flame - visible but contained
            for (int px = 4; px < TILE_SIZE - 4; px += 8) {
                for (int py = 0; py < 16; py += 8) {
                    DrawRectangle(pillarX + px, pillarY - 8 + py, 8, 8, {255, 180, 0, 255});
                }
            }
            // Flame outline
            DrawRectangleLines(pillarX + 4, pillarY - 8, TILE_SIZE - 8, 16, {255, 100, 0, 255});
        }
        
        // Central altar - properly sized
        int altarX = startX + 9 * TILE_SIZE;
        int altarY = startY + 8 * TILE_SIZE;
        for (int px = 0; px < TILE_SIZE * 3; px += 8) {
            for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                DrawRectangle(altarX + px, altarY + py, 8, 8, {140, 120, 100, 255});
            }
        }
        // Altar outline
        DrawRectangleLines(altarX, altarY, TILE_SIZE * 3, TILE_SIZE * 2, {100, 80, 60, 255});
        
        // Decorative floor patterns - spaced properly
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 3; j++) {
                int decorX = startX + (2 + i * 5) * TILE_SIZE + 8;
                int decorY = startY + (12 + j * 2) * TILE_SIZE + 8;
                DrawRectangle(decorX, decorY, 16, 16, {160, 140, 120, 255});
            }
        }
        
        // Wall banners - properly positioned
        DrawRectangle(startX + TILE_SIZE + 8, startY + 2 * TILE_SIZE, 16, 48, {150, 0, 0, 255});
        DrawRectangle(startX + (ROOM_GRID_WIDTH - 2) * TILE_SIZE - 24, startY + 2 * TILE_SIZE, 16, 48, {150, 0, 0, 255});
        // Banner outlines
        DrawRectangleLines(startX + TILE_SIZE + 8, startY + 2 * TILE_SIZE, 16, 48, {100, 0, 0, 255});
        DrawRectangleLines(startX + (ROOM_GRID_WIDTH - 2) * TILE_SIZE - 24, startY + 2 * TILE_SIZE, 16, 48, {100, 0, 0, 255});
    }
    else if (roomName == "Armory") {
        // Weapon racks with bright, visible swords
        for (int i = 0; i < 4; i++) {
            int rackX = startX + (2 + i * 4) * TILE_SIZE;
            int rackY = startY + 2 * TILE_SIZE;
            
            // Wooden rack frame with outline - fits in room
            for (int px = 0; px < TILE_SIZE; px += 8) {
                for (int py = 0; py < TILE_SIZE * 3; py += 8) {
                    DrawRectangle(rackX + px, rackY + py, 8, 8, {120, 80, 40, 255});
                }
            }
            DrawRectangleLines(rackX, rackY, TILE_SIZE, TILE_SIZE * 3, {80, 50, 25, 255});
            
            // Bright swords on racks
            for (int j = 0; j < 3; j++) {
                int swordX = rackX + 4;
                int swordY = rackY + 8 + j * 20;
                // Bright silver blade
                DrawRectangle(swordX, swordY, 6, 20, {230, 230, 230, 255}); 
                DrawRectangleLines(swordX, swordY, 6, 20, {180, 180, 180, 255});
                // Brown hilt
                DrawRectangle(swordX - 1, swordY + 20, 8, 6, {160, 100, 50, 255}); 
                // Crossguard
                DrawRectangle(swordX - 2, swordY + 18, 10, 2, {140, 140, 140, 255});
            }
        }
        
        // Shield display on back wall - properly sized
        for (int i = 0; i < 3; i++) {
            int shieldX = startX + (6 + i * 4) * TILE_SIZE;
            int shieldY = startY + TILE_SIZE;
            
            // Round shields with bright colors
            for (int px = 0; px < 24; px += 8) {
                for (int py = 0; py < 24; py += 8) {
                    DrawRectangle(shieldX + px, shieldY + py, 8, 8, {190, 190, 190, 255});
                }
            }
            DrawRectangleLines(shieldX, shieldY, 24, 24, {120, 120, 120, 255});
            // Golden shield boss (center)
            DrawRectangle(shieldX + 8, shieldY + 8, 8, 8, {255, 215, 0, 255});
            DrawRectangleLines(shieldX + 8, shieldY + 8, 8, 8, {200, 170, 0, 255});
        }
        
        // Armor stands - properly sized
        for (int i = 0; i < 2; i++) {
            int armorX = startX + (5 + i * 8) * TILE_SIZE;
            int armorY = startY + 8 * TILE_SIZE;
            
            // Armor body with outline
            for (int px = 0; px < 24; px += 8) {
                for (int py = 0; py < 32; py += 8) {
                    DrawRectangle(armorX + px, armorY + py, 8, 8, {160, 160, 160, 255});
                }
            }
            DrawRectangleLines(armorX, armorY, 24, 32, {100, 100, 100, 255});
            // Helmet
            DrawRectangle(armorX + 4, armorY - 12, 16, 12, {150, 150, 150, 255});
            DrawRectangleLines(armorX + 4, armorY - 12, 16, 12, {90, 90, 90, 255});
        }
        
        // Scattered weapons on floor - properly sized
        // Sword on floor
        DrawRectangle(startX + 12 * TILE_SIZE, startY + 12 * TILE_SIZE, 20, 4, {230, 230, 230, 255});
        DrawRectangleLines(startX + 12 * TILE_SIZE, startY + 12 * TILE_SIZE, 20, 4, {180, 180, 180, 255});
        // Axe on floor
        DrawRectangle(startX + 6 * TILE_SIZE, startY + 13 * TILE_SIZE, 4, 16, {160, 100, 50, 255});
        DrawRectangle(startX + 6 * TILE_SIZE - 4, startY + 13 * TILE_SIZE, 12, 4, {150, 150, 150, 255});
    }
    else if (roomName == "Treasure Chamber") {
        // Treasure chest in center - properly sized
        int chestX = startX + 8 * TILE_SIZE;
        int chestY = startY + 6 * TILE_SIZE;
        for (int px = 0; px < TILE_SIZE * 3; px += 8) {
            for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                DrawRectangle(chestX + px, chestY + py, 8, 8, {130, 65, 20, 255});
            }
        }
        DrawRectangleLines(chestX, chestY, TILE_SIZE * 3, TILE_SIZE * 2, {90, 45, 15, 255});
        
        // Chest lock and hinges - bright and visible
        DrawRectangle(chestX + 16, chestY + 8, 12, 12, {255, 215, 0, 255});
        DrawRectangleLines(chestX + 16, chestY + 8, 12, 12, {200, 170, 0, 255});
        DrawRectangle(chestX, chestY, 12, 6, {80, 80, 80, 255});
        DrawRectangle(chestX + 36, chestY, 12, 6, {80, 80, 80, 255});
        
        // Treasure piles around the room - bright and visible
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 6; j++) {
                int coinX = startX + (2 * TILE_SIZE) + (i * 16) + (j % 2 * 8);
                int coinY = startY + (3 * TILE_SIZE) + (j * 12) + (i % 2 * 6);
                if (coinX < startX + 21 * TILE_SIZE && coinY < startY + 15 * TILE_SIZE) {
                    // Bright gold coins
                    DrawRectangle(coinX, coinY, 6, 6, {255, 215, 0, 255});
                    DrawRectangleLines(coinX, coinY, 6, 6, {200, 170, 0, 255});
                }
            }
        }
        
        // Gem piles - bright and visible
        for (int i = 0; i < 4; i++) {
            int gemX = startX + (4 + i * 4) * TILE_SIZE;
            int gemY = startY + (10 + (i % 2) * 2) * TILE_SIZE;
            // Bright gems
            DrawRectangle(gemX, gemY, 12, 12, {255, 0, 255, 255}); // purple gems
            DrawRectangleLines(gemX, gemY, 12, 12, {200, 0, 200, 255});
            DrawRectangle(gemX + 16, gemY, 12, 12, {0, 255, 0, 255}); // green gems
            DrawRectangleLines(gemX + 16, gemY, 12, 12, {0, 200, 0, 255});
            DrawRectangle(gemX + 32, gemY, 12, 12, {255, 0, 0, 255}); // red gems
            DrawRectangleLines(gemX + 32, gemY, 12, 12, {200, 0, 0, 255});
        }
        
        // Golden candlesticks - bright and visible
        for (int i = 0; i < 4; i++) {
            int candleX = startX + (2 + i * 4) * TILE_SIZE;
            int candleY = startY + 2 * TILE_SIZE;
            DrawRectangle(candleX, candleY, 12, 32, {255, 215, 0, 255});
            DrawRectangleLines(candleX, candleY, 12, 32, {200, 170, 0, 255});
            // Flame
            DrawRectangle(candleX + 2, candleY - 8, 8, 8, {255, 150, 0, 255}); 
        }
        
        // Ornate golden pillars - properly sized
        for (int i = 0; i < 2; i++) {
            int pillarX = startX + (3 + i * 12) * TILE_SIZE;
            int pillarY = startY + 1 * TILE_SIZE;
            for (int py = 0; py < TILE_SIZE * 5; py += 8) {
                DrawRectangle(pillarX, pillarY + py, 16, 8, {255, 215, 0, 255});
            }
            DrawRectangleLines(pillarX, pillarY, 16, TILE_SIZE * 5, {200, 170, 0, 255});
        }
    }
    else if (roomName == "Library") {
        // Tall bookshelves along walls
        for (int wall = 0; wall < 2; wall++) {
            for (int i = 0; i < 4; i++) {
                int shelfX = startX + (2 + wall * 14) * TILE_SIZE;
                int shelfY = startY + (1 + i * 3) * TILE_SIZE;
                
                // Wooden shelf frame
                for (int px = 0; px < TILE_SIZE * 3; px += 8) {
                    for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                        DrawRectangle(shelfX + px, shelfY + py, 8, 8, {112, 56, 16, 255});
                    }
                }
                
                // Books on shelves
                for (int j = 0; j < 12; j++) {
                    Color bookColor = (j % 4 == 0) ? Color{200, 50, 50, 255} : 
                                    (j % 4 == 1) ? Color{50, 200, 50, 255} : 
                                    (j % 4 == 2) ? Color{50, 50, 200, 255} :
                                                   Color{200, 200, 50, 255};
                    DrawRectangle(shelfX + 4 + j * 2, shelfY + 8, 4, 12, bookColor);
                }
            }
        }
        
        // Reading table in center
        int tableX = startX + 7 * TILE_SIZE;
        int tableY = startY + 6 * TILE_SIZE;
        for (int px = 0; px < TILE_SIZE * 4; px += 8) {
            for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                DrawRectangle(tableX + px, tableY + py, 8, 8, {139, 69, 19, 255});
            }
        }
        
        // Open books on table
        DrawRectangle(tableX + 8, tableY + 4, 16, 12, {255, 248, 220, 255}); // open book
        DrawRectangle(tableX + 24, tableY + 8, 12, 8, {200, 50, 50, 255}); // closed book
        
        // Candles for reading
        for (int i = 0; i < 3; i++) {
            int candleX = tableX + 4 + i * 16;
            int candleY = tableY - 8;
            DrawRectangle(candleX, candleY, 4, 12, {255, 248, 220, 255});
            DrawRectangle(candleX + 1, candleY - 4, 2, 4, {255, 100, 0, 255}); // flame
        }
        
        // Scattered scrolls on floor
        for (int i = 0; i < 5; i++) {
            int scrollX = startX + (3 + i * 3) * TILE_SIZE + (rand() % 8);
            int scrollY = startY + (10 + i % 2) * TILE_SIZE + (rand() % 8);
            DrawRectangle(scrollX, scrollY, 16, 4, {255, 248, 220, 255});
        }
        
        // Ladder to reach high shelves
        int ladderX = startX + 16 * TILE_SIZE;
        int ladderY = startY + 2 * TILE_SIZE;
        for (int py = 0; py < TILE_SIZE * 4; py += 8) {
            DrawRectangle(ladderX, ladderY + py, 4, 8, {139, 69, 19, 255});
            DrawRectangle(ladderX + 12, ladderY + py, 4, 8, {139, 69, 19, 255});
            if (py % 16 == 0) {
                DrawRectangle(ladderX, ladderY + py + 4, 16, 4, {139, 69, 19, 255}); // rungs
            }
        }
    }
    else if (roomName == "Monster Lair") {
        // Scattered bones and skulls
        for (int i = 0; i < 8; i++) {
            int boneX = startX + (2 + (i * 2) % 16) * TILE_SIZE + (i % 8);
            int boneY = startY + (2 + (i / 2) % 11) * TILE_SIZE + ((i * 3) % 8);
            
            // Bone pile
            DrawRectangle(boneX, boneY, 20, 8, {240, 240, 220, 255});
            DrawRectangle(boneX + 6, boneY - 8, 8, 16, {240, 240, 220, 255});
            
            // Add some skulls
            if (i % 3 == 0) {
                DrawRectangle(boneX + 16, boneY - 4, 12, 10, {240, 240, 220, 255});
                DrawRectangle(boneX + 18, boneY - 2, 2, 2, {0, 0, 0, 255}); // eye socket
                DrawRectangle(boneX + 24, boneY - 2, 2, 2, {0, 0, 0, 255}); // eye socket
            }
        }
        
        // Claw marks on walls
        for (int i = 0; i < 6; i++) {
            int clawX = startX + (1 + i * 3) * TILE_SIZE;
            int clawY = startY + (1 + i % 2) * TILE_SIZE;
            DrawRectangle(clawX, clawY, 4, 16, {64, 32, 32, 255});
            DrawRectangle(clawX + 8, clawY, 4, 16, {64, 32, 32, 255});
            DrawRectangle(clawX + 16, clawY, 4, 16, {64, 32, 32, 255});
        }
    }
    else if (roomName == "Dark Corridor") {
        // Moss on walls
        for (int i = 0; i < ROOM_GRID_HEIGHT - 2; i++) {
            int mossX = startX + 2 * TILE_SIZE;
            int mossY = startY + (1 + i) * TILE_SIZE;
            
            for (int py = 8; py < TILE_SIZE - 8; py += 8) {
                DrawRectangle(mossX, mossY + py, 8, 8, {0, 96, 48, 255});
                DrawRectangle(mossX + 8, mossY + py, 8, 8, {0, 64, 32, 255});
            }
            
            mossX = startX + (ROOM_GRID_WIDTH - 3) * TILE_SIZE;
            for (int py = 8; py < TILE_SIZE - 8; py += 8) {
                DrawRectangle(mossX, mossY + py, 8, 8, {0, 96, 48, 255});
                DrawRectangle(mossX + 8, mossY + py, 8, 8, {0, 64, 32, 255});
            }
        }
        
        // Puddles of water
        for (int i = 0; i < 4; i++) {
            int puddleX = startX + (4 + i * 4) * TILE_SIZE;
            int puddleY = startY + (8 + i % 2 * 3) * TILE_SIZE;
            DrawRectangle(puddleX, puddleY, 24, 16, {0, 0, 64, 180}); // Dark water
            DrawRectangle(puddleX + 4, puddleY + 4, 16, 8, {0, 0, 96, 120}); // Reflection
        }
        
        // Dim torches
        for (int i = 0; i < 3; i++) {
            int torchX = startX + (6 + i * 4) * TILE_SIZE;
            int torchY = startY + 2 * TILE_SIZE;
            DrawRectangle(torchX, torchY, 8, 24, {64, 32, 16, 255}); // Torch handle
            DrawRectangle(torchX + 2, torchY - 8, 4, 8, {128, 64, 0, 255}); // Dim flame
        }
    }
    else if (roomName == "Kitchen") {
        // Large wooden table
        int tableX = startX + 6 * TILE_SIZE;
        int tableY = startY + 5 * TILE_SIZE;
        for (int px = 0; px < TILE_SIZE * 3; px += 8) {
            for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                DrawRectangle(tableX + px, tableY + py, 8, 8, {128, 96, 64, 255});
            }
        }
        
        // Items on table
        DrawRectangle(tableX + 8, tableY + 4, 16, 4, {192, 192, 192, 255}); // knife
        DrawRectangle(tableX + 32, tableY + 8, 12, 8, {160, 82, 45, 255}); // pot
        DrawRectangle(tableX + 48, tableY + 6, 8, 6, {255, 215, 0, 255}); // gold items
        
        // Cooking pots and utensils
        for (int i = 0; i < 3; i++) {
            int potX = startX + (3 + i * 2) * TILE_SIZE;
            int potY = startY + 3 * TILE_SIZE;
            
            // Large cooking pot
            for (int px = 0; px < TILE_SIZE; px += 8) {
                for (int py = 0; py < TILE_SIZE; py += 8) {
                    DrawRectangle(potX + px, potY + py, 8, 8, {64, 64, 64, 255});
                }
            }
            
            // Pot handle
            DrawRectangle(potX - 4, potY + 8, 8, 4, {48, 48, 48, 255});
            DrawRectangle(potX + TILE_SIZE, potY + 8, 8, 4, {48, 48, 48, 255});
            
            // Steam/smoke
            if (i == 1) {
                for (int s = 0; s < 3; s++) {
                    DrawRectangle(potX + 8 + s * 4, potY - 8 - s * 4, 4, 4, {200, 200, 200, 100});
                }
            }
        }
        
        // Stone oven
        int ovenX = startX + 2 * TILE_SIZE;
        int ovenY = startY + 8 * TILE_SIZE;
        for (int px = 0; px < TILE_SIZE * 2; px += 8) {
            for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                DrawRectangle(ovenX + px, ovenY + py, 8, 8, {80, 80, 60, 255});
            }
        }
        
        // Oven opening
        DrawRectangle(ovenX + 8, ovenY + 16, 16, 8, {32, 16, 16, 255});
        DrawRectangle(ovenX + 12, ovenY + 12, 8, 4, {255, 100, 0, 255}); // fire
    }
    else if (roomName == "Basement") {
        // Wine barrels with metal bands
        for (int i = 0; i < 6; i++) {
            int barrelX = startX + (2 + i * 3) * TILE_SIZE;
            int barrelY = startY + (2 + (i % 2) * 5) * TILE_SIZE;
            
            // Barrel body
            for (int px = 0; px < TILE_SIZE; px += 8) {
                for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                    DrawRectangle(barrelX + px, barrelY + py, 8, 8, {96, 64, 32, 255});
                }
            }
            
            // Metal bands around barrel
            DrawRectangle(barrelX, barrelY + 8, TILE_SIZE, 4, {64, 64, 64, 255});
            DrawRectangle(barrelX, barrelY + 32, TILE_SIZE, 4, {64, 64, 64, 255});
            
            // Spigot/tap
            if (i % 2 == 0) {
                DrawRectangle(barrelX + TILE_SIZE, barrelY + 16, 8, 4, {128, 128, 128, 255});
                DrawRectangle(barrelX + TILE_SIZE + 8, barrelY + 18, 4, 2, {64, 64, 64, 255});
            }
        }
        
        // Wooden crates
        for (int i = 0; i < 4; i++) {
            int crateX = startX + (15 + (i % 2) * 3) * TILE_SIZE;
            int crateY = startY + (3 + (i / 2) * 4) * TILE_SIZE;
            
            // Crate body
            for (int px = 0; px < TILE_SIZE * 2; px += 8) {
                for (int py = 0; py < TILE_SIZE; py += 8) {
                    DrawRectangle(crateX + px, crateY + py, 8, 8, {112, 80, 48, 255});
                }
            }
            
            // Crate slats
            DrawRectangle(crateX + 8, crateY, 4, TILE_SIZE, {80, 56, 32, 255});
            DrawRectangle(crateX + 24, crateY, 4, TILE_SIZE, {80, 56, 32, 255});
        }
        
        // Wine bottles on shelves
        int shelfX = startX + 17 * TILE_SIZE;
        int shelfY = startY + 1 * TILE_SIZE;
        DrawRectangle(shelfX, shelfY, TILE_SIZE * 3, 8, {112, 80, 48, 255}); // shelf
        
        for (int i = 0; i < 8; i++) {
            int bottleX = shelfX + 4 + i * 6;
            DrawRectangle(bottleX, shelfY - 16, 4, 16, {0, 64, 0, 255}); // green bottles
            DrawRectangle(bottleX, shelfY - 20, 4, 4, {64, 32, 16, 255}); // cork
        }
    }
    else if (roomName == "Throne Room") {
        int throneX = startX + 8 * TILE_SIZE;
        int throneY = startY + 3 * TILE_SIZE;
        for (int px = 0; px < TILE_SIZE * 3; px += 8) {
            for (int py = 0; py < TILE_SIZE * 4; py += 8) {
                DrawRectangle(throneX + px, throneY + py, 8, 8, {96, 64, 128, 255});
            }
        }
        
        for (int px = 8; px < TILE_SIZE * 3 - 8; px += 8) {
            for (int py = 8; py < TILE_SIZE * 2; py += 8) {
                DrawRectangle(throneX + px, throneY + py, 8, 8, {144, 112, 160, 255});
            }
        }
    }
    else if (roomName == "Garden") {
        // Draw flowers
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 4; j++) {
                int flowerX = startX + (3 + i * 2) * TILE_SIZE + (rand() % 16);
                int flowerY = startY + (3 + j * 2) * TILE_SIZE + (rand() % 16);
                Color flowerColor = (rand() % 3 == 0) ? Color{255, 64, 64, 255} :
                                  (rand() % 3 == 1) ? Color{64, 64, 255, 255} :
                                                     Color{255, 255, 64, 255};
                DrawRectangle(flowerX, flowerY, 8, 8, flowerColor);
                DrawRectangle(flowerX, flowerY + 8, 8, 8, {0, 128, 0, 255});
            }
        }
        
        // Draw trapdoor in the center-bottom of the garden
        int trapdoorX = startX + 10 * TILE_SIZE;
        int trapdoorY = startY + 12 * TILE_SIZE;
        
        // Wooden trapdoor frame
        for (int px = 0; px < TILE_SIZE * 3; px += 8) {
            for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                DrawRectangle(trapdoorX + px, trapdoorY + py, 8, 8, {101, 67, 33, 255}); // Brown wood
            }
        }
        DrawRectangleLines(trapdoorX, trapdoorY, TILE_SIZE * 3, TILE_SIZE * 2, {80, 50, 25, 255});
        
        // Metal hinges
        DrawRectangle(trapdoorX + 4, trapdoorY + 4, 8, 12, {120, 120, 120, 255});
        DrawRectangle(trapdoorX + TILE_SIZE * 3 - 12, trapdoorY + 4, 8, 12, {120, 120, 120, 255});
        
        // Iron ring handle
        DrawRectangle(trapdoorX + TILE_SIZE + 8, trapdoorY + TILE_SIZE - 4, 16, 8, {120, 120, 120, 255});
        DrawRectangleLines(trapdoorX + TILE_SIZE + 8, trapdoorY + TILE_SIZE - 4, 16, 8, {80, 80, 80, 255});
        
        // Wood grain details
        for (int i = 0; i < 3; i++) {
            DrawRectangle(trapdoorX + 8 + i * 24, trapdoorY + 8, 2, TILE_SIZE * 2 - 16, {80, 50, 25, 255});
        }
        
        // Slightly ajar opening showing darkness below
        DrawRectangle(trapdoorX + TILE_SIZE * 2, trapdoorY + TILE_SIZE, TILE_SIZE - 8, 12, {20, 20, 20, 255});
        DrawRectangleLines(trapdoorX + TILE_SIZE * 2, trapdoorY + TILE_SIZE, TILE_SIZE - 8, 12, {40, 40, 40, 255});
    }
    else if (roomName == "Dark Room") {
        // Draw glowing crystals on walls
        for (int i = 0; i < 8; i++) {
            int crystalX, crystalY;
            Color crystalColor;
            
            // Alternate between different colored crystals
            switch (i % 3) {
                case 0: crystalColor = {100, 200, 255, 255}; break; // Blue
                case 1: crystalColor = {200, 100, 255, 255}; break; // Purple
                case 2: crystalColor = {255, 200, 100, 255}; break; // Orange
            }
            
            if (i < 4) {
                // Left wall crystals
                crystalX = startX + 2 * TILE_SIZE;
                crystalY = startY + (3 + i * 3) * TILE_SIZE;
            } else {
                // Right wall crystals
                crystalX = startX + (ROOM_GRID_WIDTH - 3) * TILE_SIZE;
                crystalY = startY + (3 + (i - 4) * 3) * TILE_SIZE;
            }
            
            // Crystal formation
            DrawRectangle(crystalX, crystalY, 16, 24, crystalColor);
            DrawRectangle(crystalX + 4, crystalY - 8, 8, 16, crystalColor);
            DrawRectangle(crystalX + 8, crystalY - 12, 4, 8, crystalColor);
            
            // Crystal outline for definition
            DrawRectangleLines(crystalX, crystalY, 16, 24, {crystalColor.r - 50, crystalColor.g - 50, crystalColor.b - 50, 255});
            
            // Glowing effect
            for (int glow = 0; glow < 3; glow++) {
                Color glowColor = {crystalColor.r, crystalColor.g, crystalColor.b, (unsigned char)(50 - glow * 15)};
                DrawRectangleLines(crystalX - glow, crystalY - glow, 16 + glow * 2, 24 + glow * 2, glowColor);
            }
        }
        
        // Mysterious altar/pedestal in center
        int altarX = startX + 10 * TILE_SIZE;
        int altarY = startY + 8 * TILE_SIZE;
        
        // Stone pedestal
        for (int px = 0; px < TILE_SIZE * 2; px += 8) {
            for (int py = 0; py < TILE_SIZE; py += 8) {
                DrawRectangle(altarX + px, altarY + py, 8, 8, {60, 60, 80, 255}); // Dark stone
            }
        }
        DrawRectangleLines(altarX, altarY, TILE_SIZE * 2, TILE_SIZE, {40, 40, 60, 255});
        
        // Mystical orb on pedestal
        DrawRectangle(altarX + 20, altarY - 8, 24, 24, {150, 50, 200, 255}); // Purple orb
        DrawRectangleLines(altarX + 20, altarY - 8, 24, 24, {100, 30, 150, 255});
        
        // Orb glow effect
        for (int glow = 0; glow < 4; glow++) {
            Color glowColor = {150, 50, 200, (unsigned char)(30 - glow * 7)};
            DrawRectangleLines(altarX + 20 - glow, altarY - 8 - glow, 24 + glow * 2, 24 + glow * 2, glowColor);
        }
        
        // Mysterious stranger (only if not met yet)
        if (!strangeMet) {
            int strangerX = startX + 6 * TILE_SIZE;
            int strangerY = startY + 10 * TILE_SIZE;
            
            // Hooded cloak - dark robes
            DrawRectangle(strangerX, strangerY, 32, 48, {40, 20, 60, 255}); // Dark purple cloak
            DrawRectangle(strangerX + 4, strangerY - 8, 24, 16, {40, 20, 60, 255}); // Hood
            
            // Cloak details
            DrawRectangleLines(strangerX, strangerY, 32, 48, {20, 10, 30, 255});
            DrawRectangle(strangerX + 14, strangerY + 8, 4, 32, {60, 30, 80, 255}); // Cloak seam
            
            // Glowing eyes under hood
            DrawRectangle(strangerX + 8, strangerY - 4, 4, 4, {255, 100, 100, 255}); // Red glowing left eye
            DrawRectangle(strangerX + 20, strangerY - 4, 4, 4, {255, 100, 100, 255}); // Red glowing right eye
            
            // Eye glow effect
            for (int glow = 0; glow < 3; glow++) {
                Color eyeGlow = {255, 100, 100, (unsigned char)(60 - glow * 20)};
                DrawRectangleLines(strangerX + 8 - glow, strangerY - 4 - glow, 4 + glow * 2, 4 + glow * 2, eyeGlow);
                DrawRectangleLines(strangerX + 20 - glow, strangerY - 4 - glow, 4 + glow * 2, 4 + glow * 2, eyeGlow);
            }
            
            // Skeletal hands extending from cloak
            DrawRectangle(strangerX - 8, strangerY + 20, 12, 20, {200, 200, 180, 255}); // Left arm
            DrawRectangle(strangerX + 28, strangerY + 20, 12, 20, {200, 200, 180, 255}); // Right arm
            
            // Bony fingers
            for (int finger = 0; finger < 4; finger++) {
                DrawRectangle(strangerX - 12 + finger * 3, strangerY + 38, 2, 8, {200, 200, 180, 255});
                DrawRectangle(strangerX + 32 + finger * 3, strangerY + 38, 2, 8, {200, 200, 180, 255});
            }
            
            // Dark aura around stranger
            for (int aura = 0; aura < 5; aura++) {
                Color auraColor = {60, 20, 80, (unsigned char)(25 - aura * 5)};
                DrawRectangleLines(strangerX - 4 - aura * 2, strangerY - 12 - aura * 2, 
                                 40 + aura * 4, 64 + aura * 4, auraColor);
            }
        }
    }
    else if (roomName == "Infirmary") {
        // Medical shelves on left and right walls
        for (int i = 0; i < 3; i++) {
            // Left wall shelves
            int leftShelfX = startX + 2 * TILE_SIZE;
            int leftShelfY = startY + (2 + i * 4) * TILE_SIZE;
            for (int px = 0; px < TILE_SIZE * 2; px += 8) {
                for (int py = 0; py < TILE_SIZE; py += 8) {
                    DrawRectangle(leftShelfX + px, leftShelfY + py, 8, 8, {139, 115, 85, 255}); // Brown wood
                }
            }
            DrawRectangleLines(leftShelfX, leftShelfY, TILE_SIZE * 2, TILE_SIZE, {100, 80, 60, 255});
            
            // Right wall shelves
            int rightShelfX = startX + (ROOM_GRID_WIDTH - 4) * TILE_SIZE;
            int rightShelfY = startY + (2 + i * 4) * TILE_SIZE;
            for (int px = 0; px < TILE_SIZE * 2; px += 8) {
                for (int py = 0; py < TILE_SIZE; py += 8) {
                    DrawRectangle(rightShelfX + px, rightShelfY + py, 8, 8, {139, 115, 85, 255});
                }
            }
            DrawRectangleLines(rightShelfX, rightShelfY, TILE_SIZE * 2, TILE_SIZE, {100, 80, 60, 255});
            
            // Bottles on shelves
            for (int j = 0; j < 4; j++) {
                // Left shelf bottles
                Color bottleColor = (j % 2 == 0) ? Color{0, 150, 255, 255} : Color{255, 100, 100, 255}; // Blue and red potions
                DrawRectangle(leftShelfX + 8 + j * 12, leftShelfY - 8, 8, 12, bottleColor);
                DrawRectangleLines(leftShelfX + 8 + j * 12, leftShelfY - 8, 8, 12, {200, 200, 200, 255});
                
                // Right shelf bottles
                DrawRectangle(rightShelfX + 8 + j * 12, rightShelfY - 8, 8, 12, bottleColor);
                DrawRectangleLines(rightShelfX + 8 + j * 12, rightShelfY - 8, 8, 12, {200, 200, 200, 255});
            }
        }
        
        // Medical cot in the corner
        int cotX = startX + (ROOM_GRID_WIDTH - 6) * TILE_SIZE;
        int cotY = startY + (ROOM_GRID_HEIGHT - 4) * TILE_SIZE;
        
        // Wooden frame
        for (int px = 0; px < TILE_SIZE * 4; px += 8) {
            for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                DrawRectangle(cotX + px, cotY + py, 8, 8, {139, 115, 85, 255});
            }
        }
        DrawRectangleLines(cotX, cotY, TILE_SIZE * 4, TILE_SIZE * 2, {100, 80, 60, 255});
        
        // White sheet/mattress
        for (int px = 4; px < TILE_SIZE * 4 - 4; px += 8) {
            for (int py = 4; py < TILE_SIZE * 2 - 4; py += 8) {
                DrawRectangle(cotX + px, cotY + py, 8, 8, {240, 240, 240, 255});
            }
        }
        
        // Hanging herbs from ceiling
        for (int i = 0; i < 4; i++) {
            int herbX = startX + (4 + i * 3) * TILE_SIZE + 8;
            int herbY = startY + 2 * TILE_SIZE;
            
            // String/rope
            DrawRectangle(herbX + 6, herbY, 2, 16, {139, 115, 85, 255});
            
            // Herb bundle
            DrawRectangle(herbX, herbY + 16, 16, 12, {0, 128, 0, 255}); // Green herbs
            DrawRectangleLines(herbX, herbY + 16, 16, 12, {0, 100, 0, 255});
            
            // Small herb details
            for (int j = 0; j < 3; j++) {
                DrawRectangle(herbX + 2 + j * 4, herbY + 14, 4, 4, {50, 150, 50, 255});
            }
        }
        
        // Medicine table in center
        int tableX = startX + 10 * TILE_SIZE;
        int tableY = startY + 8 * TILE_SIZE;
        
        // Table surface
        for (int px = 0; px < TILE_SIZE * 3; px += 8) {
            for (int py = 0; py < TILE_SIZE * 2; py += 8) {
                DrawRectangle(tableX + px, tableY + py, 8, 8, {160, 130, 100, 255});
            }
        }
        DrawRectangleLines(tableX, tableY, TILE_SIZE * 3, TILE_SIZE * 2, {120, 90, 70, 255});
        
        // Items on table
        DrawRectangle(tableX + 8, tableY + 4, 12, 8, {200, 200, 200, 255}); // Bandages
        DrawRectangle(tableX + 24, tableY + 8, 8, 12, {150, 75, 0, 255}); // Medicine bottle
        DrawRectangle(tableX + 36, tableY + 6, 16, 6, {255, 255, 200, 255}); // Plaster/healing cloth
    } else if (roomName == "Sunlit Meadow") {
        // Dungeon exit at the top center of the room
        int exitX = startX + (TILE_SIZE * 11);
        int exitY = startY + (TILE_SIZE * 1);
        DrawRectangle(exitX, exitY, TILE_SIZE * 2, 20, {139, 69, 19, 255}); // Brown wooden door
        DrawRectangle(exitX + 8, exitY + 6, 8, 8, {160, 82, 45, 255}); // Door handle
        DrawRectangleLines(exitX, exitY, TILE_SIZE * 2, 20, {101, 67, 33, 255}); // Door outline
        
        // Flowers with centers and simple stems
        for (int i = 0; i < 8; i++) {
            int flowerX = startX + (TILE_SIZE * (3 + (i * 2) % 18));
            int flowerY = startY + (TILE_SIZE * (4 + (i * 3) % 10));
            
            Color flowerColors[] = {
                {255, 100, 100, 255}, // Red
                {255, 255, 100, 255}, // Yellow
                {100, 150, 255, 255}, // Blue
                {255, 150, 200, 255}  // Pink
            };
            Color flowerColor = flowerColors[i % 4];
            
            // Flower head with center
            DrawRectangle(flowerX, flowerY, 16, 16, flowerColor);
            DrawRectangle(flowerX + 4, flowerY + 4, 8, 8, {255, 255, 0, 255}); // Yellow center
            DrawRectangleLines(flowerX, flowerY, 16, 16, {200, 200, 200, 255});
            
            // Simple stem
            DrawRectangle(flowerX + 6, flowerY + 16, 4, 12, {0, 128, 0, 255});
        }
        
        // Trees with better proportions
        for (int i = 0; i < 4; i++) {
            int treeX = startX + (TILE_SIZE * (2 + i * 5));
            int treeY = startY + (TILE_SIZE * 12);
            
            // Tree trunk
            DrawRectangle(treeX + 8, treeY + 16, 16, TILE_SIZE * 2, {101, 67, 33, 255});
            DrawRectangleLines(treeX + 8, treeY + 16, 16, TILE_SIZE * 2, {80, 50, 20, 255});
            
            // Tree leaves - larger crown
            DrawRectangle(treeX, treeY, TILE_SIZE, 24, {34, 139, 34, 255});
            DrawRectangle(treeX + 4, treeY - 8, 24, 16, {50, 205, 50, 255}); // Lighter top
            DrawRectangleLines(treeX, treeY, TILE_SIZE, 24, {0, 100, 0, 255});
        }
        
        // Grass patches
        for (int i = 0; i < 12; i++) {
            int grassX = startX + (TILE_SIZE * (1 + (i * 2) % 22));
            int grassY = startY + (TILE_SIZE * (6 + (i * 2) % 8));
            
            // Small grass clumps
            DrawRectangle(grassX, grassY, 8, 12, {50, 150, 50, 255});
            DrawRectangle(grassX + 8, grassY + 2, 6, 8, {60, 160, 60, 255});
        }
        
        // Rocks with some variety
        for (int i = 0; i < 4; i++) {
            int rockX = startX + (TILE_SIZE * (6 + i * 3));
            int rockY = startY + (TILE_SIZE * (8 + i % 3));
            
            // Main rock
            DrawRectangle(rockX, rockY, 20, 16, {128, 128, 128, 255});
            DrawRectangle(rockX + 4, rockY - 6, 12, 8, {160, 160, 160, 255}); // Lighter top
            DrawRectangleLines(rockX, rockY, 20, 16, {100, 100, 100, 255});
        }
        
        // Path to exit
        for (int i = 0; i < 4; i++) {
            int pathX = startX + (TILE_SIZE * (11 + (i % 2) * 1));
            int pathY = startY + (TILE_SIZE * (3 + i));
            
            DrawRectangle(pathX, pathY, 16, 16, {192, 192, 192, 255});
            DrawRectangleLines(pathX, pathY, 16, 16, {128, 128, 128, 255});
        }
    } else if (roomName == "Chapel") {
        // Stone altar at the front center
        int altarX = startX + 8 * TILE_SIZE;
        int altarY = startY + 2 * TILE_SIZE;
        DrawRectangle(altarX, altarY, TILE_SIZE * 4, TILE_SIZE * 2, {160, 160, 160, 255});
        DrawRectangleLines(altarX, altarY, TILE_SIZE * 4, TILE_SIZE * 2, {120, 120, 120, 255});
        
        // Cross on altar
        DrawRectangle(altarX + 56, altarY - 16, 8, 24, {255, 215, 0, 255}); // Vertical
        DrawRectangle(altarX + 48, altarY - 12, 24, 8, {255, 215, 0, 255}); // Horizontal
        
        // Wooden pews (benches)
        for (int i = 0; i < 3; i++) {
            int pewX = startX + (4 + i * 5) * TILE_SIZE;
            int pewY = startY + 8 * TILE_SIZE;
            DrawRectangle(pewX, pewY, TILE_SIZE * 3, TILE_SIZE, {139, 69, 19, 255});
            DrawRectangleLines(pewX, pewY, TILE_SIZE * 3, TILE_SIZE, {101, 67, 33, 255});
            
            // Pew backs
            DrawRectangle(pewX, pewY - 12, TILE_SIZE * 3, 12, {139, 69, 19, 255});
            DrawRectangleLines(pewX, pewY - 12, TILE_SIZE * 3, 12, {101, 67, 33, 255});
        }
        
        // Candle stands
        for (int i = 0; i < 4; i++) {
            int candleX = startX + (3 + i * 4) * TILE_SIZE;
            int candleY = startY + 12 * TILE_SIZE;
            DrawRectangle(candleX, candleY, 8, 20, {255, 215, 0, 255});
            DrawRectangleLines(candleX, candleY, 8, 20, {200, 170, 0, 255});
            // Flame
            DrawRectangle(candleX + 2, candleY - 8, 4, 8, {255, 150, 0, 255});
        }
    } else if (roomName == "Sleeping Quarters") {
        // Beds along the walls
        for (int i = 0; i < 4; i++) {
            int bedX = startX + (2 + (i % 2) * 10) * TILE_SIZE;
            int bedY = startY + (3 + (i / 2) * 6) * TILE_SIZE;
            
            // Bed frame
            DrawRectangle(bedX, bedY, TILE_SIZE * 4, TILE_SIZE * 2, {139, 69, 19, 255});
            DrawRectangleLines(bedX, bedY, TILE_SIZE * 4, TILE_SIZE * 2, {101, 67, 33, 255});
            
            // Mattress
            DrawRectangle(bedX + 4, bedY + 4, TILE_SIZE * 4 - 8, TILE_SIZE * 2 - 8, {255, 248, 220, 255});
            
            // Pillow
            DrawRectangle(bedX + 8, bedY + 8, 24, 16, {200, 200, 255, 255});
            DrawRectangleLines(bedX + 8, bedY + 8, 24, 16, {150, 150, 200, 255});
        }
        
        // Personal belongings (chests)
        for (int i = 0; i < 4; i++) {
            int chestX = startX + (3 + (i % 2) * 10) * TILE_SIZE;
            int chestY = startY + (6 + (i / 2) * 6) * TILE_SIZE;
            
            DrawRectangle(chestX, chestY, TILE_SIZE, 16, {160, 82, 45, 255});
            DrawRectangleLines(chestX, chestY, TILE_SIZE, 16, {120, 60, 30, 255});
            
            // Lock
            DrawRectangle(chestX + 12, chestY + 6, 8, 6, {255, 215, 0, 255});
        }
        
        // Hanging lanterns
        for (int i = 0; i < 2; i++) {
            int lanternX = startX + (6 + i * 8) * TILE_SIZE;
            int lanternY = startY + TILE_SIZE;
            
            DrawRectangle(lanternX, lanternY, 16, 20, {255, 215, 0, 255});
            DrawRectangleLines(lanternX, lanternY, 16, 20, {200, 170, 0, 255});
            
            // Light glow
            DrawRectangle(lanternX + 4, lanternY + 4, 8, 12, {255, 255, 150, 180});
        }
    }
    
    // Draw monsters
    const auto& monsters = room->GetMonsters();
    for (size_t i = 0; i < monsters.size(); i++) {
        if (monsters[i].alive) {
            int monsterX = startX + (int)(monsters[i].x * TILE_SIZE);
            int monsterY = startY + (int)(monsters[i].y * TILE_SIZE);
            
            if (monsters[i].name == "goblin") {
                // Goblin head - green skin
                DrawRectangle(monsterX, monsterY, 16, 12, {34, 139, 34, 255});
                
                // Large pointed ears
                DrawRectangle(monsterX - 4, monsterY + 2, 4, 8, {34, 139, 34, 255});
                DrawRectangle(monsterX + 16, monsterY + 2, 4, 8, {34, 139, 34, 255});
                
                // Red glowing eyes
                DrawRectangle(monsterX + 2, monsterY + 3, 4, 4, {255, 0, 0, 255});
                DrawRectangle(monsterX + 10, monsterY + 3, 4, 4, {255, 0, 0, 255});
                
                // Snarling mouth with teeth
                DrawRectangle(monsterX + 6, monsterY + 8, 4, 2, {139, 0, 0, 255});
                DrawRectangle(monsterX + 4, monsterY + 9, 2, 2, {255, 255, 255, 255}); // fangs
                DrawRectangle(monsterX + 10, monsterY + 9, 2, 2, {255, 255, 255, 255});
                
                // Hunched body
                DrawRectangle(monsterX + 2, monsterY + 12, 12, 16, {34, 139, 34, 255});
                
                // Arms with claws
                DrawRectangle(monsterX - 2, monsterY + 14, 6, 10, {34, 139, 34, 255});
                DrawRectangle(monsterX + 12, monsterY + 14, 6, 10, {34, 139, 34, 255});
                DrawRectangle(monsterX - 4, monsterY + 22, 4, 2, {255, 255, 255, 255}); // claws
                DrawRectangle(monsterX + 16, monsterY + 22, 4, 2, {255, 255, 255, 255});
                
                // Legs
                DrawRectangle(monsterX + 2, monsterY + 28, 4, 8, {34, 139, 34, 255});
                DrawRectangle(monsterX + 10, monsterY + 28, 4, 8, {34, 139, 34, 255});
                
                // Crude loincloth
                DrawRectangle(monsterX + 4, monsterY + 24, 8, 6, {139, 69, 19, 255});
            } 
            else if (monsters[i].name == "skeleton") {
                // Skull
                DrawRectangle(monsterX, monsterY, 16, 12, {245, 245, 220, 255});
                
                // Large dark eye sockets
                DrawRectangle(monsterX + 2, monsterY + 2, 4, 6, {0, 0, 0, 255});
                DrawRectangle(monsterX + 10, monsterY + 2, 4, 6, {0, 0, 0, 255});
                
                // Nasal cavity
                DrawRectangle(monsterX + 7, monsterY + 6, 2, 4, {0, 0, 0, 255});
                
                // Jaw with teeth
                DrawRectangle(monsterX + 2, monsterY + 10, 12, 4, {245, 245, 220, 255});
                for (int t = 0; t < 4; t++) {
                    DrawRectangle(monsterX + 4 + t * 2, monsterY + 12, 1, 2, {255, 255, 255, 255});
                }
                
                // Spine and ribcage
                DrawRectangle(monsterX + 6, monsterY + 14, 4, 16, {245, 245, 220, 255});
                for (int r = 0; r < 3; r++) {
                    DrawRectangle(monsterX + 2, monsterY + 16 + r * 4, 12, 2, {245, 245, 220, 255});
                }
                
                // Bone arms
                DrawRectangle(monsterX - 2, monsterY + 16, 6, 4, {245, 245, 220, 255});
                DrawRectangle(monsterX + 12, monsterY + 16, 6, 4, {245, 245, 220, 255});
                DrawRectangle(monsterX - 4, monsterY + 20, 4, 8, {245, 245, 220, 255});
                DrawRectangle(monsterX + 16, monsterY + 20, 4, 8, {245, 245, 220, 255});
                
                // Bone legs
                DrawRectangle(monsterX + 2, monsterY + 30, 4, 12, {245, 245, 220, 255});
                DrawRectangle(monsterX + 10, monsterY + 30, 4, 12, {245, 245, 220, 255});
                
                // Joints
                DrawRectangle(monsterX + 1, monsterY + 36, 6, 2, {245, 245, 220, 255}); // feet
                DrawRectangle(monsterX + 9, monsterY + 36, 6, 2, {245, 245, 220, 255});
            } 
            else if (monsters[i].name == "rat") {
                // Rat head with snout
                DrawRectangle(monsterX, monsterY + 2, 12, 8, {101, 67, 33, 255});
                DrawRectangle(monsterX + 12, monsterY + 4, 6, 4, {101, 67, 33, 255}); // snout
                
                // Beady red eyes
                DrawRectangle(monsterX + 2, monsterY + 3, 2, 2, {255, 0, 0, 255});
                DrawRectangle(monsterX + 8, monsterY + 3, 2, 2, {255, 0, 0, 255});
                
                // Large front teeth
                DrawRectangle(monsterX + 14, monsterY + 6, 2, 3, {255, 255, 255, 255});
                DrawRectangle(monsterX + 16, monsterY + 6, 2, 3, {255, 255, 255, 255});
                
                // Large ears
                DrawRectangle(monsterX - 2, monsterY, 4, 6, {101, 67, 33, 255});
                DrawRectangle(monsterX + 12, monsterY, 4, 6, {101, 67, 33, 255});
                
                // Fat body
                DrawRectangle(monsterX - 2, monsterY + 10, 20, 12, {101, 67, 33, 255});
                
                // Four legs
                DrawRectangle(monsterX + 2, monsterY + 22, 3, 6, {101, 67, 33, 255});
                DrawRectangle(monsterX + 7, monsterY + 22, 3, 6, {101, 67, 33, 255});
                DrawRectangle(monsterX + 12, monsterY + 22, 3, 6, {101, 67, 33, 255});
                DrawRectangle(monsterX + 17, monsterY + 22, 3, 6, {101, 67, 33, 255});
                
                // Long hairless tail
                DrawRectangle(monsterX + 18, monsterY + 14, 16, 2, {160, 82, 45, 255});
                DrawRectangle(monsterX + 34, monsterY + 16, 8, 2, {160, 82, 45, 255});
            } 
            else if (monsters[i].name == "ghost") {
                // Ghostly head - translucent
                DrawRectangle(monsterX, monsterY, 16, 12, {200, 200, 255, 180});
                
                // Hollow glowing eyes
                DrawRectangle(monsterX + 3, monsterY + 3, 3, 4, {100, 100, 255, 255});
                DrawRectangle(monsterX + 10, monsterY + 3, 3, 4, {100, 100, 255, 255});
                
                // Dark mouth opening
                DrawRectangle(monsterX + 6, monsterY + 8, 4, 3, {50, 50, 150, 200});
                
                // Flowing ghostly body
                DrawRectangle(monsterX - 2, monsterY + 12, 20, 16, {200, 200, 255, 160});
                
                // Wispy tendrils instead of legs
                for (int t = 0; t < 4; t++) {
                    DrawRectangle(monsterX + 2 + t * 3, monsterY + 28, 2, 8, {200, 200, 255, 120});
                    DrawRectangle(monsterX + 1 + t * 3, monsterY + 36, 2, 4, {200, 200, 255, 80});
                }
                
                // Floating arms
                DrawRectangle(monsterX - 4, monsterY + 14, 6, 8, {200, 200, 255, 140});
                DrawRectangle(monsterX + 14, monsterY + 14, 6, 8, {200, 200, 255, 140});
                
                // Ethereal glow effect
                DrawRectangle(monsterX - 6, monsterY - 2, 28, 44, {150, 150, 255, 30});
            }
            else if (monsters[i].name == "guardian spirit") {
                // Guardian spirit - translucent holy figure
                // Hooded head
                DrawRectangle(monsterX, monsterY, 16, 12, {255, 255, 255, 180});
                DrawRectangle(monsterX + 2, monsterY - 4, 12, 8, {200, 200, 255, 180}); // Hood
                
                // Glowing eyes
                DrawRectangle(monsterX + 4, monsterY + 3, 2, 4, {255, 255, 0, 255});
                DrawRectangle(monsterX + 10, monsterY + 3, 2, 4, {255, 255, 0, 255});
                
                // Robed body
                DrawRectangle(monsterX - 2, monsterY + 12, 20, 20, {240, 240, 255, 180});
                
                // Arms in prayer position
                DrawRectangle(monsterX + 2, monsterY + 14, 4, 12, {255, 255, 255, 180});
                DrawRectangle(monsterX + 10, monsterY + 14, 4, 12, {255, 255, 255, 180});
                
                // Holy aura effect
                DrawRectangle(monsterX - 4, monsterY - 2, 24, 36, {255, 255, 200, 40});
            }
            else if (monsters[i].name == "nightmare wraith") {
                // Nightmare wraith - dark shadowy creature
                // Dark smoky head
                DrawRectangle(monsterX, monsterY, 16, 12, {50, 20, 80, 200});
                DrawRectangle(monsterX - 2, monsterY + 2, 20, 8, {30, 10, 60, 150}); // Wispy edges
                
                // Red glowing eyes
                DrawRectangle(monsterX + 3, monsterY + 3, 3, 4, {255, 0, 0, 255});
                DrawRectangle(monsterX + 10, monsterY + 3, 3, 4, {255, 0, 0, 255});
                
                // Dark writhing body
                DrawRectangle(monsterX + 1, monsterY + 12, 14, 18, {40, 20, 70, 200});
                DrawRectangle(monsterX - 1, monsterY + 16, 18, 12, {30, 10, 50, 150}); // Shadowy tendrils
                
                // Clawed arms
                DrawRectangle(monsterX - 3, monsterY + 14, 6, 10, {50, 20, 80, 180});
                DrawRectangle(monsterX + 13, monsterY + 14, 6, 10, {50, 20, 80, 180});
                
                // Dark aura effect
                DrawRectangle(monsterX - 6, monsterY - 2, 28, 36, {80, 0, 100, 60});
            }
            
            // Draw health bar above monster
            int maxHealth = 0;
            if (monsters[i].name == "goblin") maxHealth = 15;
            else if (monsters[i].name == "skeleton") maxHealth = 20;
            else if (monsters[i].name == "rat") maxHealth = 8;
            else if (monsters[i].name == "ghost") maxHealth = 25;
            else if (monsters[i].name == "guardian spirit") maxHealth = 35;
            else if (monsters[i].name == "nightmare wraith") maxHealth = 30;
            
            if (maxHealth > 0) {
                float healthPercent = (float)monsters[i].health / (float)maxHealth;
                
                // Health bar background
                DrawRectangle(monsterX - 4, monsterY - 12, 24, 6, {100, 100, 100, 255});
                
                // Health bar foreground
                Color healthColor = {255, 100, 100, 255}; // Red
                if (healthPercent > 0.6f) healthColor = {100, 255, 100, 255}; // Green
                else if (healthPercent > 0.3f) healthColor = {255, 255, 100, 255}; // Yellow
                
                int healthWidth = (int)(22 * healthPercent);
                DrawRectangle(monsterX - 3, monsterY - 11, healthWidth, 4, healthColor);
                
                // Health text
                std::string healthText = std::to_string(monsters[i].health) + "/" + std::to_string(maxHealth);
                DrawText(healthText.c_str(), monsterX - 8, monsterY - 24, 12, {255, 255, 255, 255});
            }
        }
    }
}

void TextAdventure::DrawPlayer() {
    int startX = 40;
    int startY = 80;
    
    int playerPixelX = startX + (int)(playerRoomX * TILE_SIZE);
    int playerPixelY = startY + (int)(playerRoomY * TILE_SIZE);
    
    // Body bobbing animation (4 distinct poses)
    int bodyBob = 0;
    if (isWalking) {
        switch (walkAnimFrame) {
            case 0: bodyBob = 0; break;   // Standing
            case 1: bodyBob = -3; break; // Down during step
            case 2: bodyBob = 0; break;   // Center
            case 3: bodyBob = -3; break; // Down during step
        }
    }
    
    // Player head - flesh tone with bobbing
    DrawRectangle(playerPixelX - 8, playerPixelY - 16 + bodyBob, 16, 8, {255, 220, 177, 255});
    
    // Hair - different styles for male/female (with bobbing)
    if (isFemale) {
        // Longer hair for female
        DrawRectangle(playerPixelX - 8, playerPixelY - 24 + bodyBob, 16, 12, {218, 165, 32, 255}); // blonde
        DrawRectangle(playerPixelX - 10, playerPixelY - 18 + bodyBob, 4, 8, {218, 165, 32, 255}); // side hair
        DrawRectangle(playerPixelX + 14, playerPixelY - 18 + bodyBob, 4, 8, {218, 165, 32, 255}); // side hair
    } else {
        // Short hair for male
        DrawRectangle(playerPixelX - 8, playerPixelY - 24 + bodyBob, 16, 8, {139, 69, 19, 255}); // brown
    }
    
    // Eyes - small black pixels (with bobbing)
    DrawRectangle(playerPixelX - 6, playerPixelY - 14 + bodyBob, 2, 2, {0, 0, 0, 255});
    DrawRectangle(playerPixelX + 4, playerPixelY - 14 + bodyBob, 2, 2, {0, 0, 0, 255});
    
    // Nose - small flesh pixel (with bobbing)
    DrawRectangle(playerPixelX - 1, playerPixelY - 12 + bodyBob, 2, 2, {220, 180, 140, 255});
    
    // Different clothing for male/female (with bobbing)
    if (isFemale) {
        // Purple dress for female
        DrawRectangle(playerPixelX - 8, playerPixelY - 8 + bodyBob, 16, 20, {128, 0, 128, 255});
        DrawRectangle(playerPixelX - 10, playerPixelY + 4 + bodyBob, 20, 8, {128, 0, 128, 255}); // dress flare
    } else {
        // Blue shirt for male
        DrawRectangle(playerPixelX - 8, playerPixelY - 8 + bodyBob, 16, 16, {0, 100, 200, 255});
        // Pants - dark blue
        DrawRectangle(playerPixelX - 8, playerPixelY + 8 + bodyBob, 16, 8, {0, 50, 100, 255});
    }
    
    // Arms - flesh tone with animation (very pronounced, 4 distinct poses)
    int leftArmOffset = 0, rightArmOffset = 0;
    if (isWalking) {
        switch (walkAnimFrame) {
            case 0: leftArmOffset = 0; rightArmOffset = 0; break;     // Standing
            case 1: leftArmOffset = -6; rightArmOffset = 6; break;   // Left arm back, right forward
            case 2: leftArmOffset = 0; rightArmOffset = 0; break;     // Center
            case 3: leftArmOffset = 6; rightArmOffset = -6; break;   // Right arm back, left forward
        }
    }
    DrawRectangle(playerPixelX - 12, playerPixelY - 4 + bodyBob + leftArmOffset, 4, 12, {255, 220, 177, 255});
    DrawRectangle(playerPixelX + 8, playerPixelY - 4 + bodyBob + rightArmOffset, 4, 12, {255, 220, 177, 255});
    
    // Legs with walking animation (very pronounced, 4 distinct poses)
    int legOffset1 = 0, legOffset2 = 0;
    if (isWalking) {
        switch (walkAnimFrame) {
            case 0: legOffset1 = 0; legOffset2 = 0; break;     // Standing
            case 1: legOffset1 = -8; legOffset2 = 6; break;   // Left leg back, right forward
            case 2: legOffset1 = 0; legOffset2 = 0; break;     // Center
            case 3: legOffset1 = 6; legOffset2 = -8; break;   // Right leg back, left forward
        }
    }
    
    if (!isFemale) {
        // Animated legs for male
        DrawRectangle(playerPixelX - 6, playerPixelY + 16 + legOffset1, 4, 8, {0, 50, 100, 255});
        DrawRectangle(playerPixelX + 2, playerPixelY + 16 + legOffset2, 4, 8, {0, 50, 100, 255});
    }
    
    // Shoes with animation
    if (isFemale) {
        // Simple shoes for female with walking animation
        DrawRectangle(playerPixelX - 6, playerPixelY + 24 + legOffset1, 4, 4, {101, 67, 33, 255});
        DrawRectangle(playerPixelX + 2, playerPixelY + 24 + legOffset2, 4, 4, {101, 67, 33, 255});
    } else {
        // Boots for male with walking animation
        DrawRectangle(playerPixelX - 8, playerPixelY + 24 + legOffset1, 6, 4, {101, 67, 33, 255});
        DrawRectangle(playerPixelX + 2, playerPixelY + 24 + legOffset2, 6, 4, {101, 67, 33, 255});
    }
}

void TextAdventure::MovePlayer(float deltaX, float deltaY) {
    float newX = playerRoomX + deltaX;
    float newY = playerRoomY + deltaY;
    
    if (IsWalkable((int)newX, (int)playerRoomY, currentRoom)) {
        playerRoomX = newX;
    }
    
    if (IsWalkable((int)playerRoomX, (int)newY, currentRoom)) {
        playerRoomY = newY;
    }
    
    if (playerRoomX < 1.5f) playerRoomX = 1.5f;
    if (playerRoomX > ROOM_GRID_WIDTH - 2.5f) playerRoomX = ROOM_GRID_WIDTH - 2.5f;
    if (playerRoomY < 1.5f) playerRoomY = 1.5f;
    if (playerRoomY > ROOM_GRID_HEIGHT - 2.5f) playerRoomY = ROOM_GRID_HEIGHT - 2.5f;
}

bool TextAdventure::IsWalkable(int tileX, int tileY, Room* room) {
    if (tileX <= 0 || tileX >= ROOM_GRID_WIDTH - 1 || tileY <= 0 || tileY >= ROOM_GRID_HEIGHT - 1) {
        return false;
    }
    
    std::string roomName = room->GetName();
    
    if (roomName == "Entrance Hall") {
        for (int i = 0; i < 3; i++) {
            int torchTileX = 3 + i * 6;
            if (tileX == torchTileX && tileY >= 3 && tileY <= 5) {
                return false;
            }
        }
    } else if (roomName == "Armory") {
        for (int i = 0; i < 4; i++) {
            int rackTileX = 2 + i * 4;
            if (tileX == rackTileX && tileY >= 2 && tileY <= 3) {
                return false;
            }
        }
    } else if (roomName == "Treasure Chamber") {
        if (tileX >= 8 && tileX <= 9 && tileY == 6) {
            return false;
        }
    } else if (roomName == "Library") {
        for (int i = 0; i < 3; i++) {
            int shelfTileY = 2 + i * 3;
            if (tileX >= 2 && tileX <= 5 && tileY == shelfTileY) {
                return false;
            }
        }
    } else if (roomName == "Kitchen") {
        if (tileX >= 6 && tileX <= 8 && tileY >= 5 && tileY <= 6) {
            return false;
        }
        for (int i = 0; i < 3; i++) {
            int potTileX = 3 + i * 2;
            if (tileX == potTileX && tileY == 3) {
                return false;
            }
        }
    } else if (roomName == "Basement") {
        for (int i = 0; i < 6; i++) {
            int barrelTileX = 2 + i * 3;
            int barrelTileY = 2 + (i % 2) * 5;
            if (tileX == barrelTileX && (tileY == barrelTileY || tileY == barrelTileY + 1)) {
                return false;
            }
        }
    } else if (roomName == "Throne Room") {
        if (tileX >= 8 && tileX <= 10 && tileY >= 3 && tileY <= 6) {
            return false;
        }
    }
    
    return true;
}

void TextAdventure::UpdateMonsters() {
    if (!currentRoom) return;
    
    auto& monsters = currentRoom->GetMonsters();
    
    for (auto& monster : monsters) {
        if (!monster.alive) continue;
        
        monster.moveTimer += GetFrameTime();
        
        float distToPlayer = GetDistance(monster.x, monster.y, playerRoomX, playerRoomY);
        
        // Check if player is in aggro range
        if (distToPlayer <= monster.aggroRange) {
            monster.isAggro = true;
        }
        
        // Monster movement every 0.3 seconds (slower than player)
        if (monster.moveTimer >= 0.3f) {
            monster.moveTimer = 0.0f;
            
            if (monster.isAggro && distToPlayer > 1.0f) {
                // Move towards player
                float dx = playerRoomX - monster.x;
                float dy = playerRoomY - monster.y;
                
                // Determine direction to move (one axis at a time for retro feel)
                if (fabs(dx) > fabs(dy)) {
                    if (dx > 0) monster.targetX = monster.x + 1;
                    else monster.targetX = monster.x - 1;
                    monster.targetY = monster.y;
                } else {
                    if (dy > 0) monster.targetY = monster.y + 1;
                    else monster.targetY = monster.y - 1;
                    monster.targetX = monster.x;
                }
                
                // Check if target position is walkable
                if (IsWalkable((int)monster.targetX, (int)monster.targetY, currentRoom)) {
                    monster.x = monster.targetX;
                    monster.y = monster.targetY;
                }
            } else if (!monster.isAggro) {
                // Random wandering
                int direction = rand() % 5; // 0-3 = directions, 4 = stay still
                
                switch (direction) {
                    case 0: monster.targetX = monster.x - 1; monster.targetY = monster.y; break;
                    case 1: monster.targetX = monster.x + 1; monster.targetY = monster.y; break;
                    case 2: monster.targetX = monster.x; monster.targetY = monster.y - 1; break;
                    case 3: monster.targetX = monster.x; monster.targetY = monster.y + 1; break;
                    default: continue; // Stay still
                }
                
                // Check boundaries and walkability
                if (monster.targetX >= 1.5f && monster.targetX <= ROOM_GRID_WIDTH - 2.5f &&
                    monster.targetY >= 1.5f && monster.targetY <= ROOM_GRID_HEIGHT - 2.5f &&
                    IsWalkable((int)monster.targetX, (int)monster.targetY, currentRoom)) {
                    monster.x = monster.targetX;
                    monster.y = monster.targetY;
                }
            }
        }
    }
}

void TextAdventure::CheckMonsterCollisions() {
    if (!currentRoom) return;
    
    auto& monsters = const_cast<std::vector<Monster>&>(currentRoom->GetMonsters());
    
    for (auto& monster : monsters) {
        if (!monster.alive) continue;
        
        float distToPlayer = GetDistance(monster.x, monster.y, playerRoomX, playerRoomY);
        
        // If monster is adjacent to player, initiate combat
        if (distToPlayer <= 1.5f && monster.isAggro) {
            static float lastAttackTime = 0.0f;
            float currentTime = GetTime();
            
            // Attack every 2 seconds
            if (currentTime - lastAttackTime >= 2.0f) {
                lastAttackTime = currentTime;
                
                int damage = monster.attack + (rand() % 5) - GetTotalArmor();
                if (damage < 1) damage = 1; // Minimum damage
                playerHealth -= damage;
                
                AddMessage("The " + monster.name + " attacks you for " + std::to_string(damage) + " damage!");
                
                if (playerHealth <= 0) {
                    AddMessage("You have been defeated! Game Over.");
                    playerHealth = 0;
                }
            }
        }
    }
}

float TextAdventure::GetDistance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

void TextAdventure::AddMessage(const std::string& message) {
    messages.push_back(message);
    // Simple force scroll to bottom - will be handled in DrawTextPanel
    // No manual scroll offset manipulation here to avoid conflicts
}

std::vector<std::string> TextAdventure::SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::string TextAdventure::ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::vector<std::string> TextAdventure::WrapText(const std::string& text, int maxWidth, int fontSize) {
    std::vector<std::string> lines;
    
    if (text.empty()) {
        lines.push_back("");
        return lines;
    }
    
    // More conservative character width estimation
    int charWidth = fontSize * 0.6f; // Slightly wider estimation
    int maxChars = (maxWidth - 20) / charWidth; // Extra margin for safety
    
    if (maxChars <= 10) { // Minimum reasonable line length
        maxChars = 40; // Fallback to reasonable default
    }
    
    // Handle text that's already short enough
    if ((int)text.length() <= maxChars) {
        lines.push_back(text);
        return lines;
    }
    
    // Split text into logical chunks first (by newlines)
    std::vector<std::string> paragraphs = SplitString(text, '\n');
    
    for (const auto& paragraph : paragraphs) {
        if (paragraph.empty()) {
            lines.push_back(""); // Preserve empty lines
            continue;
        }
        
        // Split by words to avoid breaking words when possible
        std::vector<std::string> words = SplitString(paragraph, ' ');
        std::string currentLine = "";
        
        for (const auto& word : words) {
            // Check if adding this word would exceed the line limit
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
            
            if ((int)testLine.length() <= maxChars) {
                currentLine = testLine;
            } else {
                // Current line is full, start a new line
                if (!currentLine.empty()) {
                    lines.push_back(currentLine);
                    currentLine = word;
                } else {
                    // Single word is too long, break it carefully
                    if ((int)word.length() > maxChars) {
                        for (size_t i = 0; i < word.length(); i += maxChars) {
                            lines.push_back(word.substr(i, std::min((size_t)maxChars, word.length() - i)));
                        }
                        currentLine = "";
                    } else {
                        currentLine = word;
                    }
                }
            }
        }
        
        // Add the last line if it's not empty
        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }
    }
    
    // Ensure we have at least one line
    if (lines.empty()) {
        lines.push_back("");
    }
    
    return lines;
}

void TextAdventure::InitializeDungeon() {
    // Create all rooms using RoomFactory
    rooms = RoomFactory::CreateAllRooms();
    
    // Connect the rooms
    RoomFactory::ConnectRooms(rooms);
    
    // Set the starting room to entrance hall (first room)
    currentRoom = rooms[0].get();
    currentRoom->SetVisited(true);
}

int TextAdventure::GetTotalAttack() const {
    int total = basePlayerAttack;
    if (equippedWeaponIndex >= 0 && equippedWeaponIndex < (int)inventory.size()) {
        total += inventory[equippedWeaponIndex].damageBonus;
    }
    return total;
}

int TextAdventure::GetTotalArmor() const {
    int total = basePlayerArmor;
    if (equippedArmorIndex >= 0 && equippedArmorIndex < (int)inventory.size()) {
        total += inventory[equippedArmorIndex].armorBonus;
    }
    return total;
}

Item* TextAdventure::FindItemInInventory(const std::string& itemName) {
    for (auto& item : inventory) {
        if (item.name == itemName) {
            return &item;
        }
    }
    return nullptr;
}

int TextAdventure::FindItemIndexInInventory(const std::string& itemName) {
    for (int i = 0; i < (int)inventory.size(); ++i) {
        if (inventory[i].name == itemName) {
            return i;
        }
    }
    return -1;
}

void TextAdventure::EquipItem(const std::string& itemName) {
    int itemIndex = FindItemIndexInInventory(itemName);
    if (itemIndex == -1) {
        AddMessage("You don't have a " + itemName + " in your inventory.");
        return;
    }
    
    Item& item = inventory[itemIndex];
    
    // Auto-categorize items as weapon or armor based on their bonuses
    bool isWeapon = (item.damageBonus > 0);
    bool isArmor = (item.armorBonus > 0);
    
    if (isWeapon && isArmor) {
        // If item has both bonuses, ask player to specify
        AddMessage("The " + itemName + " can be used as weapon or armor. Use 'equip " + itemName + " weapon' or 'equip " + itemName + " armor'.");
        return;
    }
    else if (isWeapon) {
        // Equip as weapon - handle previous weapon
        if (equippedWeaponIndex >= 0 && equippedWeaponIndex < (int)inventory.size()) {
            AddMessage("You unequip the " + inventory[equippedWeaponIndex].name + " and drop it.");
            currentRoom->AddItem(inventory[equippedWeaponIndex]);
            
            // Remove the previously equipped weapon from inventory
            int oldWeaponIndex = equippedWeaponIndex;
            inventory.erase(inventory.begin() + oldWeaponIndex);
            
            // Adjust itemIndex if it's after the removed item
            if (itemIndex > oldWeaponIndex) {
                itemIndex--;
            }
            
            // Adjust armor index if it's after the removed item
            if (equippedArmorIndex > oldWeaponIndex) {
                equippedArmorIndex--;
            }
            
            equippedWeaponIndex = -1; // Reset first
        }
        
        equippedWeaponIndex = itemIndex;
        AddMessage("You equip the " + itemName + " as a weapon. (+" + std::to_string(item.damageBonus) + " attack)");
    }
    else if (isArmor) {
        // Equip as armor - handle previous armor
        if (equippedArmorIndex >= 0 && equippedArmorIndex < (int)inventory.size()) {
            AddMessage("You unequip the " + inventory[equippedArmorIndex].name + " and drop it.");
            currentRoom->AddItem(inventory[equippedArmorIndex]);
            
            // Remove the previously equipped armor from inventory
            int oldArmorIndex = equippedArmorIndex;
            inventory.erase(inventory.begin() + oldArmorIndex);
            
            // Adjust itemIndex if it's after the removed item
            if (itemIndex > oldArmorIndex) {
                itemIndex--;
            }
            
            // Adjust weapon index if it's after the removed item
            if (equippedWeaponIndex > oldArmorIndex) {
                equippedWeaponIndex--;
            }
            
            equippedArmorIndex = -1; // Reset first
        }
        
        equippedArmorIndex = itemIndex;
        AddMessage("You equip the " + itemName + " as armor. (+" + std::to_string(item.armorBonus) + " protection)");
    }
    else {
        // Default: try to equip as weapon slot (any item can be "equipped")
        if (equippedWeaponIndex >= 0 && equippedWeaponIndex < (int)inventory.size()) {
            AddMessage("You unequip the " + inventory[equippedWeaponIndex].name + " and drop it.");
            currentRoom->AddItem(inventory[equippedWeaponIndex]);
            
            // Remove the previously equipped weapon from inventory
            int oldWeaponIndex = equippedWeaponIndex;
            inventory.erase(inventory.begin() + oldWeaponIndex);
            
            // Adjust itemIndex if it's after the removed item
            if (itemIndex > oldWeaponIndex) {
                itemIndex--;
            }
            
            // Adjust armor index if it's after the removed item
            if (equippedArmorIndex > oldWeaponIndex) {
                equippedArmorIndex--;
            }
            
            equippedWeaponIndex = -1; // Reset first
        }
        
        equippedWeaponIndex = itemIndex;
        AddMessage("You equip the " + itemName + ". (No combat bonus)");
    }
}

void TextAdventure::DropItem(const std::string& itemName) {
    int itemIndex = FindItemIndexInInventory(itemName);
    if (itemIndex == -1) {
        AddMessage("You don't have a " + itemName + " to drop.");
        return;
    }
    
    // Unequip if equipped
    if (equippedWeaponIndex == itemIndex) {
        equippedWeaponIndex = -1;
        AddMessage("You unequip the " + itemName + ".");
    }
    if (equippedArmorIndex == itemIndex) {
        equippedArmorIndex = -1;
        AddMessage("You unequip the " + itemName + ".");
    }
    
    // Add to current room
    currentRoom->AddItem(inventory[itemIndex]);
    
    // Adjust equipment indices if they reference items after the dropped item
    if (equippedWeaponIndex > itemIndex) {
        equippedWeaponIndex--;
    }
    if (equippedArmorIndex > itemIndex) {
        equippedArmorIndex--;
    }
    
    inventory.erase(inventory.begin() + itemIndex);
    AddMessage("You drop the " + itemName + ".");
}

void TextAdventure::ShowItemStats(const std::string& itemName) {
    int itemIndex = FindItemIndexInInventory(itemName);
    if (itemIndex == -1) {
        AddMessage("You don't have a " + itemName + " in your inventory.");
        return;
    }
    
    Item& item = inventory[itemIndex];
    
    AddMessage("=== " + item.name + " STATS ===");
    AddMessage(item.description);
    AddMessage("");
    
    // Show current attack and armor values
    int currentAttack = GetTotalAttack();
    int currentArmor = GetTotalArmor();
    
    // Show what attack/armor would be with this item
    if (item.damageBonus > 0) {
        AddMessage("WEAPON DAMAGE: +" + std::to_string(item.damageBonus) + " attack bonus");
        if (equippedWeaponIndex != itemIndex) {
            int newAttack = basePlayerAttack + item.damageBonus;
            AddMessage("Total attack with this weapon: " + std::to_string(newAttack) + 
                      " (currently: " + std::to_string(currentAttack) + ")");
        } else {
            AddMessage("Currently equipped as weapon - contributing to your " + std::to_string(currentAttack) + " total attack");
        }
    } else {
        AddMessage("WEAPON DAMAGE: +0 attack bonus");
    }
    
    if (item.armorBonus > 0) {
        AddMessage("ARMOR PROTECTION: +" + std::to_string(item.armorBonus) + " armor bonus");
        if (equippedArmorIndex != itemIndex) {
            int newArmor = basePlayerArmor + item.armorBonus;
            AddMessage("Total armor with this item: " + std::to_string(newArmor) + 
                      " (currently: " + std::to_string(currentArmor) + ")");
        } else {
            AddMessage("Currently equipped as armor - contributing to your " + std::to_string(currentArmor) + " total armor");
        }
    } else {
        AddMessage("ARMOR PROTECTION: +0 armor bonus");
    }
    
    AddMessage("");
    
    // Determine item classification
    if (item.damageBonus > 0 && item.armorBonus > 0) {
        AddMessage("TYPE: Hybrid (can be weapon or armor)");
    } else if (item.damageBonus > 0) {
        AddMessage("TYPE: Weapon");
    } else if (item.armorBonus > 0) {
        AddMessage("TYPE: Armor");
    } else {
        AddMessage("TYPE: Miscellaneous item");
    }
    
    // Show equipment status
    if (equippedWeaponIndex == itemIndex) {
        AddMessage("STATUS: EQUIPPED as weapon (ERROR if armor item!)");
    } else if (equippedArmorIndex == itemIndex) {
        AddMessage("STATUS: EQUIPPED as armor");
    } else {
        AddMessage("STATUS: In inventory (not equipped)");
    }
}

void TextAdventure::UseItem(const std::string& itemName) {
    int itemIndex = FindItemIndexInInventory(itemName);
    if (itemIndex == -1) {
        AddMessage("You don't have a " + itemName + " to use.");
        return;
    }
    
    if (itemName == "plaster") {
        if (playerHealth >= 100) {
            AddMessage("You are already at full health!");
            return;
        }
        
        int healthBefore = playerHealth;
        playerHealth += 20;
        if (playerHealth > 100) playerHealth = 100;
        
        int healedAmount = playerHealth - healthBefore;
        AddMessage("You use the magical plaster and heal for " + std::to_string(healedAmount) + " health!");
        AddMessage("Your health is now " + std::to_string(playerHealth) + "/100.");
        
        // Remove the plaster from inventory (single use)
        inventory.erase(inventory.begin() + itemIndex);
        
        // Adjust equipment indices if they reference items after the removed item
        if (equippedWeaponIndex > itemIndex) {
            equippedWeaponIndex--;
        }
        if (equippedArmorIndex > itemIndex) {
            equippedArmorIndex--;
        }
        // If the removed item was equipped, unequip it
        if (equippedWeaponIndex == itemIndex) {
            equippedWeaponIndex = -1;
        }
        if (equippedArmorIndex == itemIndex) {
            equippedArmorIndex = -1;
        }
        
        AddMessage("The plaster dissolves after use.");
    } else if (itemName == "gem" && currentRoom->GetName() == "Throne Room") {
        if (gemUsed) {
            AddMessage("You have already used the gem here.");
            return;
        }
        
        gemUsed = true;
        AddMessage("You approach the ancient throne and notice a ruby-shaped indentation in its armrest.");
        AddMessage("You carefully place the sparkling ruby into the slot...");
        AddMessage("*CLICK* The gem slots perfectly into place with a satisfying sound!");
        AddMessage("Suddenly, the floor trembles and ancient mechanisms whir to life!");
        AddMessage("A hidden door slides open in the south wall, revealing sunlight beyond!");
        
        // Create the exit to the meadow
        Room* throne = rooms[8].get();
        Room* meadow = rooms[11].get();
        throne->SetExit("south", meadow);
        meadow->SetExit("north", throne);
        
        // Remove the gem from inventory
        inventory.erase(inventory.begin() + itemIndex);
        
        // Adjust equipment indices if they reference items after the removed item
        if (equippedWeaponIndex > itemIndex) {
            equippedWeaponIndex--;
        }
        if (equippedArmorIndex > itemIndex) {
            equippedArmorIndex--;
        }
        // If the removed item was equipped, unequip it
        if (equippedWeaponIndex == itemIndex) {
            equippedWeaponIndex = -1;
        }
        if (equippedArmorIndex == itemIndex) {
            equippedArmorIndex = -1;
        }
    } else if (itemName == "gem" && currentRoom->GetName() != "Throne Room") {
        AddMessage("The gem doesn't seem to have any effect here. Perhaps it belongs somewhere special...");
    } else if (itemName == "gold" && currentRoom->GetName() == "Dark Room") {
        if (strangeMet) {
            AddMessage("You have already traded with the mysterious stranger.");
            return;
        }
        
        strangeMet = true;
        hasTeleport = true;
        AddMessage("The mysterious stranger's eyes gleam as you offer the gold.");
        AddMessage("'Excellent...' the stranger whispers, taking the gold with bony fingers.");
        AddMessage("'In return, I shall grant you the ancient art of teleportation...'");
        AddMessage("Dark energy swirls around you as mystical knowledge floods your mind!");
        AddMessage("You have learned TELEPORT! When viewing the map, click on any explored room to instantly travel there.");
        
        // Remove gold from inventory
        inventory.erase(inventory.begin() + itemIndex);
        
        // Adjust equipment indices if they reference items after the removed item
        if (equippedWeaponIndex > itemIndex) {
            equippedWeaponIndex--;
        }
        if (equippedArmorIndex > itemIndex) {
            equippedArmorIndex--;
        }
        if (equippedWeaponIndex == itemIndex) {
            equippedWeaponIndex = -1;
        }
        if (equippedArmorIndex == itemIndex) {
            equippedArmorIndex = -1;
        }
    } else if (itemName == "gold" && currentRoom->GetName() != "Dark Room") {
        AddMessage("The gold feels heavy in your hands, but there's nothing to spend it on here.");
    } else if (itemName == "note" && !noteRead) {
        AddMessage("You carefully unfold the ancient, weathered parchment and read:");
        AddMessage("");
        AddMessage("'To whoever finds this cursed record...'");
        AddMessage("'I have done something terrible. The Ancient Staff of [text torn]'");
        AddMessage("'...possessed unimaginable power. In my hubris, I tried to control it.'");
        AddMessage("'The magic was too strong. It nearly destroyed everything.'");
        AddMessage("");
        AddMessage("'I have broken the staff into four parts and hidden them:'");
        AddMessage("'- The Staff itself, where meals are prepared'");
        AddMessage("'- The Diamond, in a place of prayer and reverence'");
        AddMessage("'- The Emerald, where wine sleeps in darkness'");
        AddMessage("'- The Opal, where the weary rest their heads'");
        AddMessage("");
        AddMessage("'I have sealed these places with guardians and creatures.'");
        AddMessage("'The staff must never be whole again, for its true power is...'");
        AddMessage("[The rest of the note is torn and unreadable]");
        AddMessage("");
        AddMessage("You feel a strange energy pulse through the dungeon...");
        
        noteRead = true;
        
        // Reveal the Chapel (east of Sunlit Meadow)
        Room* sunlitMeadow = rooms[11].get();
        Room* chapel = rooms[13].get();
        sunlitMeadow->SetExit("east", chapel);
        chapel->SetExit("west", sunlitMeadow);
        
        // Reveal the Sleeping Quarters (north of Throne Room)
        Room* throneRoom = rooms[8].get();
        Room* sleepingQuarters = rooms[14].get();
        throneRoom->SetExit("north", sleepingQuarters);
        sleepingQuarters->SetExit("south", throneRoom);
        
        // Add hidden items to rooms
        Room* kitchen = rooms[6].get();
        kitchen->AddItem(Item("staff", "An ancient wooden staff, carved with mysterious runes. Its wood is dark with age.", true, ItemType::MISC, 0, 0));
        
        Room* basement = rooms[7].get();
        basement->AddItem(Item("emerald", "A brilliant green emerald that seems to glow with inner light.", true, ItemType::MISC, 0, 0));
        
        chapel->AddItem(Item("diamond", "A flawless diamond that sparkles with pure, radiant light.", true, ItemType::MISC, 0, 0));
        
        sleepingQuarters->AddItem(Item("opal", "A shimmering opal that displays all colors of the rainbow.", true, ItemType::MISC, 0, 0));
        
        AddMessage("You hear distant rumbling and shifting sounds throughout the dungeon...");
        AddMessage("New paths have opened!");
    } else if (itemName == "?????" && staffComplete && !gameEnding) {
        AddMessage("You raise the Ancient Staff of Power high above your head...");
        AddMessage("The gems in the staff head begin to glow with intense magical energy!");
        AddMessage("Suddenly, you hear a distant whistling sound from far above...");
        AddMessage("The whistling grows louder and more intense...");
        AddMessage("Something is dropping from the sky!");
        AddMessage("");
        AddMessage("The staff's power has torn a rift in the heavens themselves!");
        AddMessage("A massive asteroid hurtles through the atmosphere toward the earth!");
        AddMessage("You realize with horror what the staff's true power was...");
        AddMessage("");
        AddMessage("IMPACT!");
        AddMessage("");
        AddMessage("Type 'continue' to continue...");
        
        gameEnding = true;
        waitingForContinue = true;
    } else {
        AddMessage("You can't use the " + itemName + ".");
    }
}

void TextAdventure::AttackNearestMonster() {
    if (!currentRoom) return;
    
    auto& monsters = currentRoom->GetMonsters();
    Monster* closestMonster = nullptr;
    float closestDistance = 3.0f; // Attack range
    
    // Find the closest living monster within attack range
    for (auto& monster : monsters) {
        if (monster.alive) {
            float distance = GetDistance(playerRoomX, playerRoomY, monster.x, monster.y);
            if (distance < closestDistance) {
                closestDistance = distance;
                closestMonster = &monster;
            }
        }
    }
    
    if (closestMonster) {
        int damage = GetTotalAttack() + (rand() % 3) - 1; // Less variable damage
        if (damage < 1) damage = 1;
        
        closestMonster->health -= damage;
        closestMonster->isAggro = true;
        
        AddMessage("You attack the " + closestMonster->name + " for " + std::to_string(damage) + " damage!");
        
        if (closestMonster->health <= 0) {
            closestMonster->alive = false;
            AddMessage("The " + closestMonster->name + " is defeated!");
        } else {
            AddMessage("The " + closestMonster->name + " has " + std::to_string(closestMonster->health) + " HP left.");
        }
    } else {
        AddMessage("No monsters nearby to attack.");
    }
}

void TextAdventure::DrawPlayerStats() {
    int statsX = MAP_WIDTH + 40;
    int statsY = SCREEN_HEIGHT - 200; // Position above input area
    
    // Stats background
    DrawRectangle(statsX, statsY, TEXT_WIDTH, 120, {25, 25, 35, 255});
    DrawRectangleLines(statsX, statsY, TEXT_WIDTH, 120, {100, 100, 120, 255});
    
    // Title
    DrawText("PLAYER STATS", statsX + 20, statsY + 10, 24, {220, 220, 220, 255});
    
    // Health bar
    std::string healthText = "Health: " + std::to_string(playerHealth) + "/100";
    DrawText(healthText.c_str(), statsX + 20, statsY + 40, 20, {255, 100, 100, 255});
    
    // Health bar visual
    int barWidth = 200;
    int barHeight = 8;
    float healthPercent = (float)playerHealth / 100.0f;
    DrawRectangle(statsX + 20, statsY + 65, barWidth, barHeight, {100, 100, 100, 255});
    DrawRectangle(statsX + 20, statsY + 65, (int)(barWidth * healthPercent), barHeight, {255, 100, 100, 255});
    
    // Attack and Armor (with equipment bonuses)
    std::string attackText = "Attack: " + std::to_string(GetTotalAttack()) + 
                           " (" + std::to_string(basePlayerAttack) + " base";
    if (equippedWeaponIndex >= 0 && equippedWeaponIndex < (int)inventory.size()) {
        attackText += " + " + std::to_string(inventory[equippedWeaponIndex].damageBonus) + " weapon";
    }
    attackText += ")";
    
    std::string armorText = "Armor: " + std::to_string(GetTotalArmor()) + 
                          " (" + std::to_string(basePlayerArmor) + " base";
    if (equippedArmorIndex >= 0 && equippedArmorIndex < (int)inventory.size()) {
        armorText += " + " + std::to_string(inventory[equippedArmorIndex].armorBonus) + " armor";
    }
    armorText += ")";
    
    DrawText(attackText.c_str(), statsX + 20, statsY + 80, 16, {255, 200, 100, 255});
    DrawText(armorText.c_str(), statsX + 20, statsY + 100, 16, {100, 200, 255, 255});
    
    // Inventory
    std::string invText = "Inventory: ";
    if (inventory.empty()) {
        invText += "Empty";
    } else {
        for (size_t i = 0; i < inventory.size() && i < 3; ++i) {
            if (i > 0) invText += ", ";
            invText += inventory[i].name;
        }
        if (inventory.size() > 3) {
            invText += "... (" + std::to_string(inventory.size()) + " items)";
        }
    }
    DrawText(invText.c_str(), statsX + 20, statsY + 120, 16, {200, 200, 200, 255});
}

void TextAdventure::DrawDungeonMap() {
    // Draw background for map area
    DrawRectangle(20, 20, MAP_WIDTH, SCREEN_HEIGHT - 40, {15, 15, 25, 255});
    DrawRectangleLines(20, 20, MAP_WIDTH, SCREEN_HEIGHT - 40, {100, 100, 120, 255});
    
    // Map title
    DrawText("DUNGEON MAP", 40, 40, 32, {220, 220, 220, 255});
    DrawText("Press SHIFT to exit", 40, 80, 16, {150, 150, 150, 255});
    DrawText("Lines show connections between rooms", 40, 100, 14, {120, 120, 120, 255});
    
    // Grid layout for rooms - arranged to match actual connections
    int roomWidth = 140;
    int roomHeight = 80;
    int startX = 80;
    int startY = 140;
    
    // Room positions in a layout that makes logical sense
    struct RoomPos {
        int x, y;
        std::string name;
        bool available;
    };
    
    // Exact game layout - matching north/south/east/west directions:
    //                    [Monster Lair]
    //                         |
    //        [Basement]---[Dark Corridor]
    //            |             |
    //        [Library]---[Entrance Hall]---[Armory]---[Treasure]---[Throne]
    //            |             |              |
    //      [Infirmary]     [Kitchen]---[Garden]
    
    std::vector<RoomPos> roomPositions = {
        // Top row (far north)
        {startX + roomWidth, startY, "Monster Lair", true},
        
        // Second row (north)
        {startX, startY + roomHeight, "Basement", true},
        {startX + roomWidth, startY + roomHeight, "Dark Corridor", true},
        
        // Third row (center) - main horizontal chain
        {startX, startY + roomHeight * 2, "Library", true},
        {startX + roomWidth, startY + roomHeight * 2, "Entrance Hall", true},
        {startX + roomWidth * 2, startY + roomHeight * 2, "Armory", true},
        {startX + roomWidth * 3, startY + roomHeight * 2, "Treasure Chamber", true},
        {startX + roomWidth * 4, startY + roomHeight * 2, "Throne Room", true},
        
        // Bottom row (south)
        {startX + roomWidth, startY + roomHeight * 3, "Kitchen", true},
        {startX + roomWidth * 2, startY + roomHeight * 3, "Garden", true},
        
        // Hidden infirmary (only show if revealed) - south of library
        {startX, startY + roomHeight * 3, "Infirmary", infirmaryRevealed},
        
        // Hidden ending meadow (only show if gem is used) - south of throne room
        {startX + roomWidth * 4, startY + roomHeight * 3, "Sunlit Meadow", gemUsed},
        
        // Hidden dark room - below garden
        {startX + roomWidth * 2, startY + roomHeight * 4, "Dark Room", true},
        
        // Hidden Chapel and Sleeping Quarters (only show if note is read)
        {startX + roomWidth * 5, startY + roomHeight * 3, "Chapel", noteRead}, // East of Sunlit Meadow
        {startX + roomWidth * 4, startY - roomHeight, "Sleeping Quarters", noteRead} // North of Throne Room
    };
    
    // Draw rooms
    for (const auto& roomPos : roomPositions) {
        if (!roomPos.available) continue;
        
        // Determine room color based on visited status and current location
        Color roomColor = {50, 50, 60, 255}; // Default unvisited
        Color textColor = {150, 150, 150, 255};
        
        // Find the room by name to check if visited
        Room* room = nullptr;
        for (auto& r : rooms) {
            if (r->GetName() == roomPos.name) {
                room = r.get();
                break;
            }
        }
        
        if (room) {
            if (room == currentRoom) {
                roomColor = {100, 150, 100, 255}; // Current room (green)
                textColor = {220, 255, 220, 255};
            } else if (room->IsVisited()) {
                roomColor = {70, 70, 80, 255}; // Visited room
                textColor = {200, 200, 200, 255};
            }
            
            // Color rooms by their staff parts (only after note is read)
            if (noteRead) {
                if (roomPos.name == "Kitchen" && !hasStaff) {
                    roomColor = {139, 69, 19, 255}; // Brown for wooden staff
                    textColor = {255, 220, 180, 255};
                } else if (roomPos.name == "Chapel" && !hasDiamond) {
                    roomColor = {200, 200, 255, 255}; // Diamond white/blue
                    textColor = {255, 255, 255, 255};
                } else if (roomPos.name == "Basement" && !hasEmerald) {
                    roomColor = {50, 200, 50, 255}; // Emerald green
                    textColor = {150, 255, 150, 255};
                } else if (roomPos.name == "Sleeping Quarters" && !hasOpal) {
                    roomColor = {255, 150, 200, 255}; // Opal rainbow (pink tint)
                    textColor = {255, 255, 255, 255};
                }
            }
        }
        
        // Draw room rectangle with better proportions
        int rectWidth = roomWidth - 15;
        int rectHeight = roomHeight - 15;
        DrawRectangle(roomPos.x, roomPos.y, rectWidth, rectHeight, roomColor);
        DrawRectangleLines(roomPos.x, roomPos.y, rectWidth, rectHeight, textColor);
        
        // Handle teleport clicks if enabled
        if (hasTeleport && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            if (mousePos.x >= roomPos.x && mousePos.x <= roomPos.x + rectWidth &&
                mousePos.y >= roomPos.y && mousePos.y <= roomPos.y + rectHeight) {
                // Find the room and check if visited
                Room* targetRoom = nullptr;
                for (auto& r : rooms) {
                    if (r->GetName() == roomPos.name) {
                        targetRoom = r.get();
                        break;
                    }
                }
                
                if (targetRoom && targetRoom->IsVisited()) {
                    TeleportToRoom(roomPos.name);
                    return; // Exit map view after teleport
                }
            }
        }
        
        // Draw room name with better formatting
        std::string displayName = roomPos.name;
        size_t spacePos = displayName.find(' ');
        if (spacePos != std::string::npos) {
            // Split into two lines for better readability
            std::string line1 = displayName.substr(0, spacePos);
            std::string line2 = displayName.substr(spacePos + 1);
            
            // Center the text in the room
            int textX = roomPos.x + (rectWidth - MeasureText(line1.c_str(), 16)) / 2;
            int textX2 = roomPos.x + (rectWidth - MeasureText(line2.c_str(), 16)) / 2;
            
            DrawText(line1.c_str(), textX, roomPos.y + 25, 16, textColor);
            DrawText(line2.c_str(), textX2, roomPos.y + 45, 16, textColor);
        } else {
            // Single line - center it
            int textX = roomPos.x + (rectWidth - MeasureText(displayName.c_str(), 16)) / 2;
            DrawText(displayName.c_str(), textX, roomPos.y + 35, 16, textColor);
        }
    }
    
    // Draw connections between rooms
    Color connectionColor = {80, 80, 100, 255};
    int lineThickness = 2;
    
    // Exact connections from the game code
    std::vector<std::pair<std::string, std::string>> connections = {
        // North-South connections
        {"Monster Lair", "Dark Corridor"},
        {"Dark Corridor", "Entrance Hall"},
        {"Entrance Hall", "Kitchen"},
        {"Library", "Basement"},
        
        // East-West connections  
        {"Basement", "Dark Corridor"},
        {"Library", "Entrance Hall"},
        {"Entrance Hall", "Armory"},
        {"Kitchen", "Garden"},
        {"Garden", "Armory"},
        {"Garden", "Dark Room"},
        {"Treasure Chamber", "Throne Room"},
        
        // Diagonal/Cross connections
        {"Monster Lair", "Basement"}
    };
    
    // Add infirmary connection if revealed
    if (infirmaryRevealed) {
        connections.push_back({"Library", "Infirmary"});
    }
    
    // Add armory to treasure connection if key is found
    if (hasKey) {
        connections.push_back({"Armory", "Treasure Chamber"});
    }
    
    // Add throne room to meadow connection if gem is used
    if (gemUsed) {
        connections.push_back({"Throne Room", "Sunlit Meadow"});
    }
    
    // Add Chapel and Sleeping Quarters connections if note is read
    if (noteRead) {
        connections.push_back({"Sunlit Meadow", "Chapel"});
        connections.push_back({"Throne Room", "Sleeping Quarters"});
    }
    
    // Draw connection lines
    for (const auto& connection : connections) {
        auto room1Pos = std::find_if(roomPositions.begin(), roomPositions.end(),
            [&connection](const RoomPos& pos) { return pos.name == connection.first && pos.available; });
        auto room2Pos = std::find_if(roomPositions.begin(), roomPositions.end(),
            [&connection](const RoomPos& pos) { return pos.name == connection.second && pos.available; });
            
        if (room1Pos != roomPositions.end() && room2Pos != roomPositions.end()) {
            int x1 = room1Pos->x + (roomWidth - 15) / 2;
            int y1 = room1Pos->y + (roomHeight - 15) / 2;
            int x2 = room2Pos->x + (roomWidth - 15) / 2;
            int y2 = room2Pos->y + (roomHeight - 15) / 2;
            
            DrawLineEx({(float)x1, (float)y1}, {(float)x2, (float)y2}, lineThickness, connectionColor);
        }
    }
    
    // Draw legend
    int legendY = startY + roomHeight * 4 + 20;
    DrawText("LEGEND:", 50, legendY, 16, {200, 200, 200, 255});
    DrawRectangle(50, legendY + 25, 20, 15, {100, 150, 100, 255});
    DrawText("Current Room", 80, legendY + 25, 14, {200, 200, 200, 255});
    DrawRectangle(50, legendY + 45, 20, 15, {70, 70, 80, 255});
    DrawText("Visited Room", 80, legendY + 45, 14, {200, 200, 200, 255});
    DrawRectangle(50, legendY + 65, 20, 15, {50, 50, 60, 255});
    DrawText("Unvisited Room", 80, legendY + 65, 14, {200, 200, 200, 255});
}

void TextAdventure::TeleportToRoom(const std::string& roomName) {
    // Find the target room
    Room* targetRoom = nullptr;
    for (auto& room : rooms) {
        if (room->GetName() == roomName) {
            targetRoom = room.get();
            break;
        }
    }
    
    if (targetRoom && targetRoom->IsVisited()) {
        currentRoom = targetRoom;   
        playerRoomX = 12.0f;
        playerRoomY = 9.0f;
        inMapView = false;
        
        AddMessage("*Magical energy swirls around you*");
        AddMessage("You teleport to the " + roomName + "!");
        AddMessage(currentRoom->GetName());
        
        // Show room description and exits
        std::string fullDesc = currentRoom->GetDescription();
        auto exits = currentRoom->GetExits();
        if (!exits.empty()) {
            fullDesc += "\n\nExits: ";
            for (size_t i = 0; i < exits.size(); ++i) {
                if (i > 0) fullDesc += ", ";
                fullDesc += exits[i];
            }
        }
        
        // Add locked exits for armory
        if (currentRoom->GetName() == "Armory" && !hasKey) {
            if (!exits.empty()) {
                fullDesc += ", east (locked)";
            } else {
                fullDesc += "\n\nExits: east (locked)";
            }
        }
        
        AddMessage(fullDesc);
    }
}