#include "codegen_x86.h"

int main() {
  init_tokenizer();
  init_parser();
  init_codegen();
  generate_code(process(parse(tokenize())));
  return 0;
}
