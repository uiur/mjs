#include "tokenize.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define EXPECT_TOKEN_TYPE(TOKEN, TYPE) if ((TOKEN)->type != TYPE) { fprintf(stderr, "expect token type %s, but got %s\n", #TYPE, NodeTypeString[(TOKEN)->type]); abort(); }

#define PARSE_BINARY_OPERATION(SYMBOLS, NEXT) \
  Node *node = NEXT(state); \
  while (state->token != NULL && token_matches_any(state->token, TOKEN_SYMBOL, SYMBOLS)) { \
    char *symbol = state->token->value; \
    parse_state_next(state); \
    \
    Node *left = node; \
    node = node_alloc(NODE_BINARY_OPERATOR, 2); \
    node->value = symbol; \
    node->children[0] = left; \
    node->children[1] = NEXT(state); \
  } \
  return node;

int token_matches(Token *token, TokenType type, const char *value) {
  return token->type == type && strcmp(token->value, value) == 0;
}

int token_matches_any(Token *token, TokenType type, const char *values[]) {
  for (int i = 0; values[i] != NULL; i++) {
    if (token_matches(token, type, values[i])) {
      return 1;
    }
  }

  return 0;
}

typedef struct ParseState {
  struct Token *token;
} ParseState;

Node* parse_statement_list(ParseState *state);
Node* parse_statement(ParseState *state);

Node* node_alloc(NodeType type, int children_size) {
  Node *node = malloc(sizeof(Node));
  node->value = "";
  node->type = type;

  int arg_size = 0;
  node->args = malloc((arg_size + 1) * sizeof(Node*));
  node->args[arg_size] = NULL;

  node->children = malloc((children_size + 1) * sizeof(Node*));
  node->children[children_size] = NULL;
  return node;
}

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

Node* parse_expression(ParseState *state);

Node* parse_identifier(ParseState *state) {
  if (state->token->type == TOKEN_IDENTIFIER) {
    Node *node = node_alloc(NODE_IDENTIFIER, 0);
    node->value = state->token->value;

    parse_state_next(state);
    return node;
  }

  return NULL;
}

Node* parse_primary(ParseState *state) {
  if (state->token->type == TOKEN_NUMBER) {
    Node *node = node_alloc(NODE_PRIMITIVE_NUMBER, 0);
    node->value = state->token->value;

    parse_state_next(state);
    return node;
  }

  Node *identfier = parse_identifier(state);
  if (identfier != NULL) return identfier;

  if (token_matches(state->token, TOKEN_KEYWORD, "undefined")) {
    Node *node = node_alloc(NODE_PRIMITIVE_UNDEFINED, 0);
    parse_state_next(state);
    return node;
  }

  if (token_matches(state->token, TOKEN_KEYWORD, "null")) {
    Node *node = node_alloc(NODE_PRIMITIVE_NULL, 0);
    parse_state_next(state);
    return node;
  }

  if (token_matches(state->token, TOKEN_KEYWORD, "true")) {
    Node *node = node_alloc(NODE_PRIMITIVE_BOOLEAN, 0);
    node->value = "true";
    parse_state_next(state);
    return node;
  }

  if (token_matches(state->token, TOKEN_KEYWORD, "false")) {
    Node *node = node_alloc(NODE_PRIMITIVE_BOOLEAN, 0);
    node->value = "false";
    parse_state_next(state);
    return node;
  }

  if (token_matches(state->token, TOKEN_SYMBOL, "'")) {
    parse_state_next(state);

    EXPECT_TOKEN_TYPE(state->token, TOKEN_IDENTIFIER);
    Node *node = node_alloc(NODE_PRIMITIVE_STRING, 0);
    node->value = state->token->value;

    parse_state_next(state);

    parse_state_expect(state, "'");
    return node;
  }

  if (token_matches(state->token, TOKEN_SYMBOL, "(")) {
    parse_state_next(state);
    Node *expression = parse_expression(state);
    parse_state_expect(state, ")");
    return expression;
  }

  return NULL;
}

