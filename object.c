#include "hash.h"
#include "value.h"
#include "object.h"
#include "string.h"
#include "array.h"
#include <stdlib.h>

Value* value_null_new() {
  Value *v = malloc(sizeof(Value));
  v->kind = VALUE_KIND_NULL;
  v->table = NULL;
  v->proto = NULL;
  return v;
}

Value* value_undefined_new() {
  Value *v = malloc(sizeof(Value));
  v->kind = VALUE_KIND_UNDEFINED;
  v->table = NULL;
  v->proto = NULL;
  return v;
}

Value* value_object_new() {
  Value *v = malloc(sizeof(Value));
  v->kind = VALUE_KIND_OBJECT;
  v->table = hash_table_new();
  v->proto = NULL;
  return v;
}

void value_object_set(Value *object, Value *key, Value *value) {
  if (object->primitive != NULL && object->primitive->type == PRIMITIVE_ARRAY) {
    value_array_set(object, key, value);
    return;
  }
  hash_table_set(object->table, value_string_unwrap(key), value);
}

Value* value_object_get(Value *object, Value *key) {
  if (object->primitive != NULL && object->primitive->type == PRIMITIVE_ARRAY) {
    Value *v = value_array_get(object, key);
    if (v != NULL) return v;
  }

  const char *s = value_string_unwrap(key);
  if (s == NULL) return value_undefined_new();

  Value *v = hash_table_get(object->table, s);
  if (v == NULL) {
    return value_undefined_new();
  }
  return v;
}
