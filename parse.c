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
    fprintf(stderr, "parse error: expect `%s`, but got `%s` (%d:%d)\n", str, state->token->value, state->token->line, state->token->column);
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

Node* parse_function_call(ParseState *state, Node *callee) {
  if (!token_matches(state->token, TOKEN_SYMBOL, "(")) return NULL;

  parse_state_next(state);

  Node *node = node_alloc(NODE_FUNCTION_CALL, 1);
  node->children[0] = callee;

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


void node_children_push(Node *node, Node *element) {
  int size = 0;
  while (node->children[size] != NULL) size++;
  int new_size = size + 1;

  node->children = realloc(node->children, (new_size + 1) * sizeof(Node*));
  node->children[new_size - 1] = element;
  node->children[new_size] = NULL;
}

Node* parse_array(ParseState *state) {
  if (token_matches(state->token, TOKEN_SYMBOL, "[")) {
    parse_state_next(state);

    Node *node = node_alloc(NODE_ARRAY, 0);
    while (1) {
      Node *expression = parse_expression(state);
      if (expression == NULL) break;

      node_children_push(node, expression);

      if (!token_matches(state->token, TOKEN_ANY, ",")) break;
      parse_state_next(state);
    }

    parse_state_expect(state, "]");
    return node;
  }

  return NULL;
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

      node_children_push(node, entry);

      if (!token_matches(state->token, TOKEN_ANY, ",")) break;
      parse_state_next(state);
    }

    parse_state_expect(state, "}");
    return node;
  }

  return NULL;
}

Node* parse_variable_assignment(ParseState *state, Node *left) {
  if (token_matches(state->token, TOKEN_SYMBOL, "=")) {
    parse_state_next(state);

    Node *node = node_alloc(NODE_VAR_ASSIGNMENT, 2);

    Node *expression = parse_expression(state);

    node->children[0] = left;
    node->children[1] = expression;

    return node;
  }

  return NULL;
}

Node* parse_term(ParseState *state) {
  Node *node;
  node = parse_reference(state);
  if (node != NULL) return node;

  node = parse_object(state);
  if (node != NULL) return node;

  node = parse_array(state);
  if (node != NULL) return node;

  node = parse_primary(state);
  if (node != NULL) return node;

  return NULL;
}

Node* parse_term_function_call(ParseState *state) {
  Node *node = parse_term(state);

  Node *function_call;
  while (1) {
    function_call = parse_function_call(state, node);
    if (function_call == NULL) break;
    node = function_call;
  }

  return node;
}

Node* parse_member_access(ParseState *state, Node *callee) {
  if (token_matches(state->token, TOKEN_SYMBOL, "[")) {
    parse_state_next(state);

    Node *expression = parse_expression(state);

    Node *node = node_alloc(NODE_OBJECT_MEMBER_ACCESS, 2);
    node->children[0] = callee;
    node->children[1] = expression;

    parse_state_expect(state, "]");

    return node;
  }

  return NULL;
}

Node* parse_term_member_access(ParseState *state) {
  Node *node = parse_term_function_call(state);

  Node *member_access;
  while (1) {
    member_access = parse_member_access(state, node);
    if (member_access == NULL) return node;
    node = member_access;
  }

  return node;
}

Node* parse_variable_assignment_operation(ParseState *state) {
  Node *node = parse_term_member_access(state);
  Node *assignment;
  while (1) {
    assignment = parse_variable_assignment(state, node);
    if (assignment == NULL) break;
    node = assignment;
  }

  return node;
}