const char *dot_symbols[] =  { ".", NULL };
Node* parse_reference(ParseState *state) {
  PARSE_BINARY_OPERATION(dot_symbols, parse_identifier)
}

Node* parse_function_call(ParseState *state) {
  Node *identifier = parse_reference(state);
  if (identifier == NULL) return NULL;
  if (!token_matches(state->token, TOKEN_SYMBOL, "(")) return identifier;
  parse_state_next(state);

  Node *node = node_alloc(NODE_FUNCTION_CALL, 1);
  node->children[0] = identifier;

  int i = 1;
  while (1) {
    Node *expression = parse_expression(state);
    if (expression == NULL) break;

    int size = i + 1;
    node->children = realloc(node->children, (size + 1) * sizeof(Node*));
    node->children[i] = expression;
    node->children[size] = NULL;

    if (!token_matches(state->token, TOKEN_ANY, ",")) break;
    parse_state_next(state);

    i++;
  }

  parse_state_expect(state, ")");

  return node;
}

Node* parse_object(ParseState *state) {
  if (token_matches(state->token, TOKEN_SYMBOL, "{")) {
    parse_state_next(state);

    Node *node = node_alloc(NODE_OBJECT, 0);
    int size = 0;
    while (1) {
      Node *identifier = parse_identifier(state);
      if (identifier == NULL) break;

      size++;

      parse_state_expect(state, ":");

      Node *value = parse_expression(state);
      if (value == NULL) {
        fprintf(stderr, "parse error: expect expression after : in object, but got NULL");
        abort();
      }

      Node *entry = node_alloc(NODE_OBJECT_ENTRY, 2);
      entry->children[0] = identifier;
      entry->children[1] = value;

      node->children = realloc(node->children, (size + 1) * sizeof(Node*));
      node->children[size] = NULL;
      node->children[size - 1] = entry;

      if (!token_matches(state->token, TOKEN_ANY, ",")) break;
      parse_state_next(state);
    }

    parse_state_expect(state, "}");
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

  node = parse_object(state);
  if (node != NULL) return node;

  return NULL;
}

const char *multiplicative_symbols[] =  { "*", "/", NULL };
Node* parse_multiplicative_operation(ParseState *state) {
  PARSE_BINARY_OPERATION(multiplicative_symbols, parse_term)
}

const char *additive_symbols[] =  { "+", "-", NULL };
Node* parse_additive_operation(ParseState *state) {
  PARSE_BINARY_OPERATION(additive_symbols, parse_multiplicative_operation)
}

const char *equality_symbols[] =  { "===", ">", "<", NULL };
Node* parse_equality_operation(ParseState *state) {
  PARSE_BINARY_OPERATION(equality_symbols, parse_additive_operation)
}

Node* parse_expression(ParseState *state) {
  return parse_equality_operation(state);
}

Node* parse_return_statement(ParseState *state) {
  Token *head = state->token;
  if (token_matches(head, TOKEN_KEYWORD, "return")) {
    parse_state_next(state);

    Node *node = node_alloc(NODE_STATEMENT_RETURN, 1);
    Node *expression = parse_expression(state);
    node->value = "return";
    node->children[0] = expression;

    parse_state_expect(state, ";");
    return node;
  }

  return NULL;
}

Node* parse_function_declaration(ParseState *state) {
  Token *head = state->token;
  if (token_matches(head, TOKEN_KEYWORD, "function")) {
    parse_state_next(state);

    EXPECT_TOKEN_TYPE(head->next, TOKEN_IDENTIFIER);
    Token *function_name = head->next;
    parse_state_next(state);

    parse_state_expect(state, "(");

    Node *node = node_alloc(NODE_FUNCTION_DECLARATION, 0);
    node->value = function_name->value;
    int size = 0;
    while (state->token->type == TOKEN_IDENTIFIER) {
      size++;
      node->args = realloc(node->args, (size + 1) * sizeof(Node*));
      node->args[size] = NULL;

      Node *argument = node_alloc(NODE_IDENTIFIER, 0);
      Token *token = state->token;
      argument->value = token->value;
      node->args[size - 1] = argument;

      parse_state_next(state);

      if (!token_matches(state->token, TOKEN_ANY, ",")) break;
      parse_state_next(state);
    }

    parse_state_expect(state, ")");

    parse_state_expect(state, "{");

    Node *statement_list = parse_statement_list(state);

    node->children = statement_list->children;
    free(statement_list);


    parse_state_expect(state, "}");
    return node;
  }

  return NULL;
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
    parse_state_expect(state, ";");

    return node;
  }

  return NULL;
}

