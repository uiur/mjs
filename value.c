#include "tokenize.h"
#include "parse.h"
#include "hash.h"
#include "value.h"
#include "object.h"
#include "boolean.h"
#include "number.h"
#include "array.h"
#include "function.h"
#include "string.h"
#include "inspect.h"
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

typedef struct CallContext {
  int returned;
} CallContext;

CallContext context;
CallContext *ctx = &context;

Binding binding_data;
Binding *binding = &binding_data;

Value* env_get(Env *env, const char *key)  {
  if (env == NULL) return NULL;

  Value* value = hash_table_get(env->table, key);
  if (value != NULL) return value;

  return env_get(env->parent, key);
}

void env_set(Env *env, const char *key, Value *value) {
  hash_table_set(env->table, key, value);
}

Value* require_object_prototype(Binding *binding) {
  Value *proto = value_object_create(NULL);
  binding->object_prototype = proto;
  return binding->object_prototype;
}

void load_prelude() {
  require_object_prototype(binding);
}

int value_is_truthy(Value *v) {
  if (v->primitive == NULL) return 0;

  switch (v->primitive->type) {
    case PRIMITIVE_BOOLEAN:
    case PRIMITIVE_NUMBER: {
      double n = value_number_unwrap(v);
      return n != 0;
    }

    default: {
      fprintf(stderr, "unexpected value type %d for value_is_truthy\n", v->kind);
      abort();
    }
  }
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
      fprintf(stderr, "log error: type %s cannot be inspect\n", PrimitiveTypeString[v->kind]);
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

  if (value_number_unwrap(left) == value_number_unwrap(right)) {
    return value_true_new();
  } else  {
    return value_false_new();
  }
}

Value* value_greater_than(int size, Value **args) {
  assert_args_size(size, 2);
  Value *left = args[0];
  Value *right = args[1];

  if (value_number_unwrap(left) > value_number_unwrap(right)) {
    return value_true_new();
  } else  {
    return value_false_new();
  }
}

Value* value_less_than(int size, Value **args) {
  assert_args_size(size, 2);
  Value *left = args[0];
  Value *right = args[1];

  if (value_number_unwrap(left) < value_number_unwrap(right)) {
    return value_true_new();
  } else  {
    return value_false_new();
  }
}

Value* evaluate_node(Node *node, Env *env);

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

Value* evaluate_function_call(Value *f, Value *this, Value **args, int size, Env *env) {
  PrimitiveFunction *value = (PrimitiveFunction*)(f->primitive);
  if (value->fn != NULL) {
    return (*(value->fn))(size, args);
  }

  Node *node = value->node;
  ctx->returned = 0;

  Env *function_env = env_new(env);

  for (int i = 0; i < size; i++) {
    Node *arg = value->node->args[i];
    hash_table_set(function_env->table, arg->value, args[i]);
  }

  if (this == NULL) this = value_undefined_new();
  env_set(function_env, "this", this);
  Value *result = evaluate_node_children(node, function_env);
  return result;
}

Value* evaluate_node(Node *node, Env *env) {
  switch (node->type) {
    // primitive nodes
    case NODE_PRIMITIVE_NUMBER: {
      char *str = node->value;
      Value *v = value_number_new((double)atoi(str));
      return v;
    }

    case NODE_PRIMITIVE_UNDEFINED: {
      return value_undefined_new();
    }

    case NODE_PRIMITIVE_NULL: {
      return value_null_new();
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

    case NODE_STATEMENT_LIST: {
      for (int i = 0; node->children[i] != NULL; i++) {
        Node *child = node->children[i];
        evaluate_node(child, env);
      }

      break;
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
      Node *left = node->children[0];
      Node *right = node->children[1];
      Value *right_value = evaluate_node(right, env);

      switch (left->type) {
        case NODE_IDENTIFIER: {
          env_set(env, left->value, right_value);
          break;
        }

        case NODE_OBJECT_MEMBER_ACCESS: {
          Value *v = evaluate_node(left->children[0], env);
          Value *property = evaluate_node(left->children[1], env);
          value_object_set(v, property, right_value);
          break;
        }

        default: {
          fprintf(stderr, "runtime error: unexpected node type for left of assignment: %s\n", NodeTypeString[left->type]);
          abort();
        }
      }

      return right_value;
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

    case NODE_BINARY_OPERATOR: {
      char *identifier = node->value;
      Node **children = node->children;

      int size = 0;
      while(children[size] != NULL) size++;

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
      Node **children = (node->children) + 1;

      int size = 0;
      while(children[size] != NULL) size++;

      Value **args = malloc(size * sizeof(Value));
      for (int i = 0; children[i] != NULL; i++) {
        args[i] = evaluate_node(children[i], env);
      }

      Node *callee_node = node->children[0];
      env_set(env, "this", NULL);
      Value *callee = evaluate_node(callee_node, env);
      if (callee == NULL) {
        RUNTIME_ERROR("function `%s` is not defined", callee_node->value);
        abort();
      }

      Value *this = env_get(env, "this");
      Value *return_value = evaluate_function_call(callee, this, args, size, env);
      return return_value;
    }


    case NODE_OBJECT: {
      Value *object = value_object_new(binding);
      for (int i = 0; node->children[i] != NULL; i++) {
        Node *entry = node->children[i];
        Node *identifier_node = entry->children[0];
        Node *value_node = entry->children[1];

        Value *vs = value_string_new(identifier_node->value);
        Value *v = evaluate_node(value_node, env);

        value_object_set(object, vs, v);
      }

      return object;
    }

    case NODE_OBJECT_MEMBER_ACCESS: {
      Value *v = evaluate_node(node->children[0], env);
      Value *member = evaluate_node(node->children[1], env);
      if (v->kind != VALUE_KIND_OBJECT) {
        fprintf(stderr, "runtime error: unexpected member access: %s\n", value_inspect(v));
        abort();
      }

      env_set(env, "this", v);
      return value_object_get(v, member);
    }

    case NODE_ARRAY: {
      Value *array = value_array_new(binding);

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

Value* require_klass_object() {
  Value *klass = value_function_new(NULL);
  value_object_set(klass, value_string_new("prototype"), binding->object_prototype);
  return klass;
}

Value* require_module_console() {
  Value *f = value_function_native_new(console_log);

  Value *console = value_object_create(NULL);
  value_object_set(console, value_string_new("log"), f);
  return console;
}

Env* env_global_new() {
  Env *global = env_new(NULL);

  load_prelude();

  env_set(global, "Object", require_klass_object());
  env_set(global, "console", require_module_console());

  return global;
}

Value* evaluate(Node *node) {
  Env *global = env_global_new();

  return evaluate_node(node, global);
}
