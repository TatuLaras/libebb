# Used for building and running unit test, usually compilation of this library is handled by the includer.
NAME = noble
BUILD_DIR = build
BUILD_DIR_TESTS = build/tests
SRC_DIR = src
SRC_DIR_TESTS = test
UNITY_DIR = external/unity

EXTERNAL_INCLUDE = -I$(LIBEBB)/src -Iexternal


CC = gcc
PACKAGES = $(shell pkg-config --libs raylib opengl) -lm
SANITIZE = -fsanitize=address
CFLAGS = $(PACKAGES) $(EXTERNAL_INCLUDE) -Wall -Wextra -Wshadow -pedantic -Wstrict-prototypes -march=native
CFLAGS_TEST = $(PACKAGES) -DTEST -I$(UNITY_DIR) -I$(SRC_DIR) $(EXTERNAL_INCLUDE) -ggdb $(SANITIZE)

CFLAGS_DEBUG = $(CFLAGS) -DDEBUG -ggdb
CFLAGS_ASAN = $(CFLAGS) -DDEBUG $(SANITIZE)
CFLAGS_RELEASE = $(CFLAGS) -DNDEBUG -Ofast

# Arguments to append to the program run with "make run"
ARGS = 

$(BUILD_DIR_TESTS):
	mkdir -p $(BUILD_DIR_TESTS)

# Build and run tests
SRC = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(LIBEBB)/src/*.c)
TEST_IGNORE = $(SRC_DIR)/main.c $(SRC_DIR)/model_files.c $(SRC_DIR)/game_interface.c
SRC_FOR_TESTS = $(filter-out $(TEST_IGNORE), $(SRC)) $(wildcard $(UNITY_DIR)/*.c)
OBJS_TESTS = $(patsubst $(SRC_DIR_TESTS)/%.c, $(BUILD_DIR_TESTS)/%.o, $(wildcard $(SRC_DIR_TESTS)/test_*.c))

test: $(BUILD_DIR_TESTS) run_tests
	@echo

NOOP=
SPACE = $(NOOP) $(NOOP)

run_tests: $(OBJS_TESTS)
	@echo -e "\n\n--------------\n Test results\n--------------\n"
	@$(subst $(SPACE), && echo -e "\n" && ,$^)

$(OBJS_TESTS): $(BUILD_DIR_TESTS)/%.o: $(SRC_DIR_TESTS)/%.c $(SRC_FOR_TESTS)
	@echo -e "\nBuilding $@"
	$(CC) -o $@ $^ $(CFLAGS_TEST)

clean:
	rm -rf $(BUILD_DIR)

