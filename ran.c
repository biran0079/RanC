#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int int_token = 0;
int string_token = 1;
int char_token  = 2;
int symbol_token = 3;
int operator_token = 4;
int other_token = 5;

char* token;
int token_len; 
int token_type; 
char* peeked_token = 0;
int peeked_token_type;
char cur_char = 0;

char peek_char() {
  if (cur_char == 0) {
    cur_char = getchar();
  }
  return cur_char;
}

void append_char(char c) {
  token[token_len] = c;
  token_len = token_len + 1;
}

void eat_char() {
  append_char(peek_char());
  cur_char = 0;  
}

void ignore_char() {
  peek_char();
  cur_char = 0; 
}

void ignore_line() {
  while (peek_char() != '\n') {
    ignore_char();
  }
  ignore_char();
}

void ignore_includes() {
  while (peek_char() == '#') {
    ignore_line();
  }
}

void check(int state, char* msg) {
  if (!state) {
    fputs(msg, stderr);
    exit(1);
  }
}

void check_and_ignore_char(char c) {
  check(peek_char() == c, "check_and_ignore_char");
  cur_char = 0;
}

int is_space(char c) {
  return c == ' ' || (c == '\t' || c == '\n');
}

int is_digit(char c) {
  return c >= '0' && c <= '9';
}

