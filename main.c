#include "tokenize.h"
#include "parse.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* read_source(FILE *fp) {
  int cap = 1024;
  char *buf = malloc(cap * sizeof(char));
  int i = 0;
  char c;
  while ((c = fgetc(fp)) != EOF) {
    if (i + 1 == cap) {
      cap *= 2;
      buf = realloc(buf, cap * sizeof(char));
    }

    buf[i++] = c;
  }
  buf[i] = '\0';

  return buf;
}

int main(int argc, char const **argv) {
  char *source;
  if (argc < 2) {
    source = read_source(stdin);
  } else {
    const char *file_name = argv[1];
    FILE *fp = fopen(file_name, "r");
    if(!fp) {
      perror("File opening failed");
      return EXIT_FAILURE;
    }
    source = read_source(fp);
    fclose(fp);
  }

  Token *token = tokenize(source);
  Node *node = parse(token);
  evaluate(node);

  return 0;
}
