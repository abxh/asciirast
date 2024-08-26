LIB_FILE_FP := acrast.so
SRC_DIR     := ./acrast

CC        := gcc
CFLAGS    += -Wall -Wextra -Wshadow -Wconversion -pedantic -std=c2x
CFLAGS    += -I./$(SRC_DIR)

LD_FLAGS  += -lm

SRC_FILES  := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES  := $(SRC_FILES:.c=.o)

.PHONY: all

all: $(LIB_FILE_FP)

$(LIB_FILE_FP): $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -fPIC $^ -shared -o $(LIB_FILE_FP)

$(OBJ_FILES): $(SRC_FILES)

$(SRC_FILES):
	$(CC) -c $(CFLAGS) $@

.PHONY: clean

clean:
	rm -rf $(OBJ_FILES)
	rm -rf $(LIB_FILE_FP)
