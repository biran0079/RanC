#include "tokenizer.h"

char cur_char = 0;
int* token_type;
char** token;
int token_num = 0;

char* buffer_token;
int buffer_len = 0;
int buffer_token_type;

char** token_type_str;

void init_tokenizer() {
  token_type = malloc(MAX_TOKEN_NUM * WORD_SIZE);
  token = malloc(MAX_TOKEN_NUM * WORD_SIZE);
  buffer_token = malloc(MAX_TOKEN_LEN + 1);
  token_type_str = malloc(token_type_num * WORD_SIZE);
  token_type_str[int_token] = "int";
  token_type_str[string_token] = "string";
  token_type_str[char_token] = "char";
  token_type_str[symbol_token] = "symbol";
  token_type_str[operator_token] = "op";
  token_type_str[comment_token] = "comment";
  token_type_str[other_token] = "other";
  token_type_str[eof_token] = "eof";
}

char peek_char() {
  if (cur_char == 0) {
    cur_char = getchar();
  }
  return cur_char;
}

void append_char(char c) {
  (buffer_token + buffer_len++)[0] = c;
  check(buffer_len <= MAX_TOKEN_LEN, "max token len exceeded\n");
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

int is_space(char c) {
  return c == ' ' || (c == '\t' || c == '\n');
}

int is_digit(char c) {
  return c >= '0' && c <= '9';
}

int is_letter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void check_and_eat_char(char c) {
  check(peek_char() == c, "check_and_eat_char");
  eat_char();
}

void read_single_token() {
  buffer_len = 0;
  while (1) {
    if (peek_char() == '#') {
      // ignore text after #
      ignore_line();
    } else if (is_space(peek_char())) {
      // ignore spaces
      ignore_char();
    } else {
      break;
    }
  }
  if (is_letter(peek_char())) {
    while (is_letter(peek_char()) 
        || is_digit(peek_char()) 
        || peek_char() == '_') {
      eat_char();
    }
    buffer_token_type = symbol_token;
  } else if (is_digit(peek_char())) {
    while (is_digit(peek_char())) {
      eat_char();
    }
    buffer_token_type = int_token;
  } else if (peek_char() == '\'') {
    eat_char();
    if (peek_char() == '\\') {
      eat_char();
    }
    eat_char(); 
    check_and_eat_char('\'');
    buffer_token_type = char_token;
  } else if (peek_char() == '"') {
    eat_char();
    while (peek_char() != '"') {
      if (peek_char() == '\\') {
        eat_char();
      }
      eat_char(); 
    }
    check_and_eat_char('"');
    buffer_token_type = string_token;
  } else if(peek_char() == '|') {
    eat_char();
    check_and_eat_char('|');
    buffer_token_type = operator_token;
  } else if(peek_char() == '&') {
    eat_char();
    check_and_eat_char('&');
    buffer_token_type = operator_token;
  } else if (peek_char() == '=' 
     || peek_char() == '!'
     || peek_char() == '<' 
     || peek_char() == '>') {
    eat_char();
    if (peek_char() == '=') {
      eat_char();
    }
    buffer_token_type = operator_token;
  } else if (peek_char() == '/') {
    eat_char();
    if (peek_char() == '/') {
      eat_char();
      ignore_line();
      buffer_token_type = comment_token;
    } else if (peek_char() == '*') {
      eat_char();
      int state = 0;
      while (state != 2) {
        if (state == 0) {
          if (peek_char() == '*') {
            state = 1;
          } 
        } else {
          if (peek_char() == '/') {
            state = 2;
          } else if (peek_char() != '*') {
            state = 0;
          }
        }
        ignore_char();
      }
      buffer_token_type = comment_token;
    } else if (peek_char() == '=') {
      eat_char();
      buffer_token_type = operator_token;
    } else {
      buffer_token_type = operator_token;
    }
  } else if (strchr("+-*%,", peek_char())) {
    char c = peek_char();
    eat_char();
    if ((c == '+' || c == '-') && peek_char() == c) {
      eat_char();
    } else if ((c == '+' || c == '-' || c == '*') && peek_char() == '=') {
      eat_char();
    }
    buffer_token_type = operator_token;
  } else if (strchr("[](){};", peek_char())) {
    eat_char();
    buffer_token_type = other_token;
  } else if (peek_char() == EOF) {
    buffer_token_type = eof_token; 
  } else {
    check(0, "unknown token");
  }
  append_char(0);
}


void tokenize() {
  int last_token_type = -1;
  while (last_token_type != eof_token) {
    read_single_token();
    token[token_num] = strdup(buffer_token);
    token_type[token_num] = buffer_token_type;
    token_num++;
    check(token_num <= MAX_TOKEN_NUM, "too many tokens\n");
    last_token_type = buffer_token_type;
  }
}
