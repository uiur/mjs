#include "value.h"
#include "object.h"
#include "array.h"
#include "number.h"
#include "string.h"
#include <stdlib.h>
#include <string.h>

#define NUMBER_UNWRAP(X) ((X)->primitive->value)
#define ARRAY_UNWRAP(X) ((PrimitiveArray*)(X)->primitive)

Value* value_array_new(Binding *binding) {
  Value *v = value_object_new(binding);

  PrimitiveArray *a = malloc(sizeof(PrimitiveArray));
  a->type = PRIMITIVE_ARRAY;
  a->cap = 10;
  a->size = 0;

  a->values = malloc(a->cap * sizeof(Value*));
  memset(a->values, (int)NULL, a->cap * sizeof(Value*));
  v->primitive = (Primitive*)a;

  return v;
}

Value* value_array_get(Value *v, Value *index) {
  PrimitiveArray *array = ARRAY_UNWRAP(v);
  int i = (int)NUMBER_UNWRAP(index);
  return array->values[i];
}

void value_array_resize(PrimitiveArray *array, unsigned int new_cap) {
  array->cap = new_cap;
  array->values = realloc(array->values, array->cap * sizeof(Primitive*));
  for (unsigned int i = array->size; i < array->cap; i++) {
    array->values[i] = NULL;
  }
}

void value_array_resize_if_needed(PrimitiveArray *array) {
  if (array->cap <= array->size * 2) {
    value_array_resize(array, array->cap * 2);
  }
}

void value_array_set(Value *v, Value *index, Value *value) {
  PrimitiveArray *array = ARRAY_UNWRAP(v);
  int i = NUMBER_UNWRAP(index);
  array->values[i] = value;

  if (i + 1 >= array->size) {
    array->size = i + 1;
    value_array_resize_if_needed(array);
  }
}

Value* value_array_length(Value *v) {
  return value_number_new((double)ARRAY_UNWRAP(v)->size);
}
