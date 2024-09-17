# Compiler
CC = gcc

# Directories
SRC = ./src
INCLUDE = ./include
BIN = ./bin
BUILD = ./build
HELPING_DIR := $(SRC)/helping

# Compiler flags
CFLAGS = -pthread -Wall -g3 -std=c99 -I$(INCLUDE)

# Source files in src/helping
HELPING_SRCS := $(wildcard $(HELPING_DIR)/*.c)
# Object files for helping
HELPING_OBJS := $(patsubst $(HELPING_DIR)/%.c,$(BUILD)/%.o,$(HELPING_SRCS))

# Rule to build object files from helping source files
$(BUILD)/%.o: $(HELPING_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Source files
COMMANDER_SOURCE := $(SRC)/jobCommander.c
EXECUTOR_SOURCE := $(SRC)/jobExecutorServer.c
PROGDELAY_SOURCE := $(SRC)/progDelay.c



# Object files
COMMANDER_OBJECT := $(BUILD)/jobCommander.o $(HELPING_OBJS)
EXECUTOR_OBJECT := $(BUILD)/jobExecutorServer.o $(HELPING_OBJS)
PROGDELAY_OBJECT := $(BUILD)/progDelay.o




# Rule to build object files from main source files
$(BUILD)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Targets
all: $(BIN)/jobCommander $(BIN)/jobExecutorServer $(BIN)/progDelay

$(BIN)/jobCommander: $(COMMANDER_OBJECT)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/jobExecutorServer: $(EXECUTOR_OBJECT)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/progDelay: $(PROGDELAY_OBJECT)
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -f $(BIN)/jobCommander $(BIN)/jobExecutorServer $(BIN)/progDelay $(BUILD)/*.o

help:
	@echo "Available targets:"
	@echo "  all: Build all executables"
	@echo "  clean: Clean object files and executables"
	@echo "  help: Display this help message"