
#include "tokenize.h"
#include "parse.h"
#include "value.h"
#include "hash.h"
#include "assert.h"
#include <stdio.h>
#include <string.h>

void eval(char *source) {
  printf("input: `%s`\n", source);
  Token *token = tokenize(source);
  token_pp(token);
  Node *node = parse(token);
  node_pp(node);
  printf("\n");
  evaluate(node);
  printf("---\n");
}

void test_hash() {
  printf("test_hash\n");
  HashTable *hash = hash_table_new();
  char *value = hash_table_get(hash, "foo");
  assert(value == NULL);

  hash_table_set(hash, "foo", "bar");
  value = hash_table_get(hash, "foo");
  assert(strcmp(value, "bar") == 0);

  hash_table_set(hash, "foo", "bar2");
  value = hash_table_get(hash, "foo");
  assert(strcmp(value, "bar2") == 0);

  // resize test
  for (int i = 0; i < 100; i++) {
    char ch = 'a' + i;
    char str[] = { ch, '\0' };
    hash_table_set(hash, str, str);
  }

  assert(hash->used > 100);
  assert(hash->cap > 100);

  printf("---\n");
}

void test_example() {
  eval("1;");
  eval("1 + 2;");
  eval("1 * 2;");
  eval("1 * 2 + 3;");
  eval("1 * (2 + 3);");
  eval("log(1);");
  eval("log(1); log(2);");
  eval("log(1 + 2);");
  eval("var foo; foo = 1 + 2; log(foo);");

  eval("function add(a, b) { return a + b; } log(add(1, 2));");
}

int main(int argc, char const **argv) {
  test_hash();
  test_example();
  return 0;
}
