# ==============================
# Configuration
# ==============================

PROJECT      := ilo
CC           := gcc
CSTD         := -std=c++23

WARNINGS     := -Wall
WARNINGS     += -Wextra
WARNINGS     += -Werror
WARNINGS     += -Wpedantic
WARNINGS     += -Wshadow

CFLAGS       := $(CSTD) $(WARNINGS) -g
INCLUDES     := -Iinclude

SRC_DIR      := src
TEST_DIR     := tests
BUILD_DIR    := build

CLANG_FORMAT := clang-format
CLANG_TIDY   := clang-tidy

# ==============================
# Sources
# ==============================

SRC_FILES    := $(shell find $(SRC_DIR) -name "*.cpp")
TEST_FILES   := $(shell find $(TEST_DIR) -name "*.cpp")

SRC_NO_MAIN := $(filter-out $(SRC_DIR)/main.c,$(SRC_FILES))
OBJ_NO_MAIN := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_NO_MAIN))

OBJ_FILES    := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))
TEST_OBJS    := $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/tests_%.o,$(TEST_FILES))

TARGET       := $(BUILD_DIR)/$(PROJECT)

# ==============================
# Default target
# ==============================

.PHONY: all
all: $(TARGET)

# ==============================
# Build main binary
# ==============================

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $@

# ==============================
# Tests
# ==============================

.PHONY: test
test: $(OBJ_NO_MAIN) $(TEST_OBJS)
	$(CC) $(OBJ_NO_MAIN) $(TEST_OBJS) -o $(BUILD_DIR)/run_tests
	@echo "Running tests..."
	@./$(BUILD_DIR)/run_tests

$(BUILD_DIR)/tests_%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ==============================
# Format
# ==============================

.PHONY: format
format:
	@echo "Formatting..."
	@find $(SRC_DIR) include $(TEST_DIR) -name "*.cpp" -o -name "*.hpp" | \
	xargs $(CLANG_FORMAT) -i

# ==============================
# Tidy
# ==============================

.PHONY: tidy
tidy:
	@echo "Running clang-tidy..."
	@for file in $(SRC_FILES); do \
		$(CLANG_TIDY) $$file -- $(INCLUDES) $(CFLAGS) || exit 1; \
	done

# ==============================
# Clean
# ==============================

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# ==============================
# Rebuild
# ==============================

.PHONY: re
re: clean all
