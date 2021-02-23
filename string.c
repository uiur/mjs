#include "value.h"
#include "string.h"
#include <string.h>

Value* value_string_new(const char *s) {
  ValueString *v = malloc(sizeof(ValueString));
  v->type = VALUE_STRING;
  v->value = 0;
  v->string = malloc(strlen(s) * sizeof(char));
  strcpy(v->string, s);

  return (Value*)v;
}
