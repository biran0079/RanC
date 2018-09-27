#include "codegen_x86.h"

int main() {
  init_tokenizer();
  init_parser();
  int ast = parse(tokenize());
  init_codegen();
  generate_code(ast);
  return 0;
}
