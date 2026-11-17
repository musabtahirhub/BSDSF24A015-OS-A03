CC      = gcc
CFLAGS  = -Wall -Iinclude
LDFLAGS =

SRC    = $(wildcard src/*.c)
OBJ    = $(SRC:src/%.c=obj/%.o)
TARGET = bin/myshell

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

obj/%.o: src/%.c include/shell.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj bin

.PHONY: all clean
