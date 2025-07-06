CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -Iinclude
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRCDIR = src
OBJDIR = obj
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/textadventure.cpp $(SRCDIR)/room.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET = retro_dungeon

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)