# Compiler and flags
CC = gcc
CFLAGS = -Werror -g -fsanitize=address -MMD -MP
LDFLAGS = -fsanitize=address -lm

# Dossiers
SRC_DIR = src
BIN_DIR = bin

# Récupère tous les .c récursivement, mais exclut syntaxList.c
SRC = $(filter-out $(SRC_DIR)/syntaxList.c,$(shell find $(SRC_DIR) -name '*.c'))

# Liste des objets correspondants dans bin/
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)

# Binaire final
TARGET = $(BIN_DIR)/main

# Règle par défaut
all: $(TARGET)

# Linking
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# Compilation .c -> .o (crée les sous-dossiers si besoin)
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Inclure les dépendances
-include $(DEP)

# Nettoyage
clean:
	rm -rf $(BIN_DIR)


# Débogage avec gdb
debug: $(TARGET)
	gdb --args ./$(TARGET) $(ARGS)

# Par défaut
ARGS ?= ""

run: all
	clear
	./$(TARGET) $(ARGS) 2>&1 | tee draft/output.txt

print: all
	clear
	./$(TARGET) $(ARGS)

build: all