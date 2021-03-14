
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
  printf("\n");
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
  eval("undefined;");
  eval("null;");
  eval("true;");
  eval("false;");
  eval("'foobar';");

  eval("1 + 2;");
  eval("1 * 2;");
  eval("1 + 2 * 3;");
  eval("3 - 2 - 1;");
  eval("1 * (2 + 3);");
  eval("1 === 2;");
  eval("1 > 2;");
  eval("1 < 2;");

  eval("{};");
  eval("{ foo: 1, bar: 2 };");

  eval("[];");
  eval("[0, 1, 2];");

  eval("console.log(1);");
  eval("console.log(1); console.log(2);");
  eval("console.log(1 + 2);");

  eval("var foo;");
  eval("var foo = 1;");
  eval("var foo = [1, 2, 3];");
  eval("var foo; var bar; foo = bar = 1;");
  eval("var foo; foo = 1 + 2; console.log(foo);");
  eval("var foo; console.log(foo);");
  eval("var s; s = 'foo'; console.log(s);");

  eval("var o; o = { foo: 1 }; console.log(o.foo); ");
  eval("var o = {}; o.foo = 1;");
  eval("var a = [1]; a[0];");
  eval("var a = [[0, 1]]; a[0][1];");
  eval("var a = []; a[0] = 0;");

  eval("function add(a, b) { return a + b; } console.log(add(1, 2));");
  eval("function add(a, b) { return a + b; } var o; o = { plus: add }; console.log(o.plus(1, 2));");
  // eval("var f = function (n) { return n }; f(42);");
  eval("if (1) { console.log(1); }");
  eval("if (1 < 2) { console.log(1 < 2); }");
  eval("for (var i = 0; i < 5; i = i + 1) {}");

  eval("var o = { a: 1, b: 2 }; function f() { return this.a + this.b; } o.f = f; console.log(o.f());");
  eval("var a = [1, 2]; console.log(a.length());");
}

int main(int argc, char const **argv) {
  test_hash();
  test_example();
  return 0;
}
