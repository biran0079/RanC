#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "base.h"
#include "list.h"

#define EOF -1

enum TokenType {
  int_token,
  string_token,
  char_token ,
  symbol_token,
  operator_token,
  comment_token,
  other_token,
  eof_token,
  token_type_num,
};

struct Token {
  enum TokenType type;
  char* s;
  int line;
  int start_col; // inclusive
  int end_col;   // exclusive
};

struct TokenizerOutput {
  struct List* tokens;
  struct List* code;
};

struct Token* new_token(enum TokenType type, char* s, int line, int start_col, int end_col);

struct TokenizerOutput* tokenize();
void init_tokenizer();

extern char** token_type_str;

#endif