const char *multiplicative_symbols[] =  { "*", "/", NULL };
Node* parse_multiplicative_operation(ParseState *state) {
  PARSE_BINARY_OPERATION(multiplicative_symbols, parse_variable_assignment_operation)
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

Node* parse_variable_declaration_statement(ParseState *state) {
  if (token_matches(state->token, TOKEN_KEYWORD, "var")) {
    parse_state_next(state);

    EXPECT_TOKEN_TYPE(state->token, TOKEN_IDENTIFIER);

    Node *node = node_alloc(NODE_VAR_DECLARATION, 2);

    Node *identifier = node_alloc(NODE_IDENTIFIER, 0);
    identifier->value = state->token->value;

    node->children[0] = identifier;

    parse_state_next(state);

    if (token_matches(state->token, TOKEN_SYMBOL, "=")) {
      parse_state_next(state);
      Node *expression = parse_expression(state);
      node->children[1] = expression;
    } else {
      node->children[1] = NULL;
    }

    parse_state_expect(state, ";");

    return node;
  }

  return NULL;
}

Node* parse_for_statement(ParseState *state) {
  if (token_matches(state->token, TOKEN_KEYWORD, "for")) {
    parse_state_next(state);
    Node *node = node_alloc(NODE_STATEMENT_FOR, 0);

    int arg_size = 3;
    node->args = malloc((arg_size + 1) * sizeof(Node*));
    node->args[arg_size] = NULL;

    parse_state_expect(state, "(");

    // init

    Node *variable_declaration_statement = parse_variable_declaration_statement(state);
    if (variable_declaration_statement != NULL) {
      node->args[0] = variable_declaration_statement;
    } else {
      node->args[0] = parse_expression(state);
      parse_state_expect(state, ";");
    }

    // condition
    node->args[1] = parse_expression(state);

    parse_state_expect(state, ";");

    // next
    node->args[2] = parse_expression(state);

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

Node* parse_while_statement(ParseState *state) {
  if (token_matches(state->token, TOKEN_KEYWORD, "while")) {
    parse_state_next(state);
    Node *node = node_alloc(NODE_STATEMENT_WHILE, 0);

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

Node* parse_if_statement(ParseState *state) {
  if (token_matches(state->token, TOKEN_KEYWORD, "if")) {
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

  Node *function_declaration = parse_function_declaration(state);
  if (function_declaration != NULL) return function_declaration;

  Node *if_statement = parse_if_statement(state);
  if (if_statement != NULL) return if_statement;

  Node *while_statement = parse_while_statement(state);
  if (while_statement != NULL) return while_statement;

  Node *for_statement = parse_for_statement(state);
  if (for_statement != NULL) return for_statement;

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
  return node;
}

// makes the number of node patterns less in order to help implementation of evaluator
//
// obj.foo => obj['foo']
Node* transform(Node *node) {
  for (int i = 0; node->children[i] != NULL; i++) {
    Node *child = node->children[i];
    node->children[i] = transform(child);
  }

  for (int i = 0; node->args[i] != NULL; i++) {
    Node *arg = node->args[i];
    node->args[i] = transform(arg);
  }

  switch (node->type) {
    case NODE_STATEMENT_FOR: {
      Node *init = node->args[0];
      Node *condition = node->args[1];
      Node *next = node->args[2];

      Node *while_node = node_alloc(NODE_STATEMENT_WHILE, 0);

      int arg_size = 1;
      while_node->args = malloc((arg_size + 1) * sizeof(Node*));
      while_node->args[arg_size] = NULL;
      while_node->args[0] = condition;

      while_node->children = node->children;
      if (next != NULL) {
        node_children_push(while_node, next);
      }

      Node *statement_list = node_alloc(NODE_STATEMENT_LIST, 0);
      if (init != NULL) {
        node_children_push(statement_list, init);
      }
      node_children_push(statement_list, while_node);

      return statement_list;
    }

    case NODE_BINARY_OPERATOR: {
      if (strcmp(node->value, ".") == 0) {
        Node *right = node->children[1];

        Node *str = node_alloc(NODE_PRIMITIVE_STRING, 0);
        str->value = right->value;
        free(right);

        node->type = NODE_OBJECT_MEMBER_ACCESS;
        node->children[1] = str;

        return node;
      }

      return node;
    }

    default: {
      return node;
    }
  }
}

Node* parse(Token *token) {
  ParseState state;
  state.token = token;

  Node *node = transform(parse_program(&state));
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
    if (node->type == NODE_PRIMITIVE_NUMBER || node->type == NODE_PRIMITIVE_BOOLEAN || node->type == NODE_PRIMITIVE_STRING || node->type == NODE_IDENTIFIER) {
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
