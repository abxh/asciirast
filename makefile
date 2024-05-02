name := ascii-engine

CC         = gcc
SRC_PATH   = src
CFLAGS     = -Wall -lm -I$(SRC_PATH)
_SRCS      = main.c screen.c transform.c draw.c misc.c
_SRCS     += objects/star.c
_SRCS     += objects/spiral.c 

SRCS       = $(patsubst %,$(SRC_PATH)/%, $(_SRCS))
OBJS       = $(patsubst %.c, %.o, $(SRCS))

# CFLAGS += -DDEBUG

.PHONY: all clean test

all: $(name)

$(name): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(name)

$(OBJS): $(SRCS)

@(SRCS):
	$(CC) -c $(CFLAGS) $@

clean:
	rm -rf $(OBJS)
	rm -rf $(name)
