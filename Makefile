CC=cc -c
CFLAGS=-W -Wall -Wextra -pedantic
LD=cc
LDFLAGS=`sdl2-config --libs` -lm

SOURCES:=$(shell find src/ -type f -name '*.c')
OBJECTS=$(SOURCES:src/%.c=obj/%.o)
SUBDIRS=$(dir $(OBJECTS))
EXEC=bin/gbemu

ifeq ($(MODE),)
	MODE = release
endif

ifeq ($(OS),Windows_NT)
	EXEC := $(EXEC).exe
endif

ifeq ($(MODE), debug)
	CFLAGS += -DDEBUG -O0 -g
	LDFLAGS += -DDEBUG -O0 -g
else
	CFLAGS += -DRELEASE -O3 -s -march=native
	LDFLAGS += -DRELEASE -O3 -s -march=native
endif

CFLAGS += -I`pwd`/src

DUMMY := $(shell mkdir -p $(SUBDIRS))

all: $(EXEC)

$(EXEC): bin $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

obj/%.o: src/%.c $(SUBDIRS)
	$(CC) $(CFLAGS) $< -o $@

bin:
	mkdir bin

clean:
	rm -rf bin obj
