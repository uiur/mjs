#include "value.h"
#include "object.h"
#include <stdlib.h>

Value* value_function_new(Node *node) {
  Value *v = value_object_create(NULL);

  PrimitiveFunction *function_value = malloc(sizeof(PrimitiveFunction));
  function_value->type = PRIMITIVE_FUNCTION;
  function_value->node = node;
  function_value->fn = NULL;
  if (node != NULL) {
    function_value->name = node->value;
  } else {
    function_value->name = "";
  }

  v->primitive = (Primitive*)function_value;

  return v;
}

Value* value_function_native_new(NativeFunction *fn) {
  Value *v = value_function_new(NULL);
  ((PrimitiveFunction*)v->primitive)->fn = fn;
  return v;
}
