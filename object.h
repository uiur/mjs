Value* value_null_new();
Value* value_undefined_new();
Value* value_object_create(Value *proto);
Value* value_object_new(Binding *binding);
void value_object_set(Value *object, Value *key, Value *value);
Value* value_object_get(Value *object, Value *key);
