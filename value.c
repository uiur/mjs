#include "tokenize.h"
#include "parse.h"
#include "hash.h"
#include "value.h"
#include "object.h"
#include "array.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define RUNTIME_ERROR(...) \
  fprintf(stderr, "runtime error: "); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__); \
  abort();

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

void env_set(Env *env, const char *key, Value *value) {
  hash_table_set(env->table, key, value);
}


Value* value_new(ValueType type) {
  Value *v = malloc(sizeof(Value));
  v->type = type;
  v->value = 0;
  return v;
}

Value* value_true_new() {
  Value *v = value_new(VALUE_BOOLEAN);
  v->value = 1;
  return v;
}

Value* value_false_new() {
  Value *v = value_new(VALUE_BOOLEAN);
  v->value = 0;
  return v;
}

Value* value_undefined_new() {
  return value_new(VALUE_UNDEFINED);
}

Value* value_number_new(double n) {
  Value *number = malloc(sizeof(Value));
  number->type = VALUE_NUMBER;
  number->value = n;

  return number;
}

double value_number_unwrap(Value* v) {
  return v->value;
}

Value* value_function_new(Node *node) {
  ValueFunction *function_value = malloc(sizeof(ValueFunction));
  function_value->type = VALUE_FUNCTION;
  function_value->node = node;
  if (node != NULL) {
    function_value->name = node->value;
  } else {
    function_value->name = "";
  }
  return (Value*)function_value;
}

int value_is_truthy(Value *v) {
  switch (v->type) {
    case VALUE_NUMBER: {
      double n = v->value;
      return n != 0;
    }

    case VALUE_BOOLEAN: {
      return v->value == 1;
    }

    default: {
      fprintf(stderr, "unexpected value type %d for value_is_truthy\n", v->type);
      abort();
    }
  }
}

char* value_inspect(Value *v) {
  char *buf = malloc(100 * sizeof(char));
  switch (v->type) {
    case VALUE_NUMBER: {
      double n = v->value;
      sprintf(buf, "%.0f", n);
      return buf;
    }

    case VALUE_STRING: {
      ValueString *vs = (ValueString*)v;
      return vs->string;
    }

    case VALUE_BOOLEAN: {
      if (v->value == 1) {
        return "true";
      } else {
        return "false";
      }
    }

    case VALUE_ARRAY: {
      ValueArray *array = (ValueArray*)v;
      strcat(buf, "[");
      for (int i = 0; i < value_number_unwrap(value_array_length(array)); i++) {
        if (i > 0) {
          strcat(buf, ", ");
        }
        const char *s = value_inspect(value_array_get(array, value_number_new(i)));
        strcat(buf, s);
      }

      strcat(buf, "]");
      return buf;
    }

    case VALUE_NULL: {
      return "null";
    }

    case VALUE_UNDEFINED: {
      return "undefined";
    }

    case VALUE_FUNCTION: {
      return "function";
    }

    default: {
      return NULL;
    }
  }

  return NULL;
}

void value_pp(Value *v) {
  if (v == NULL) {
    RUNTIME_ERROR("unexpected NULL");
  }
  char *s = value_inspect(v);
  printf("%s\n", s);
}

Value* console_log(int size, Value **args) {
  for (int i = 0; i < size; i++) {
    Value *v = args[i];
    if (v == NULL) {
      fprintf(stderr, "log error: unexpected null\n");
      abort();
    }

    const char *str = value_inspect(v);
    if (str == NULL) {
      fprintf(stderr, "log error: type %s cannot be inspect\n", ValueTypeString[v->type]);
      abort();
    }
    printf("%s\n", str);
  }
  return NULL;
}

void assert_args_size(int size, int expected) {
  if (size != expected) {
    fprintf(stderr, "requires %d arguments, but %d\n", expected, size);
    abort();
  }
}

Value* value_equal(int size, Value **args) {
  assert_args_size(size, 2);
  Value *left = args[0];
  Value *right = args[1];

  if (left->type == right->type && left->value == right->value) {
    return value_true_new();
  } else  {
    return value_false_new();
  }
}

Value* value_greater_than(int size, Value **args) {
  assert_args_size(size, 2);
  Value *left = args[0];
  Value *right = args[1];

  if (left->type == right->type && left->value > right->value) {
    return value_true_new();
  } else  {
    return value_false_new();
  }
}

