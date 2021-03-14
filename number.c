#include "value.h"
#include "object.h"

double value_number_unwrap(Value* v) {
  return v->primitive->value;
}

Value* value_number_new(double n) {
  Value *v = value_object_create(NULL);

  Primitive *primitive = malloc(sizeof(Primitive));
  primitive->type = PRIMITIVE_NUMBER;
  primitive->value = n;
  v->primitive = primitive;

  return v;
}

Value* value_number_subtract(int size, Value **args) {
  assert_args_size(size, 2);

  Value *left = args[0];
  Value *right = args[1];

  double sum = value_number_unwrap(left) - value_number_unwrap(right);
  Value *number = value_number_new(sum);
  return number;
}

Value* value_number_add(int size, Value **args) {
  assert_args_size(size, 2);

  Value *left = args[0];
  Value *right = args[1];

  double sum = value_number_unwrap(left) + value_number_unwrap(right);

  Value *number = value_number_new(sum);
  return number;
}

Value* value_number_multiply(int size, Value **args) {
  assert_args_size(size, 2);

  Value *left = args[0];
  Value *right = args[1];

  double result = value_number_unwrap(left) * value_number_unwrap(right);
  Value *number = value_number_new(result);

  return number;
}

Value* value_number_divide(int size, Value **args) {
  assert_args_size(size, 2);

  Value *left = args[0];
  Value *right = args[1];

  double result = value_number_unwrap(left) / value_number_unwrap(right);
  Value *number = value_number_new(result);
  return number;
}
