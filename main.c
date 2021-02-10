#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

    case NODE_FUNCTION_CALL: {
      char *identifier = node->value;

      int size = 0;
      while(node->children[size] != NULL) size++;

      Value **args = malloc(size * sizeof(Value));
      for (int i = 0; node->children[i] != NULL; i++) {
        args[i] = evaluate(node->children[i]);
      }

      if (strcmp(identifier, "log") == 0) {
        for (int i = 0; i < size; i++) {
          printf("%f\n", args[i]->value);
        }
      }

      break;
    }

    default:
      fprintf(stderr, "unexpected node type: %d\n", node->type);
      abort();
      break;
    }

  return NULL;
}

int main(int argc, char const **argv) {
  char *source = "log(23);";
  Token *token = tokenize(source);
  // token_pp(token);
  Node *node = parse(token);
  node_pp(node);
  printf("\n");
  evaluate(node);

  return 0;
}
