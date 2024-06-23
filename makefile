EXEC_NAME := ascii-library

SRC_PATH  := src

# CC        := gcc
# CFLAGS    += -I$(SRC_PATH)
# CFLAGS    += -lm 
# CFLAGS    += -Wall -Wextra -pedantic
# CFLAGS    += -fsanitize=address,undefined

CC        := clang
CFLAGS    += -I$(SRC_PATH)
CFLAGS    += -std=c11
CFLAGS    += -ggdb3
CFLAGS    += -Weverything -Wno-unsafe-buffer-usage -Wno-missing-noreturn -Wno-pre-c2x-compat
CFLAGS    += -Wno-declaration-after-statement
CFLAGS    += -fsanitize=address,undefined
CFLAGS    += -D'SCREEN_WIDTH=60' -D'SCREEN_HEIGHT=30'
CFLAGS    += -D'MS_PER_UPDATE=200'
# CFLAGS    += -DDEBUG

LD_FLAGS  := -lm
LD_FLAGS  += -fsanitize=address,undefined

_SRC_FILES := screen.c draw.c misc.c
_SRC_FILES += main.c
# _SRC_FILES += $(addprefix scenes/, $(notdir $(wildcard $(SRC_PATH)/scenes/*.c)))

SRC_FILES  := $(addprefix $(SRC_PATH)/, $(_SRC_FILES))
SRC_FILES  += $(wildcard example_scenes/*.c) 

OBJ_FILES  := $(SRC_FILES:.c=.o)

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
