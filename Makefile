OBJECTS = parse.o value.o
CFLAGS = -g

main: main.c $(OBJECTS)
test: test.c $(OBJECTS)
parse.o: parse.c
value.o: value.c

.PHONY: run_test debug_test clean
run_test: test
	./test

debug_test: test
	lldb ./test

clean:
	rm -rf *.o ./main ./test
