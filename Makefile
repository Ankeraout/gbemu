MAKEFLAGS += --no-builtin-rules

MKDIR := mkdir -p
RM := rm -rf
CC := gcc -c
LD := gcc

CFLAGS += -MMD -MP
CFLAGS += -W -Wall -Wextra
CFLAGS += -std=gnu99 -pedantic-errors
CFLAGS += -g3 -O0
CFLAGS += -Iinclude
CFLAGS += `sdl2-config --cflags`
LDFLAGS += -g3 -O0
LIBS += `sdl2-config --libs`

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SOURCES := $(call rwildcard, src, *.c)
OBJECTS := $(patsubst src/%.c, obj/%.o, $(SOURCES))
DIRECTORIES := $(patsubst src/%, obj/%, $(dir $(SOURCES)))
EXECUTABLE := bin/gbemu
DEPENDENCIES := $(patsubst src/%.c, obj/%.c.d, $(SOURCES))

all: dirs $(EXECUTABLE)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

clean:
	$(RM) bin obj

-include $(DEPENDENCIES)

dirs:
	$(MKDIR) bin $(DIRECTORIES)

.PHONY: all clean dirs
