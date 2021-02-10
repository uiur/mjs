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

Token* tokenize(char *source);
void token_pp(Token* token);

Node* parse(Token *token);
void node_pp(Node *node);
