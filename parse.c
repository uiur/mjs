#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define EXPECT_TOKEN_TYPE(TOKEN, TYPE) if ((TOKEN)->type != TYPE) { fprintf(stderr, "expect token type %s, but got %d", #TYPE, (TOKEN)->type); abort(); }

char *token_symbols[] = {
  "+",
  "-",
  "=",
  NULL,
};

Token* tokenize(char *source) {
  char *current = source;
  Token *head = malloc(sizeof(Token));
  Token *prev_token = head;

  while (*current != '\0') {
    if (isspace(*current)) {
      current++;
      continue;
    }
    Token *token = malloc(sizeof(Token));
    token->type = TOKEN_ANY;

    int size = 1;
    if (isalpha(*current)) {
      token->type = TOKEN_IDENTIFIER;
      for (; current[size] != '\0' && isalnum(current[size]); size++) ;
    } else if (isdigit(*current)) {
      token->type = TOKEN_NUMBER;
      for (; current[size] != '\0' && isdigit(current[size]); size++) ;
    } else {
      for (int i = 0; token_symbols[i] != NULL; i++) {
        char *symbol = token_symbols[i];
        int symbol_length = strlen(symbol);
        if (strncmp(symbol, current, symbol_length) == 0) {
          size = symbol_length;
          token->type = TOKEN_SYMBOL;
          break;
        }
      }
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
// statement := expression ';' | variable_declaration_statement | variable_assignment_statement
// variable_declaration_statement := 'var' identifier ';'
// variable_assignment_statement := identifier '=' expression ';'
// expression := term (op expression)?
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
    fprintf(stderr, "parse error: expect `%s`, but got `%s`\n", str, state->token->value);
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

      parse_state_next(state);
      parse_state_next(state);

      int i = 0;
      while(1) {
        Node *expression = parse_expression(state);
        if (expression == NULL) break;

        int size = i + 1;
        node->children = realloc(node->children, (size + 1) * sizeof(Node*));
        node->children[i] = expression;
        node->children[size] = NULL;
        i++;
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

Node* parse_term(ParseState *state) {
  Node *node;

  node = parse_function_call(state);
  if (node != NULL) return node;

  node = parse_primary(state);
  if (node != NULL) return node;

  return NULL;
}

Node* parse_expression(ParseState *state) {
  Node *term = parse_term(state);

  if (state->token == NULL) return term;
  if (state->token->type != TOKEN_SYMBOL) return term;

  char *symbol = state->token->value;
  parse_state_next(state);
  Node *expression = parse_expression(state);

  Node *node = malloc(sizeof(Node));
  node->type = NODE_BINARY_OPERATOR;
  node->value = symbol;

  int size = 2;
  node->children = malloc((size + 1) * sizeof(Node*));
  node->children[0] = term;
  node->children[1] = expression;
  node->children[size] = NULL;

  return node;
}

Node* node_alloc(NodeType type, int children_size) {
  Node *node = malloc(sizeof(Node));
  node->type = type;
  node->children = malloc((children_size + 1) * sizeof(Node*));
  node->children[children_size] = NULL;
  return node;
}

int token_matches(Token *token, TokenType type, const char *value) {
  return token->type == type && strcmp(token->value, value) == 0;
}

Node* parse_variable_assignment_statement(ParseState *state) {
  Token *current = state->token;
  if (current->type == TOKEN_IDENTIFIER && token_matches(current->next, TOKEN_SYMBOL, "=")) {
    parse_state_next(state);
    parse_state_next(state);

    Node *node = node_alloc(NODE_VAR_ASSIGNMENT, 2);
    Node *identifier = node_alloc(NODE_IDENTIFIER, 0);
    identifier->value = current->value;

    Node *expression = parse_expression(state);

    node->children[0] = identifier;
    node->children[1] = expression;

    return node;
  }

  return NULL;
}

Node* parse_variable_declaration_statement(ParseState *state) {
  if (strcmp(state->token->value, "var") == 0) {
    parse_state_next(state);

    EXPECT_TOKEN_TYPE(state->token, TOKEN_IDENTIFIER);

    Node *node = node_alloc(NODE_VAR_DECLARATION, 1);
    node->value = "var";

    Node *identifier = node_alloc(NODE_IDENTIFIER, 0);
    identifier->value = state->token->value;

    node->children[0] = identifier;

    parse_state_next(state);
    parse_state_expect(state, ";");

    return node;
  }

  return NULL;
}

Node* parse_statement(ParseState *state) {
  if (state->token == NULL) return NULL;

  Node *variable_declaration_statement = parse_variable_declaration_statement(state);
  if (variable_declaration_statement != NULL) return variable_declaration_statement;

  Node *variable_assignment_statement = parse_variable_assignment_statement(state);
  if (variable_assignment_statement != NULL) return variable_assignment_statement;

  Node *expression = parse_expression(state);
  if (expression == NULL) return NULL;
  parse_state_expect(state, ";");
  return expression;
}

Node* parse_program(ParseState *state) {
  Node *program_node = malloc(sizeof(Node));
  program_node->type = NODE_PROGRAM;

  int size = 0;
  program_node->children = malloc(sizeof(Node*));
  program_node->children[0] = NULL;

  while (1) {
    Node *statement_node = parse_statement(state);
    if (statement_node == NULL) break;

    size++;
    program_node->children = realloc(program_node->children, (size + 1) * sizeof(Node*));
    program_node->children[size - 1] = statement_node;
    program_node->children[size] = NULL;
  }


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
