EXEC_NAME := ascii-rasterizer

CFLAGS    += -I./src
CFLAGS    += -I./example_scenes

CFLAGS    += -D'SCREEN_WIDTH=60' -D'SCREEN_HEIGHT=30'
CFLAGS    += -D'MS_PER_UPDATE=20'

CFLAGS    += -D'EXTRA_WINDOW_WIDTH=650' -D'EXTRA_WINDOW_HEIGHT=100'
CFLAGS    += -D'FONT_PATH="ttf/terminus.ttf"'
CFLAGS    += -D'FONT_SIZE=32'

SRC_FILES  := $(wildcard src/*.c) $(wildcard example_scenes/*.c) 
OBJ_FILES  := $(SRC_FILES:.c=.o)

LD_FLAGS  += -lm
LD_FLAGS  += -lSDL2main -lSDL2 
LD_FLAGS  += -lSDL2_ttf

RELEASE ?= 0 # use `make RELEASE=1` to turn on release flags
DEBUG ?= 0 # use `make DEBUG=1` to turn on debugging flags

ifeq ($(RELEASE), 1) 
	CFLAGS    += -O3 -march=native
else ifeq ($(DEBUG), 1) 
	CC        := clang
	CFLAGS    += -DDEBUG -ggdb3
	CFLAGS    += -std=c11
	CFLAGS    += -Weverything -Wno-unsafe-buffer-usage -Wno-missing-noreturn -Wno-declaration-after-statement -Wno-padded -Wno-gnu-binary-literal
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
