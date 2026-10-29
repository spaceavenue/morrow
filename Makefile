CC       ?= cc
BUILD    ?= build
WARN     := -Wall -Werror -Wextra -Wpedantic -Wshadow -Wstrict-prototypes \
             -Wmissing-prototypes

CFLAGS    ?= -O2 -g -std=c11 $(WARN)
CPPFLAGS  += -Iinclude -D_GNU_SOURCE
LDFLAGS   += -fsanitize=address,undefined -fno-omit-frame-pointer
LDLIBS    += -pthread

SRC  := src/store.c
OBJ  := $(SRC:src/%.c=$(BUILD)/obj/%.o)
DEP  := $(OBJ:.o=.d)
LIB  := $(BUILD)/libmorrow.a


.PHONY: all lib clean help
.DEFAULT_GOAL := all

all: lib

lib: $(LIB)

$(LIB): $(OBJ)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(BUILD)/obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf build

help:
	@echo "make            build $(LIB)"
	@echo "make clean"

-include $(DEP)
