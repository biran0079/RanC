void init_tokenizer();
void tokenize();
void init_parser();
int parse();
void init_codegen();
void generate_code();

int main() {
  init_tokenizer();
  tokenize();
  init_parser();
  int ast = parse();
  init_codegen();
  generate_code(ast);
  return 0;
}
