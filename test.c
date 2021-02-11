
#include "parse.h"
#include "value.h"
#include <stdio.h>

void eval(char *source) {
  Token *token = tokenize(source);
  Node *node = parse(token);
  node_pp(node);
  printf("\n");
  evaluate(node);
}

void test_example() {
  eval("1 + 2;");
  eval("log(1);");
  eval("log(1); log(2);");
  eval("log(1 + 2);");
  eval("var foo;");
  eval("foo = 1");
}

int main(int argc, char const **argv) {
  test_example();
  return 0;
}
