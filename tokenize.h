#include <stdlib.h>

typedef enum TokenType {
  TOKEN_ANY,
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_SYMBOL,
  TOKEN_KEYWORD,
} TokenType;

typedef struct Token {
  TokenType type;
  char *value;
  struct Token *next;
} Token;

Token* tokenize(char *source);
void token_pp(Token* token);

static char *token_symbols[] = {
  "+",
  "-",
  "*",
  "/",
  "===",
  ">",
  "<",
  "=",
  "(",
  ")",
  "'",
  "{",
  "}",
  NULL,
};

static char *token_keywords[] = {
  "null",
  "undefined",
  "true",
  "false",
  "function",
  "var",
  "return",
  NULL,
};
