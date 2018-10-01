#include "tokenizer.h"

int main() {
  init_tokenizer();
  struct TokenizerOutput* output = tokenize();
  struct List* tokens = output->tokens;
  for (int i = 0; i < list_size(tokens); i++) {
    struct Token* token = list_get(tokens, i);
    printf("%s\t%4d:%3d-%3d\t%s\n", token_type_str[token->type], token->line, token->start_col, token->end_col, token->s);
  }
  return 0;
}
