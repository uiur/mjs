#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum NodeType {
  NODE_PRIMITIVE_NUMBER,
  NODE_PRIMITIVE_STRING,
  NODE_PRIMITIVE_BOOLEAN,
  NODE_PRIMITIVE_NULL,
  NODE_PRIMITIVE_UNDEFINED,

  // Node { value: "console", children: { "log" } }
  NODE_IDENTIFIER,

  // Node { value: "+", children: { left, right } }
  // Node { value: "&&", children: { left, right } }
  NODE_BINARY_OPERATOR,

  // Node { value: "-", children: {right} }
  NODE_UNARY_OPERATOR,

  // Node { value: "return", children: { expression } }
  NODE_STATEMENT_RETURN,

  // Node { condition, if_body, else_body }
  NODE_STATEMENT_IF,

  // Node { identifier: "foo", value:  }
  NODE_VAR_DECLARATION,

  // Node { identifier: "foo", value:  }
  NODE_VAR_ASSIGNMENT,

  // Node { value: identifier, args: children }
  NODE_FUNCTION_CALL,

  // Node { args, body }
  NODE_FUNCTION_DECLARATION,

  NODE_PROGRAM,
} NodeType;

typedef struct Node {
  NodeType type;
  char *value;
  struct Node **children;
} Node;

typedef enum TokenType {
  TOKEN_ANY,
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
} TokenType;

typedef struct Token {
  TokenType type;
  char *value;
  struct Token *next;
} Token;

typedef enum ValueType {
  VALUE_NUMBER,
} ValueType;

typedef struct Value {
  ValueType type;
  double value;
} Value;

Token* tokenize(char *source) {
  char *current = source;
  Token *head = malloc(sizeof(Token));
  Token *prev_token = head;

  while (*current != '\0') {
    Token *token = malloc(sizeof(Token));
    token->type = TOKEN_ANY;

    int size = 1;
    if (isalpha(*current)) {
      token->type = TOKEN_IDENTIFIER;
      for (; current[size] != '\0' && isalnum(current[size]); size++) ;
    } else if (isdigit(*current)) {
      token->type = TOKEN_NUMBER;
      for (; current[size] != '\0' && isdigit(current[size]); size++) ;
    }

    token->value = calloc(size + 1, sizeof(char));
    strncpy(token->value, current, size);

    prev_token->next = token;
    prev_token = token;
    current += size;
  }

  return head->next;
}

void token_pp(Token* token) {
  for (Token *current = token; current != NULL; current = current->next) {
    printf("%s ", current->value);
  }

  printf("\n");
}

// program := statement*
// statement := expression ';'
// expression := term (op term)*
// term := function_call | primary
// function_call := identifier '(' (expression (',' expression)*)?')'
// primary := '(' expression ')' | identifier | number_constant | string_constant
// identifier := [a-zA-Z][a-zA-Z0-9]*
// number_constant := [0-9][0-9]*
// string_constant := "'" [^']* "'"
// op := '+' | '-'
typedef struct ParseState {
  struct Token *token;
} ParseState;

void parse_state_next(ParseState *state) {
  state->token = state->token->next;
}

void parse_state_expect(ParseState *state, char *str) {
  if (strcmp(state->token->value, str) != 0) {
    fprintf(stderr, "parse error: expect `%s`, but got `%s`", str, state->token->value);
    abort();
    return;
  }

  parse_state_next(state);
}

Node* parse_primary(ParseState *state) {
  if (state->token->type == TOKEN_NUMBER) {
    Node *node = malloc(sizeof(Node));
    node->type = NODE_PRIMITIVE_NUMBER;
    node->value = state->token->value;

    parse_state_next(state);

    return node;
  }

  return NULL;
}
Node* parse_expression(ParseState *state);

Node* parse_function_call(ParseState *state) {
  Token *current = state->token;
  if (current->type == TOKEN_IDENTIFIER) {
    if (strcmp(current->next->value, "(") == 0) {
      Node *node = malloc(sizeof(Node));
      node->type = NODE_FUNCTION_CALL;
      node->value = current->value;

      int size = 1;
      node->children = malloc((size + 1) * sizeof(Node*));
      node->children[size] = NULL;

      parse_state_next(state);
      parse_state_next(state);

      Node *expression = parse_expression(state);
      if (expression != NULL) {
        node->children[0] = expression;
      }

      parse_state_expect(state, ")");

      return node;
    }

    Node *node = malloc(sizeof(Node));
    node->type = NODE_IDENTIFIER;
    node->value = current->value;

    parse_state_next(state);
    return node;
  }

  return NULL;
}

Node* parse_expression(ParseState *state) {
  Node *node;

  node = parse_function_call(state);
  if (node != NULL) return node;

  node = parse_primary(state);
  if (node != NULL) return node;

  return NULL;
}

Node* parse_statement(ParseState *state) {
  Node *expression = parse_expression(state);
  parse_state_expect(state, ";");
  return expression;
}

Node* parse_program(ParseState *state) {
  Node *program_node = malloc(sizeof(Node));
  program_node->type = NODE_PROGRAM;
  Node *statement_node = parse_statement(state);
  program_node->children = malloc(2 * sizeof(Node*));

  if (statement_node != NULL) {
    program_node->children[0] = statement_node;
  }

  program_node->children[1] = NULL;

  return program_node;
}

Node* parse(Token *token) {
  ParseState state;
  state.token = token;

  Node *node = parse_program(&state);
  if (state.token != NULL) {
    fprintf(stderr, "parse error: unexpected token `%s`\n", (state.token)->value);
    abort();
  }

  return node;


  // Node *number_node = malloc(sizeof(Node));
  // number_node->type = NODE_PRIMITIVE_NUMBER;
  // number_node->value = "1";

  // Node *function_call_node = malloc(sizeof(Node));
  // function_call_node->type = NODE_FUNCTION_CALL;
  // function_call_node->value = "log";
  // function_call_node->children = malloc(2 * sizeof(Node));
  // function_call_node->children[0] = number_node;
  // function_call_node->children[1] = NULL;

  // Node *root = malloc(sizeof(Node));
  // root->type = NODE_PROGRAM;
  // root->children = malloc(2 * sizeof(Node));

  // log(1);

  // root->children[0] = function_call_node;
  // root->children[1] = NULL;

  // return root;
}

void node_pp(Node *node) {
  int size = 0;
  for (; node->children != NULL && node->children[size] != NULL; size++) ;

  if (size == 0) {
    printf("%s", node->value);
    return;
  }

  char *label;
  if (node->type == NODE_PROGRAM) {
    label = "program";
  } else {
    label = node->value;
  }

  printf("(%s", label);

  for (int i = 0; node->children[i] != NULL; i++) {
    printf(" ");
    node_pp(node->children[i]);
  }

  printf(")");
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
