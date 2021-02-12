typedef enum ValueType {
  VALUE_NUMBER,
  VALUE_FUNCTION,
} ValueType;

typedef struct Value {
  ValueType type;
  double value;
  struct Node *node;
} Value;

Value* evaluate(Node *node);
