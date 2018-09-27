#include "tokenizer.h"
#include "parser.h"

int main() {
  init_tokenizer();
  init_parser();
  print_ast(parse(tokenize()));
  return 0;
}
