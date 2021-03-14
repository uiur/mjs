#ifndef MJS_VALUE_H
#define MJS_VALUE_H

#include "parse.h"
#define PRIMITIVE_ENUM(M) \
  M(PRIMITIVE_NUMBER) \
  M(PRIMITIVE_STRING) \
  M(PRIMITIVE_ARRAY) \
  M(PRIMITIVE_FUNCTION) \
  M(PRIMITIVE_BOOLEAN)

#define PRIMITIVE_ENUM_TO_ENUM(X) X,
#define PRIMITIVE_ENUM_TO_STRING(X) #X,

typedef enum PrimitiveType {
  PRIMITIVE_ENUM(PRIMITIVE_ENUM_TO_ENUM)
} PrimitiveType;

static const char *PrimitiveTypeString[] = {
  NODE_ENUM(PRIMITIVE_ENUM_TO_STRING)
};

#define PRIMITIVE_COMMON \
  PrimitiveType type; \
  double value

typedef struct Primitive {
  PRIMITIVE_COMMON;
} Primitive;

typedef struct PrimitiveArray {
  PRIMITIVE_COMMON;
  unsigned int cap;
  unsigned int size;
  struct Value** values;
} PrimitiveArray;

typedef struct PrimitiveString {
  PRIMITIVE_COMMON;
  char *string;
} PrimitiveString;


typedef struct Value* (NativeFunction)(int, struct Value**);
typedef struct PrimitiveFunction {
  PRIMITIVE_COMMON;
  char *name;
  struct Node *node;
  NativeFunction *fn;
} PrimitiveFunction;

typedef enum ValueKind {
  VALUE_KIND_NULL,
  VALUE_KIND_UNDEFINED,
  VALUE_KIND_OBJECT
} ValueKind;

typedef struct Value {
  ValueKind kind;
  struct Primitive *primitive;
  struct HashTable *table;
  struct Value *proto;
} Value;

Value* evaluate(Node *node);
Value* value_number_new();

typedef struct Binding {
  struct Value *object_prototype;
} Binding;

#endif
