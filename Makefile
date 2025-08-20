CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -Itest

# include dirs: always include root test, plus any subproject include/ directories
INCLUDE_DIRS := test $(wildcard */include)
CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

# sources: runner + framework + all subproject src/*.c and test/*.c
SRCS := main.c test/test.c $(wildcard */src/*.c) $(wildcard */test/*.c)

# objects
OBJS := $(SRCS:.c=.o)

# binary name
BIN := tests

.PHONY: all build run clean

all: build

build: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

run: $(BIN)
	./$(BIN)

# compile rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(BIN)