Value* value_less_than(int size, Value **args) {
  assert_args_size(size, 2);
  Value *left = args[0];
  Value *right = args[1];

  if (left->type == right->type && left->value < right->value) {
    return value_true_new();
  } else  {
    return value_false_new();
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

Value* evaluate_function_call(ValueFunction *value, Value **args, int size, Env *env) {
  if (strcmp(value->name, "__console_log__") == 0) return console_log(size, args);

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
      Node *identifier = node->children[0];
      Node *right = node->children[1];

      Value *value = right == NULL ? value_undefined_new() : evaluate_node(right, env);

      env_set(env, identifier->value, value);
      break;
    }

    case NODE_VAR_ASSIGNMENT: {
      Node *identifier = node->children[0];
      Node *right = node->children[1];

      env_set(env, identifier->value, evaluate_node(right, env));

      break;
    }

    case NODE_FUNCTION_DECLARATION: {
      Value *v = value_function_new(node);
      env_set(env, node->value, v);
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

    case NODE_STATEMENT_WHILE: {
      while (value_is_truthy(evaluate_node(node->args[0], env))) {
        evaluate_node_children(node, env);
      }

      return NULL;
    }

    case NODE_STATEMENT_FOR: {
      Node *init = node->args[0];
      Node *condition = node->args[1];
      Node *next = node->args[2];

      evaluate_node(init, env);
      while (value_is_truthy(evaluate_node(condition, env))) {
        evaluate_node_children(node, env);
        evaluate_node(next, env);
      }

      return NULL;
    }

    case NODE_BINARY_OPERATOR: {
      char *identifier = node->value;
      Node **children = node->children;

      int size = 0;
      while(children[size] != NULL) size++;
      if (strcmp(identifier, ".") == 0) {
        Value *receiver = evaluate_node(node->children[0], env);
        Node *message_node = node->children[1];
        Value *message = value_string_new(message_node->value);

        // array.length
        if (receiver->type == VALUE_ARRAY && strcmp(message_node->value, "length") == 0) {
          return value_array_length((ValueArray *)receiver);
        }

        if (receiver->type != VALUE_OBJECT) {
          fprintf(stderr, "runtime error: expected object, but got %s\n", value_inspect(receiver));
          abort();
        }

        return value_object_get((ValueObject*)receiver, (ValueString*)message);
      }

      Value **args = malloc(size * sizeof(Value));
      for (int i = 0; children[i] != NULL; i++) {
        args[i] = evaluate_node(children[i], env);
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

      if (strcmp(identifier, "===") == 0) {
        return value_equal(size, args);
      }

      if (strcmp(identifier, ">") == 0) {
        return value_greater_than(size, args);
      }

      if (strcmp(identifier, "<") == 0) {
        return value_less_than(size, args);
      }

      fprintf(stderr, "runtime error: operator `%s` is not defined\n", identifier);
      abort();
    }

    case NODE_FUNCTION_CALL: {
      Node *reference = node->children[0];
      Node **children = (node->children) + 1;

      int size = 0;
      while(children[size] != NULL) size++;

      Value **args = malloc(size * sizeof(Value));
      for (int i = 0; children[i] != NULL; i++) {
        args[i] = evaluate_node(children[i], env);
      }

      Value *value = evaluate_node(reference, env);
      if (value != NULL) {
        if (value->type != VALUE_FUNCTION) {
          RUNTIME_ERROR("`%s` is not function", value_inspect(value));
        }

        ValueFunction *function = (ValueFunction*)value;
        Value *return_value = evaluate_function_call(function, args, size, env);
        return return_value;
      }

      RUNTIME_ERROR("function `%s` is not defined", reference->value);

      break;
    }

    case NODE_PRIMITIVE_UNDEFINED: {
      return value_new(VALUE_UNDEFINED);
    }

    case NODE_PRIMITIVE_NULL: {
      return value_new(VALUE_NULL);
    }

    case NODE_PRIMITIVE_BOOLEAN: {
      if (strcmp(node->value, "true") == 0) {
        return value_true_new();
      } else if (strcmp(node->value, "false") == 0) {
        return value_false_new();
      } else {
        fprintf(stderr, "runtime error: unexpected value for boolean: %s\n", node->value);
        abort();
      }
    }

    case NODE_PRIMITIVE_STRING: {
      return value_string_new(node->value);
    }

    case NODE_OBJECT: {
      Value *object = value_object_new();
      for (int i = 0; node->children[i] != NULL; i++) {
        Node *entry = node->children[i];
        Node *identifier_node = entry->children[0];
        Node *value_node = entry->children[1];

        Value *vs = value_string_new(identifier_node->value);
        Value *v = evaluate_node(value_node, env);

        value_object_set((ValueObject*)object, (ValueString*)vs, v);
      }

      return object;
    }

    case NODE_OBJECT_MEMBER_ACCESS: {
      Value *v = evaluate_node(node->children[0], env);
      Value *member = evaluate_node(node->children[1], env);

      switch (v->type) {
        case VALUE_OBJECT: {
          return value_object_get((ValueObject*)v, (ValueString*)member);
        }

        case VALUE_ARRAY: {
          return value_array_get((ValueArray*)v, member);
        }

        default: {
          fprintf(stderr, "runtime error: unexpected member access: %s\n", ValueTypeString[v->type]);
          abort();
        }
      }
      return NULL;
    }

    case NODE_ARRAY: {
      ValueArray *array = (ValueArray*)value_array_new();

      for (int i = 0; node->children[i] != NULL; i++) {
        Node *child = node->children[i];
        Value *el = evaluate_node(child, env);
        value_array_set(array, value_number_new(i), el);
      }

      return (Value*)array;
    }

    default:
      fprintf(stderr, "runtime error: unexpected node type: %s\n", NodeTypeString[node->type]);
      abort();
      break;
    }

  return NULL;

}

Env* env_global_new() {
  Env *global = env_new(NULL);

  ValueFunction *f = (ValueFunction*)value_function_new(NULL);
  f->name = "__console_log__";

  Value *o = value_object_new();
  value_object_set((ValueObject*)o, (ValueString*)value_string_new("log"), (Value*)f);
  env_set(global, "console", o);

  return global;
}

Value* evaluate(Node *node) {
  Env *global = env_global_new();

  return evaluate_node(node, global);
}
