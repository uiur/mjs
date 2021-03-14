#include "value.h"
#include "object.h"
#include "number.h"
#include "array.h"
#include <stdlib.h>
#include <string.h>

char* value_inspect(Value *v) {
  char *buf = malloc(100 * sizeof(char));
  if (v->primitive == NULL) {
    switch (v->kind) {
      case VALUE_KIND_NULL: {
        return "null";
      }

      case VALUE_KIND_UNDEFINED: {
        return "undefined";
      }

      default: {
        return NULL;
      }
    }
  } else {
    switch (v->primitive->type) {
      case PRIMITIVE_NUMBER: {
        double n = value_number_unwrap(v);
        sprintf(buf, "%.0f", n);
        return buf;
      }

      case PRIMITIVE_BOOLEAN: {
        double n = value_number_unwrap(v);
        if (n == 1) {
          return "true";
        } else {
          return "false";
        }
      }

      case PRIMITIVE_STRING: {
        PrimitiveString *vs = (PrimitiveString*)v->primitive;
        return vs->string;
      }

      case PRIMITIVE_ARRAY: {
        strcat(buf, "[");
        for (int i = 0; i < value_number_unwrap(value_array_length(v)); i++) {
          if (i > 0) {
            strcat(buf, ", ");
          }
          const char *s = value_inspect(value_array_get(v, value_number_new(i)));
          strcat(buf, s);
        }

        strcat(buf, "]");
        return buf;
      }

      case PRIMITIVE_FUNCTION: {
        return "function";
      }

      default: {
        return NULL;
      }
    }
  }

  return NULL;
}
