CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
TARGET  = build/renderer

SRCS    = $(wildcard *.c)
OBJS    = $(SRCS:%.c=build/%.o)
DEPS    = $(SRCS:%.c=build/%.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) | build
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: %.c | build
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

build:
	mkdir -p build

clean:
	rm -rf build