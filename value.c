#include "tokenize.h"
#include "parse.h"
#include "value.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Env {
  struct HashTable *table;
  struct Env *parent;
} Env;

Env* env_new(Env *parent) {
  Env *env = malloc(sizeof(Env));
  env->table = hash_table_new();
  env->parent = parent;
  return env;
}

Value* env_get(Env *env, const char *key)  {
  if (env == NULL) return NULL;

  Value* value = hash_table_get(env->table, key);
  if (value != NULL) return value;

  return env_get(env->parent, key);
}

Value* value_new(ValueType type) {
  Value *v = malloc(sizeof(Value));
  v->type = VALUE_NUMBER;
  v->value = 0;
  return v;
}

Value* value_number_new(double n) {
  Value *number = malloc(sizeof(Value));
  number->type = VALUE_NUMBER;
  number->value = n;

  return number;
}

int value_is_truthy(Value *value) {
  if (value->type == VALUE_NUMBER) {
    double n = value->value;
    return n != 0;
  }

  return 1;
}

Value* console_log(int size, Value **args) {
  for (int i = 0; i < size; i++) {
    Value *v = args[i];
    if (v == NULL) {
      fprintf(stderr, "log error: unexpected null\n");
      abort();
    }

    if (v->type == VALUE_NUMBER) {
      double n = v->value;
      printf("%.0f\n", n);
    } else {
      fprintf(stderr, "log error: %d\n", v->type);
      abort();
    }
  }
  return NULL;
}

void assert_args_size(int size, int expected) {
  if (size != expected) {
    fprintf(stderr, "requires %d arguments, but %d\n", expected, size);
    abort();
  }
}

Value* value_number_subtract(int size, Value **args) {
  assert_args_size(size, 2);

  Value *left = args[0];
  Value *right = args[1];

  double sum = left->value - right->value;
  Value *number = value_number_new(sum);
  return number;
}

Value* value_number_add(int size, Value **args) {
  assert_args_size(size, 2);

  Value *left = args[0];
  Value *right = args[1];

  double sum = left->value + right->value;

  Value *number = value_number_new(sum);
  return number;
}

Value* value_number_multiply(int size, Value **args) {
  assert_args_size(size, 2);

  Value *left = args[0];
  Value *right = args[1];

  double result = left->value * right->value;
  Value *number = value_number_new(result);

  return number;
}

Value* value_number_divide(int size, Value **args) {
  assert_args_size(size, 2);

  Value *left = args[0];
  Value *right = args[1];

  double result = left->value / right->value;
  Value *number = value_number_new(result);
  return number;
}

Value* evaluate_node(Node *node, Env *env);

typedef struct CallContext {
  int returned;
} CallContext;

CallContext context;
CallContext *ctx = &context;

Value* evaluate_node_children(Node *node, Env *env) {
  Value *result = NULL;
  for (int i = 0; node->children[i] != NULL; i++) {
    Node *child = node->children[i];

    Value *value = evaluate_node(child, env);
    if (ctx->returned) {
      result = value;
      break;
    }
  }

  return result;
}

Value* evaluate_function_call(Value *value, Value **args, int size, Env *env) {
  Node *node = value->node;
  ctx->returned = 0;

  Env *function_env = env_new(env);

  for (int i = 0; i < size; i++) {
    Node *arg = value->node->args[i];
    hash_table_set(function_env->table, arg->value, args[i]);
  }

  Value *result = evaluate_node_children(node, function_env);
  return result;
}

Value* evaluate_node(Node *node, Env *env) {
  switch (node->type) {
    case NODE_PROGRAM: {
      for (int i = 0; node->children[i] != NULL; i++) {
        Node *child = node->children[i];
        evaluate_node(child, env);
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

    case NODE_IDENTIFIER: {
      Value *value = env_get(env, node->value);
      return value;
    }

    case NODE_VAR_DECLARATION: {
      // env

      break;
    }

    case NODE_VAR_ASSIGNMENT: {
      Node *identifier = node->children[0];
      Node *right = node->children[1];

      hash_table_set(env->table, identifier->value, evaluate_node(right, env));

      break;
    }

    case NODE_FUNCTION_DECLARATION: {
      Value *function_value = malloc(sizeof(Value));
      function_value->type = VALUE_FUNCTION;
      function_value->node = node;

      hash_table_set(env->table, node->value, function_value);

      break;
    }

    case NODE_STATEMENT_RETURN: {
      Value *value = evaluate_node(node->children[0], env);
      ctx->returned = 1;
      return value;
    }

    case NODE_STATEMENT_IF: {
      Value *condition = evaluate_node(node->args[0], env);
      if (value_is_truthy(condition)) {
        Value *result = evaluate_node_children(node, env);
        return result;
      }

      return NULL;
    }

    case NODE_BINARY_OPERATOR:
    case NODE_FUNCTION_CALL: {
      char *identifier = node->value;

      int size = 0;
      while(node->children[size] != NULL) size++;

      Value **args = malloc(size * sizeof(Value));
      for (int i = 0; node->children[i] != NULL; i++) {
        args[i] = evaluate_node(node->children[i], env);
      }

      Value *value = env_get(env, identifier);
      if (value != NULL && value->type == VALUE_FUNCTION) {
        Value *return_value = evaluate_function_call(value, args, size, env);
        return return_value;
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

      if (strcmp(identifier, "*") == 0) {
        return value_number_multiply(size, args);
      }

      if (strcmp(identifier, "/") == 0) {
        return value_number_divide(size, args);
      }

      fprintf(stderr, "runtime error: `%s` is not defined\n", identifier);
      abort();

      break;
    }

    case NODE_PRIMITIVE_UNDEFINED: {
      return value_new(VALUE_UNDEFINED);
    }

    case NODE_PRIMITIVE_NULL: {
      return value_new(VALUE_NULL);
    }

    case NODE_PRIMITIVE_BOOLEAN: {
      Value *v = value_new(VALUE_BOOLEAN);
      if (strcmp(node->value, "true") == 0) {
        v->value = 1;
      } else if (strcmp(node->value, "false") == 0) {
        v->value = 0;
      } else {
        fprintf(stderr, "runtime error: unexpected value for boolean: %s\n", node->value);
        abort();
      }
      return v;
    }

    default:
      fprintf(stderr, "runtime error: unexpected node type: %s\n", NodeTypeString[node->type]);
      abort();
      break;
    }

  return NULL;

}

Value* evaluate(Node *node) {
  Env *global = env_new(NULL);
  return evaluate_node(node, global);
}
