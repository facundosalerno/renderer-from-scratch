CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
TARGET  = build/renderer

SRCS    = $(wildcard src/*.c)
OBJS    = $(SRCS:src/%.c=build/%.o)
DEPS    = $(SRCS:src/%.c=build/%.d)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS) | build
	$(CC) $(CFLAGS) -o $@ $^ -lm

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build