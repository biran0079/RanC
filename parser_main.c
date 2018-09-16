void tokenize();
int parse();
void init_tokenizer();
void init_parser();
int print_ast();

int main() {
  init_tokenizer();
  tokenize();
  init_parser();
  int ast = parse();
  print_ast(ast);
  return 0;
}
