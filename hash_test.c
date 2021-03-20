#include "hash.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_hash() {
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
}

int main(int argc, char const **argv) {
  test_hash();
  return 0;
}
