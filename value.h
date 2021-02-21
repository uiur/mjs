#define VALUE_ENUM(M) \
  M(VALUE_NUMBER) \
  M(VALUE_STRING) \
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

typedef struct Value {
  ValueType type;
  double value;
  struct Node *node;
} Value;

typedef struct ValueString {
  ValueType type;
  double value;
  struct Node *node;
  char *string;
} ValueString;

Value* evaluate(Node *node);
