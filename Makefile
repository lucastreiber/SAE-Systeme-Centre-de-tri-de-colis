CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread
LDFLAGS = -pthread

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
EXEC = centre_tri

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -MMD -c $< -o $@

build:
	mkdir -p build

-include $(OBJ:.o=.d)

clean:
	rm -rf build $(EXEC) journal.log

.PHONY: all clean
