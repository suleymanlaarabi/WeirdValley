CC := gcc
CFLAGS := -ObjC -framework Cocoa -framework Metal -framework QuartzCore -framework MetalKit \
	-Wall -Wextra -Wpedantic -Iinclude -Iinclude/External -DFLECS_NO_CPP

SRC_DIR := src
OBJ_DIR := build

SRC := $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET := ./build/main

all: $(TARGET)

run: all
	@$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

perf: CFLAGS += -Ofast -march=native -mtune=native -flto=thin -fstrict-aliasing \
	-fvectorize -fslp-vectorize -funroll-loops -fomit-frame-pointer \
	-fno-stack-protector -fno-unwind-tables \
	-fdata-sections -ffunction-sections -fno-trapping-math -fno-math-errno \
	-freciprocal-math -fmerge-all-constants -DNDEBUG

perf: LDFLAGS += -flto=thin -Wl,-dead_strip
perf: re

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

fclean: clean

re: fclean all

.PHONY: all clean fclean re run perf