Node* parse_variable_declaration_statement(ParseState *state) {
  if (token_matches(state->token, TOKEN_KEYWORD, "var")) {
    parse_state_next(state);

    EXPECT_TOKEN_TYPE(state->token, TOKEN_IDENTIFIER);

    Node *node = node_alloc(NODE_VAR_DECLARATION, 1);

    Node *identifier = node_alloc(NODE_IDENTIFIER, 0);
    identifier->value = state->token->value;

    node->children[0] = identifier;

    parse_state_next(state);
    parse_state_expect(state, ";");

    return node;
  }

  return NULL;
}

Node* parse_if_statement(ParseState *state) {
  if (strcmp(state->token->value, "if") == 0) {
    parse_state_next(state);
    Node *node = node_alloc(NODE_STATEMENT_IF, 0);

    int arg_size = 1;
    node->args = malloc((arg_size + 1) * sizeof(Node*));
    node->args[arg_size] = NULL;

    parse_state_expect(state, "(");

    Node *expression = parse_expression(state);
    node->args[0] = expression;

    parse_state_expect(state, ")");
    parse_state_expect(state, "{");

    Node *statement_list = parse_statement_list(state);
    node->children = statement_list->children;
    free(statement_list);

    parse_state_expect(state, "}");

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

  Node *function_declaration = parse_function_declaration(state);
  if (function_declaration != NULL) return function_declaration;

  Node *if_statement = parse_if_statement(state);
  if (if_statement != NULL) return if_statement;

  Node *return_statement = parse_return_statement(state);
  if (return_statement != NULL) return return_statement;

  Node *expression = parse_expression(state);
  if (expression == NULL) return NULL;
  parse_state_expect(state, ";");
  return expression;
}

Node* parse_statement_list(ParseState *state) {
  int size = 0;
  Node *node = node_alloc(NODE_STATEMENT_LIST, size);

  while (1) {
    Node *statement_node = parse_statement(state);
    if (statement_node == NULL) break;

    size++;
    node->children = realloc(node->children, (size + 1) * sizeof(Node*));
    node->children[size - 1] = statement_node;
    node->children[size] = NULL;
  }

  return node;
}

Node* parse_program(ParseState *state) {
  Node *node = parse_statement_list(state);
  node->type = NODE_PROGRAM;

  return node;
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
    const char *label;
    if (node->type == NODE_PRIMITIVE_NUMBER || node->type == NODE_PRIMITIVE_BOOLEAN || node->type == NODE_IDENTIFIER) {
      label = node->value;
    } else {
      label = NodeTypeString[node->type];
    }

    printf("%s", label);
    return;
  }

  printf("(%s", NodeTypeString[node->type]);
  if (strlen(node->value) > 0) {
    printf(" %s", node->value);
  }

  for (int i = 0; node->args[i] != NULL; i++) {
    printf(" ");
    node_pp(node->args[i]);
  }

  for (int i = 0; node->children[i] != NULL; i++) {
    printf(" ");
    node_pp(node->children[i]);
  }

  printf(")");
}
