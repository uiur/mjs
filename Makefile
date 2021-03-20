DIR = build
OBJECTS = $(addprefix $(DIR)/,tokenize.o parse.o value.o hash.o object.o boolean.o number.o string.o function.o array.o inspect.o)
TESTS = $(addprefix $(DIR)/,eval_test hash_test)
CFLAGS = -g
MAIN = $(DIR)/main

$(MAIN): $(OBJECTS)

$(DIR)/%_test: %_test.o $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(DIR)/%.o : %.c $(DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(DIR):
	mkdir -p $(DIR)

.PHONY: all test test_run clean
all: $(MAIN) $(TESTS)
test: $(TESTS)
test_run: test
	./test.sh

clean:
	rm -rf $(DIR)
