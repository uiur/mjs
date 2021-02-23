#include "hash.h"
#include "value.h"
#include "object.h"
#include <stdlib.h>

Value* value_object_new() {
  ValueObject *v = malloc(sizeof(ValueObject));
  v->type = VALUE_OBJECT;
  v->value = 0;
  v->table = hash_table_new();

  return (Value*)v;
}

void value_object_set(ValueObject *object, ValueString *key, Value *value) {
  hash_table_set(object->table, key->string, value);
}