int is_letter(char c) {
  return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

void ignore_spaces() {
  while (is_space(peek_char())) { 
    ignore_char();
  } 
}

void eat_quoted_char() {
  if (peek_char() == '\\') {
    ignore_char();
    if (peek_char() == 'n') {
      append_char('\n');
    } else if (peek_char() == 't') {
      append_char('\t');
    } else if (peek_char() == '\\') {
      append_char('\\');
    } else if (peek_char() == '\'') {
      append_char('\'');
    } else if (peek_char() == '\"') {
      append_char('\"');
    } else {
      check(0, "unrecognized escape char");  
    }
    ignore_char();
  } else {
    eat_char(); 
  }
}

int check_and_eat_char(char c) {
  check(peek_char() == c, "check_and_eat_char");
  eat_char();
}

char* read_token() {
  if (peeked_token) {
    char* res = peeked_token;
    peeked_token = 0;
    return res;
  }
  token_len = 0;
  ignore_spaces();
  ignore_includes();
  ignore_spaces();
  if (is_letter(peek_char())) {
    while (is_letter(peek_char()) 
        || is_digit(peek_char()) 
        || peek_char() == '_') {
      eat_char();
    }
    token_type = symbol_token;
  } else if (is_digit(peek_char())) {
    while (is_digit(peek_char())) {
      eat_char();
    }
    token_type = int_token;
  } else if (peek_char() == '\'') {
    ignore_char();
    eat_quoted_char();
    check_and_ignore_char('\'');
    token_type = char_token;
  } else if (peek_char() == '"') {
    ignore_char();
    while (peek_char() != '"') {
      eat_quoted_char();
    }
    ignore_char();
    token_type = string_token;
  } else if(peek_char() == '|') {
    eat_char();
    check_and_eat_char('|');
    token_type = operator_token;
  } else if(peek_char() == '&') {
    eat_char();
    check_and_eat_char('&');
    token_type = operator_token;
  } else if (peek_char() == '=' 
     || peek_char() == '!'
     || peek_char() == '<' 
     || peek_char() == '>') {
    eat_char();
    if (peek_char() == '=') {
      eat_char();
    }
    token_type = operator_token;
  } else if (strchr("+-*,", peek_char())) {
    eat_char();
    token_type = operator_token;
  } else if (strchr("[](){};", peek_char())) {
    eat_char();
    token_type = other_token;
  } else {
    check(0, "unknown token");
  }
  token[token_len] = 0;
  return token;
}

void load_peeked_token() {
  if (!peeked_token) {
    read_token();
    peeked_token = token;
    peeked_token_type = token_type;
  }
}

char* peek_token() {
  load_peeked_token();
  return peeked_token;
}

int peek_token_type() {
  load_peeked_token();
  return peeked_token_type;
}

int matche_token(char* s) {
  if (strcmp(peek_token(), s) == 0
      && (peek_token_type() != string_token
          && peek_token_type() != char_token)) {
    read_token();
    return 1;
  }
  return 0;
}

int end_of_file() {
  ignore_spaces();
  return peek_char() == EOF;
}

void check_and_ignore_token(char* s) {
  check(strcmp(s, read_token()) == 0, "check_and_ignore_token failed");
}

void ignore_type() {
  read_token();
  while (matche_token("*")) { }
}

void process_expr();

void process_expr0() {
  if (matche_token("(")) {
    process_expr();
    check_and_ignore_token(")");
    return;
  }
  int type = peek_token_type();
  if (type == int_token) {
    printf("int %s\n", read_token());
  } else if (type == string_token) {
    printf("string %s\n", read_token());
  } else if (type == char_token) {
    printf("char %s\n", read_token());
  } else if (type == symbol_token) {
    char* symbol = strdup(read_token());
    if (matche_token("(")) {
      printf("call %s\n", symbol);
      while (!matche_token(")")) {
        printf("arg\n");
        process_expr();
        matche_token(",");
      }
    } else if (matche_token("[")) {
      printf("base %s\n", symbol);
      printf("index\n");
      process_expr();
      check_and_ignore_token("]");
    } else {
      printf("identifier %s\n", symbol);
    }
  } else {
    check(0, "unknown token type in expression\n");
  }
}

void process_expr1() {
  process_expr0();
  if (matche_token("*")) {
    printf("mul\n");
    process_expr0();
  }
}

void process_expr2() {
  process_expr1();
  if (matche_token("+")) {
    printf("add\n");
    process_expr1();
  } else if (matche_token("-")) {
    printf("sub\n");
    process_expr1();
  }
} 

void process_expr3() {
  if (matche_token("!")) {
    printf("not\n");
    process_expr3();
  } else {
    process_expr2();
    if (matche_token("==")) {
      printf("eq\n");
      process_expr2();
    } else if (matche_token("!=")) {
      printf("eq\n");
      process_expr2();
    } else if (matche_token("<")) {
      printf("lt");
      process_expr2();
    } else if (matche_token("<=")) {
      printf("le");
      process_expr2();
    } else if (matche_token(">")) {
      printf("gt");
      process_expr2();
    } else if (matche_token(">=")) {
      printf("ge");
      process_expr2();
    }
  }
}

void process_expr4() {
  process_expr3();
  if (matche_token("&&")) {
    printf("and\n");
    process_expr4();
  }
}

void process_expr() {
  process_expr4();
  if (matche_token("||")) {
    printf("or\n");
    process_expr();
  }
}

void process_block();

int is_type(char* s) {
  return strcmp(s, "int") == 0 
      || strcmp(s, "char") == 0
      || strcmp(s, "void") == 0;
}

void process_stmt() {
  if (matche_token("if")) {
    printf("if\n");
    check_and_ignore_token("(");
    process_expr();
    check_and_ignore_token(")");
    printf("then\n");
    process_block();
    if (matche_token("else")) {
      printf("else\n");
      if (strcmp(peek_token(), "if") == 0) {
        process_stmt();
      } else {
        process_block();
      }
    }
    printf("endif\n");
  } else if (matche_token("while")) {
    printf("while\n");
    check_and_ignore_token("(");
    process_expr();
    check_and_ignore_token(")");
    printf("do");
    process_block();
    printf("endwhile\n");
  } else if (matche_token("return")) {
    printf("return\n");
    if (!matche_token(";")) {
      process_expr();
      check_and_ignore_token(";");
    }
    printf("endreturn\n");
  } else {
    if (is_type(peek_token())) {
      ignore_type();
      char* symbol = strdup(read_token());
      printf("local variable %s\n", symbol); 
      if (matche_token("=")) {
        printf("assign\n"); 
        process_expr();
      }
    } else {
      process_expr0();
      if (matche_token("=")) {
        printf("assign\n"); 
        process_expr();
      }
    }
    check_and_ignore_token(";");
    printf("endstmt\n");
  }
}

void process_block() {
  check_and_ignore_token("{");
  while (!matche_token("}")) {
    process_stmt();   
  }
}

void process_decl() {
  ignore_type();
  char* name = strdup(read_token());
  if (matche_token("=")) {
    printf("initialized global variable: %s\n", name);
    process_expr();
    check_and_ignore_token(";");
  } else if (matche_token("(")) {
    printf("function: %s\n", name);
    while (!matche_token(")")) {
      ignore_type();
      printf("param: %s\n", read_token());
      matche_token(",");
    }
    if (strcmp(peek_token(), "{") == 0) {
      process_block();
    } else {
      check_and_ignore_token(";");
    }
  } else if (matche_token(";")) {
    printf("global variable: %s\n", name);
  } else {
    check(0, "illegal declaration syntax");
  }
}

void process_prog() {
  while (!end_of_file()) {
    process_decl();
  }
}

void init() {
  token = malloc(1024);  
  token_len = 0;
}

int main() {
  init();
  process_prog();
  return 0;
}
