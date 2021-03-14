#include "value.h"
#include "object.h"

Value* value_number_new(double n) {
  Value *v = value_object_create(NULL);

  Primitive *primitive = malloc(sizeof(Primitive));
  primitive->type = PRIMITIVE_NUMBER;
  primitive->value = n;
  v->primitive = primitive;

  return v;
}
