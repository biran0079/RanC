#include "codegen_x86.h"

int main() {
  init_tokenizer();
  tokenize();
  init_parser();
  int ast = parse();
  init_codegen();
  generate_code(ast);
  return 0;
}
