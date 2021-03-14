#include "value.h"
#include "object.h"
#include "string.h"
#include <string.h>

Value* value_string_new(const char *s) {
  Value *v = value_object_create(NULL);
  PrimitiveString *primitive = malloc(sizeof(PrimitiveString));
  primitive->type = PRIMITIVE_STRING;
  primitive->value = 0;

  primitive->string = malloc(strlen(s) * sizeof(char));
  strcpy(primitive->string, s);

  v->primitive = (Primitive*)primitive;

  return v;
}

const char* value_string_unwrap(Value *v) {
  if (v->primitive != NULL && v->primitive->type == PRIMITIVE_STRING) {
    PrimitiveString *s = (PrimitiveString*)v->primitive;
    return s->string;
  }

  return NULL;
}
