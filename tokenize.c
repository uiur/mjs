#include "tokenize.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>



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
    token->next = NULL;

    int size = 1;
    if (isalpha(*current)) {
      for (; current[size] != '\0' && isalnum(current[size]); size++) ;

      char *buf = calloc(size + 1, sizeof(char));
      strncpy(buf, current, size);

      int is_keyword = 0;
      for (int i = 0; token_keywords[i] != NULL; i++) {
        if (strcmp(buf, token_keywords[i]) == 0) {
          is_keyword = 1;
          break;
        }
      }

      if (is_keyword) {
        token->type = TOKEN_KEYWORD;
      } else {
        token->type = TOKEN_IDENTIFIER;
      }
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
    assert(strlen(token->value) > 0);

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
