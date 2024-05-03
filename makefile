name := ascii-engine

CC         = gcc
SRC_PATH   = src
CFLAGS     = -Wall -lm -I$(SRC_PATH)
_SRCS      = screen.c transform.c draw.c misc.c
_SRCS     += main.c
_SRCS     += scenes/star.c
_SRCS     += scenes/spiral.c 

SRCS       = $(patsubst %,$(SRC_PATH)/%, $(_SRCS))
OBJS       = $(patsubst %.c, %.o, $(SRCS))

CFLAGS += -DDEBUG

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
