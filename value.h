#ifndef MJS_VALUE_H
#define MJS_VALUE_H

#include "parse.h"
#define VALUE_ENUM(M) \
  M(VALUE_NUMBER) \
  M(VALUE_STRING) \
  M(VALUE_OBJECT) \
  M(VALUE_FUNCTION) \
  M(VALUE_BOOLEAN) \
  M(VALUE_NULL) \
  M(VALUE_UNDEFINED)
#define VALUE_ENUM_TO_ENUM(X) X,
#define VALUE_ENUM_TO_STRING(X) #X,

typedef enum ValueType {
  VALUE_ENUM(VALUE_ENUM_TO_ENUM)
} ValueType;

static const char *ValueTypeString[] = {
  NODE_ENUM(VALUE_ENUM_TO_STRING)
};

#define VALUE_COMMON \
  ValueType type; \
  double value

typedef struct Value {
  VALUE_COMMON;
} Value;

typedef struct ValueObject {
  VALUE_COMMON;
  struct HashTable *table;
} ValueObject;

typedef struct ValueString {
  VALUE_COMMON;
  char *string;
} ValueString;

typedef struct ValueFunction {
  VALUE_COMMON;
  char *name;
  struct Node *node;
} ValueFunction;

Value* evaluate(Node *node);
Value* value_undefined_new();

#endif
