typedef enum ValueType {
  VALUE_NUMBER,
} ValueType;

typedef struct Value {
  ValueType type;
  double value;
} Value;

Value* evaluate(Node *node);
