typedef enum ValueType {
  VALUE_NUMBER,
  VALUE_FUNCTION,
  VALUE_BOOLEAN,
  VALUE_NULL,
  VALUE_UNDEFINED
} ValueType;

typedef struct Value {
  ValueType type;
  double value;
  struct Node *node;
} Value;

Value* evaluate(Node *node);
