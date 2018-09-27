#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "base.h"
#include "list.h"

#define MAX_TOKEN_LEN 1023
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
};

struct Token* new_token(enum TokenType type, char* s);

struct List* tokenize();
void init_tokenizer();

extern char** token_type_str;

#endif
