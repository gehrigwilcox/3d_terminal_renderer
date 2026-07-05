# ==============================================================================
# terminal_renderer
# ==============================================================================
TARGET    := renderer
SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build

# --- lalib ---
LALIB_DIR := external/lalib
LALIB_INC := $(LALIB_DIR)/include
LALIB_LIB := $(LALIB_DIR)/libla.a

CC       := gcc
CXX      := g++
CFLAGS   := -O2 -march=native -Wall -Wextra -std=c11  -I$(INC_DIR) -I$(LALIB_INC)
CXXFLAGS := -O2 -march=native -Wall -Wextra -std=c++17 -I$(INC_DIR) -I$(LALIB_INC)
LDFLAGS  := -rdynamic
LIBS     := -lm

ifeq ($(DEBUG), 1)
	CFLAGS   += -g -O0 -DDEBUG
	CXXFLAGS += -g -O0 -DDEBUG
else
	CFLAGS   += -DNDEBUG
	CXXFLAGS += -DNDEBUG
endif

# Recursive glob helper: finds files matching a pattern under a dir,
# including subdirectories (so src/lib/*.cpp is picked up same as
# src/*.cpp, with no special-casing of any particular folder name).
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

# scene.c is excluded: it's compiled at runtime by hotreload.c into
# its own .so and dlopen'd. Statically linking it into the main
# binary too would create a duplicate scene_sdf symbol, and because
# LDFLAGS has -rdynamic, the statically-linked copy would silently
# win over the hot-reloaded one -- saving scene.c would appear to
# do nothing.
SRCS_C   := $(filter-out $(SRC_DIR)/scene.c, $(call rwildcard,$(SRC_DIR)/,*.c))
SRCS_CXX := $(call rwildcard,$(SRC_DIR)/,*.cpp)
OBJS_C   := $(patsubst $(SRC_DIR)/%.c,   $(BUILD_DIR)/%.o, $(SRCS_C))
OBJS_CXX := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS_CXX))
OBJS     := $(OBJS_C) $(OBJS_CXX)

.PHONY: all clean clean-all rebuild run print

all: $(LALIB_LIB) $(TARGET)

## Build lalib first if out of date
$(LALIB_LIB):
	$(MAKE) -C $(LALIB_DIR)

## Link
$(TARGET): $(OBJS) $(LALIB_LIB)
ifeq ($(strip $(OBJS_CXX)),)
	$(CC)  $(LDFLAGS) -o $@ $^ $(LIBS)
else
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
endif
	@echo "Built: $@"

# Build C
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Build C++
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

clean-all: clean
	$(MAKE) -C $(LALIB_DIR) clean

rebuild: clean all

print:
	@echo "SRCS_C   = $(SRCS_C)"
	@echo "SRCS_CXX = $(SRCS_CXX)"
	@echo "OBJS     = $(OBJS)"
	@echo "LALIB    = $(LALIB_LIB)"
