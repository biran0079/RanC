#include "tokenizer.h"

int main() {
  init_tokenizer();
  struct List* tokens = tokenize(0);
  for (int i = 0; i < list_size(tokens); i++) {
    struct Token* token = list_get(tokens, i);
    printf("%s\t%s\n", token_type_str[token->type], token->s);
  }
  return 0;
}
