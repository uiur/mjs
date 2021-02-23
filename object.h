typedef struct ValueObject {
  ValueType type;
  double value;
  struct Node *node;

  struct HashTable *table;
} ValueObject;

Value* value_object_new();
void value_object_set(ValueObject *object, ValueString *key, Value *value);
