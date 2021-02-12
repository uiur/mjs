typedef enum NodeType {
  NODE_PRIMITIVE_NUMBER,
  NODE_PRIMITIVE_STRING,
  NODE_PRIMITIVE_BOOLEAN,
  NODE_PRIMITIVE_NULL,
  NODE_PRIMITIVE_UNDEFINED,
  NODE_IDENTIFIER,
  NODE_BINARY_OPERATOR,
  NODE_UNARY_OPERATOR,
  NODE_STATEMENT_RETURN,
  NODE_STATEMENT_IF,
  NODE_VAR_DECLARATION,
  NODE_VAR_ASSIGNMENT,
  NODE_FUNCTION_CALL,
  NODE_FUNCTION_DECLARATION,
  NODE_PROGRAM,
} NodeType;

typedef struct Node {
  NodeType type;
  char *value;
  struct Node **args;
  struct Node **children;
} Node;

typedef enum TokenType {
  TOKEN_ANY,
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_SYMBOL,
} TokenType;

typedef struct Token {
  TokenType type;
  char *value;
  struct Token *next;
} Token;

Token* tokenize(char *source);
void token_pp(Token* token);

Node* parse(Token *token);
void node_pp(Node *node);
