#include "parse.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Value* console_log(int size, Value **args) {
  for (int i = 0; i < size; i++) {
    double n = args[i]->value;
    printf("%.0f\n", n);
  }
}

Value* value_number_subtract(int size, Value **args) {
  if (size != 2) {
    fprintf(stderr, "requires %d arguments, but %d\n", 2, size);
    abort();
  }

  Value *left = args[0];
  Value *right = args[1];

  double sum = left->value - right->value;

  Value *number = malloc(sizeof(Value));
  number->type = VALUE_NUMBER;
  number->value = sum;

  return number;
}

Value* value_number_add(int size, Value **args) {
  if (size != 2) {
    fprintf(stderr, "requires %d arguments, but %d\n", 2, size);
    abort();
  }

  Value *left = args[0];
  Value *right = args[1];

  double sum = left->value + right->value;

  Value *number = malloc(sizeof(Value));
  number->type = VALUE_NUMBER;
  number->value = sum;

  return number;
}

Value* evaluate(Node *node) {
  switch (node->type) {
    case NODE_PROGRAM: {
      for (int i = 0; node->children[i] != NULL; i++) {
        Node *child = node->children[i];
        evaluate(child);
      }

      break;
    }
    case NODE_PRIMITIVE_NUMBER: {
      char *str = node->value;
      Value *number = malloc(sizeof(Value));
      number->type = VALUE_NUMBER;
      number->value = (double)atoi(str);
      return number;
    }

    case NODE_BINARY_OPERATOR:
    case NODE_FUNCTION_CALL: {
      char *identifier = node->value;

      int size = 0;
      while(node->children[size] != NULL) size++;

      Value **args = malloc(size * sizeof(Value));
      for (int i = 0; node->children[i] != NULL; i++) {
        args[i] = evaluate(node->children[i]);
      }

      if (strcmp(identifier, "log") == 0) {
        return console_log(size, args);
      }

      if (strcmp(identifier, "+") == 0) {
        return value_number_add(size, args);
      }

      if (strcmp(identifier, "-") == 0) {
        return value_number_subtract(size, args);
      }

      fprintf(stderr, "runtime error: `%s` is not defined\n", identifier);

      break;
    }

    default:
      fprintf(stderr, "unexpected node type: %d\n", node->type);
      abort();
      break;
    }

  return NULL;
}
