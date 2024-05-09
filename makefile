EXEC_NAME := ascii-engine

SRC_PATH  := src

CC        := gcc
CFLAGS    := -Wall -lm -I$(SRC_PATH)
CFLAGS    += -Wextra -pedantic -fsanitize=undefined
CFLAGS    += -g -ggdb3
CFLAGS    += -D'SCREEN_WIDTH=60' -D'SCREEN_HEIGHT=30'
# CFLAGS    += -DDEBUG

_SRC_FILES := screen.c transform.c draw.c misc.c
_SRC_FILES += $(addprefix scenes/, $(notdir $(wildcard $(SRC_PATH)/scenes/*.c)))
_SRC_FILES += main.c

SRC_FILES  := $(addprefix $(SRC_PATH)/, $(_SRC_FILES))
OBJ_FILES  := $(SRC_FILES:.c=.o)

.PHONY: all clean test

all: $(EXEC_NAME)

$(EXEC_NAME): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $(EXEC_NAME)

$(OBJ_FILES): $(SRC_FILES)

$(SRC_FILES):
	$(CC) -c $(CFLAGS) $@

clean:
	rm -rf $(OBJ_FILES)
	rm -rf $(EXEC_NAME)
