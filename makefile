name := ascii-3d

CC         = gcc
CFLAGS     = -Wall -lm
SRC_PATH   = src
_SRCS      = transform.c draw.c screen.c main.c
SRCS       = $(patsubst %,$(SRC_PATH)/%, $(_SRCS))
OBJS       = $(patsubst %.c, %.o, $(SRCS))

.PHONY: all clean test

all: $(name)

$(name): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(name)

$(OBJS): $(SRCS)

$(SRCS):
	$(CC) -c $(CFLAGS) $@

clean:
	rm -rf $(OBJS)
	rm -rf $(name)
