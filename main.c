#include "parse.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int main(int argc, char const **argv) {
  char *source = "log(2 + 3 + 3 - 2);";
  Token *token = tokenize(source);
  token_pp(token);
  Node *node = parse(token);
  node_pp(node);
  printf("\n");
  evaluate(node);

  return 0;
}
