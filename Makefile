OBJECTS = parse.o

main: CFLAGS = -g
main: main.c $(OBJECTS)
parse.o: parse.c
