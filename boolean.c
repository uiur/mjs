#include "value.h"
#include "object.h"

Primitive* primitive_boolean_init(int value) {
  Primitive *primitive = malloc(sizeof(Primitive));
  primitive->type = PRIMITIVE_BOOLEAN;
  primitive->value = value;
  return primitive;
}

Value* value_true_new() {
  Value *v = value_object_create(NULL);
  v->primitive = primitive_boolean_init(1);
  return v;
}

Value* value_false_new() {
  Value *v = value_object_create(NULL);
  v->primitive = primitive_boolean_init(0);
  return v;
}
