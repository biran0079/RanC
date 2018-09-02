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
int token_len; int token_type; 
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

void check(int state, char* msg) {
  if (!state) {
    fputs(msg, stderr);
    exit(-1);
  }
}

void check_and_ignore_char(char c) {
  check(peek_char() == c, "check_and_ignore_char");
  cur_char = 0;
}

int match(char* s) {
  return strcmp(token, s) == 0;
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

void read_token() {
  ignore_spaces();
  token_len = 0;
  if (is_letter(peek_char())) {
    while (is_letter(peek_char()) 
        || (is_digit(peek_char()) 
        ||peek_char() == '_')) {
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
  } else if(peek_char() == '=') {
    eat_char();
    if (peek_char() == '=') {
      eat_char();
    }
    token_type = operator_token;
  } else if(peek_char() == '|') {
    eat_char();
    check_and_eat_char('|');
    token_type = operator_token;
  } else if(peek_char() == '&') {
    eat_char();
    check_and_eat_char('&');
    token_type = operator_token;
  } else if(peek_char() == '#') {
    ignore_line();
  } else if (strchr("!+-*,><", peek_char())) {
    eat_char();
    token_type = operator_token;
  } else if (strchr("[](){};", peek_char())) {
    eat_char();
    token_type = other_token;
  } else {
    check(0, "unkonwn token");  
  }
  token[token_len] = 0;
}

int end_of_file() {
  ignore_spaces();
  return peek_char() == EOF;
}

void process_prog() {
  while (!end_of_file()) {
    read_token();
    printf("%s %d\n", token, token_type);
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
