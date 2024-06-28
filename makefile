EXEC_NAME := ascii-rasterizer

CFLAGS    += -I./src
CFLAGS    += -I./example_scenes

CFLAGS    += -D'SCREEN_WIDTH=60' -D'SCREEN_HEIGHT=30'
CFLAGS    += -D'MS_PER_UPDATE=100'
CFLAGS    += -D'COLOR_INTENSITY=0.8f'

CFLAGS    += -D'EXTRA_WINDOW_WIDTH=650' -D'EXTRA_WINDOW_HEIGHT=100'
CFLAGS    += -D'FONT_PATH="terminus.ttf"'
CFLAGS    += -D'FONT_SIZE=32'

SRC_FILES  := $(wildcard src/*.c) $(wildcard example_scenes/*.c) 
OBJ_FILES  := $(SRC_FILES:.c=.o)

LD_FLAGS  += -lm
LD_FLAGS  += -lSDL2main -lSDL2 
LD_FLAGS  += -lSDL2_ttf

# use `make RELEASE=0` to turn off release build
RELEASE ?= 1
ifeq ($(RELEASE), 1) 
	CFLAGS    += -O3 -march=native
endif

# use `make DEBUG=1` to turn on debugging flags
DEBUG ?= 0
ifeq ($(DEBUG), 1) 
	CC        := clang
	CFLAGS    += -std=c11 -Wmost
	CFLAGS    += -DDEBUG -ggdb3
	CFLAGS    += -fsanitize=address,undefined
	LD_FLAGS  += -fsanitize=address,undefined
endif

.PHONY: all clean test

all: $(EXEC_NAME)

$(EXEC_NAME): $(OBJ_FILES)
	$(CC) $(LD_FLAGS) $^ -o $(EXEC_NAME)

$(OBJ_FILES): $(SRC_FILES)

$(SRC_FILES):
	$(CC) -c $(CFLAGS) $@

clean:
	rm -rf $(OBJ_FILES)
	rm -rf $(EXEC_NAME)
