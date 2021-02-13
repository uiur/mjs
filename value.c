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
Value* evaluate_node(Node *node, Env *env);

Value* evaluate_function_call(Value *value, Value **args, int size, Env *env) {
  Node *node = value->node;

  Env *function_env = env_new(env);

  for (int i = 0; i < size; i++) {
    Node *arg = value->node->args[i];
    hash_table_set(function_env->table, arg->value, args[i]);
  }

  Value *result = NULL;
  for (int i = 0; node->children[i] != NULL; i++) {
    Node *child = node->children[i];
    Value *value = evaluate_node(child, function_env);
    if (child->type == NODE_STATEMENT_RETURN) {
      result = value;
      break;
    }
  }


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
      Value *value = hash_table_get(env->table, node->value);
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
      return value;
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

      Value *value = hash_table_get(env->table, identifier);
      if (value != NULL && value->type == VALUE_FUNCTION) {
        return evaluate_function_call(value, args, size, env);
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
      fprintf(stderr, "runtime error: unexpected node type: %d\n", node->type);
      abort();
      break;
    }

  return NULL;

}

Value* evaluate(Node *node) {
  Env *global = env_new(NULL);
  return evaluate_node(node, global);
}
