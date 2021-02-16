#define NODE_ENUM(M) \
  M(PRIMITIVE_NUMBER) \
  M(PRIMITIVE_STRING) \
  M(PRIMITIVE_BOOLEAN) \
  M(PRIMITIVE_NULL) \
  M(PRIMITIVE_UNDEFINED) \
  M(IDENTIFIER) \
  M(BINARY_OPERATOR) \
  M(UNARY_OPERATOR) \
  M(STATEMENT_RETURN) \
  M(STATEMENT_IF) \
  M(VAR_DECLARATION) \
  M(VAR_ASSIGNMENT) \
  M(FUNCTION_CALL) \
  M(FUNCTION_DECLARATION) \
  M(PROGRAM) \
  M(STATEMENT_LIST)
#define TO_ENUM(X) NODE_##X,
#define TO_STRING(X) #X,

typedef enum NodeType {
  NODE_ENUM(TO_ENUM)
} NodeType;

static const char *NodeTypeString[] = {
  NODE_ENUM(TO_STRING)
};

typedef struct Node {
  NodeType type;
  char *value;
  struct Node **args;
  struct Node **children;
} Node;

Node* parse(Token *token);
void node_pp(Node *node);
