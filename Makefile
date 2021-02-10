OBJECTS = parse.o value.o

main: CFLAGS = -g
main: main.c $(OBJECTS)
test: test.c $(OBJECTS)
parse.o: parse.c
value.o: value.c
