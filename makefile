EXEC_NAME := a.out

CFLAGS    += -I./src
CFLAGS    += -I./lib/ctl
CFLAGS    += -I./example_scenes

CFLAGS    += -D'SCREEN_WIDTH=60' -D'SCREEN_HEIGHT=30'
CFLAGS    += -D'MS_PER_UPDATE=100'

CFLAGS    += -D'SDL_WINDOW_WIDTH=650' -D'SDL_WINDOW_HEIGHT=100'
CFLAGS    += -D'SDL_FONT_PATH="ttf/terminus.ttf"'
CFLAGS    += -D'SDL_FONT_SIZE=32'

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
	CFLAGS    += -fPIC
	CFLAGS    += -Weverything -Wno-unsafe-buffer-usage -Wno-missing-noreturn
	CFLAGS    += -Wno-declaration-after-statement -Wno-padded -Wno-gnu-binary-literal -Wno-vla
	CFLAGS    += -Wno-unused-but-set-variable
	CFLAGS    += -fsanitize=undefined,address
	LD_FLAGS  += -fsanitize=undefined,address
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
