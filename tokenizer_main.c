#include "tokenizer.h"

int main() {
  init_tokenizer();
  tokenize();
  int i = 0;
  while (i < token_num) {
    printf("%s\t%s\n", token_type_str[token_type[i]], token[i]);
    i++;
  }
  return 0;
}
