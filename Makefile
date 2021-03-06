DIR = build
OBJECTS = $(addprefix $(DIR)/,tokenize.o parse.o value.o hash.o object.o string.o array.o)
CFLAGS = -g
MAIN = $(DIR)/main
TEST = $(DIR)/test

$(MAIN): $(OBJECTS)
$(TEST): $(OBJECTS)

$(DIR)/%.o : %.c $(DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(DIR):
	mkdir -p $(DIR)

.PHONY: all run_test debug_test clean
all: $(MAIN) $(TEST)

run_test: $(TEST)
	$(TEST)

debug_test: $(TEST)
	lldb $(TEST)

clean:
	rm -rf $(DIR)
