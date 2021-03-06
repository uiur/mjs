#include "value.h"
#include "array.h"
#include <stdlib.h>
#include <string.h>

Value* value_array_new() {
  ValueArray *v = malloc(sizeof(ValueArray));
  v->type = VALUE_ARRAY;
  v->cap = 10;
  v->size = 0;

  v->values = malloc(v->cap * sizeof(Value*));
  memset(v->values, (int)NULL, v->cap * sizeof(Value*));
  return (Value*)v;
}

Value* value_array_get(ValueArray *array, Value *index) {
  int i = (int)index->value;
  return array->values[i];
}

void value_array_resize(ValueArray *array, unsigned int new_cap) {
  array->cap = new_cap;
  array->values = realloc(array->values, array->cap * sizeof(Value*));
  for (unsigned int i = array->size; i < array->cap; i++) {
    array->values[i] = NULL;
  }
}

void value_array_resize_if_needed(ValueArray *array) {
  if (array->cap <= array->size * 2) {
    value_array_resize(array, array->cap * 2);
  }
}

void value_array_set(ValueArray *array, Value *index, Value *value) {
  int i = (int)index->value;
  array->values[i] = value;

  if (i + 1 >= array->size) {
    array->size = i + 1;
    value_array_resize_if_needed(array);
  }
}

Value* value_array_length(ValueArray *array) {
  return value_number_new((double)array->size);
}
