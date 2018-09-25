#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "base.h"

#define MAX_TOKEN_NUM 100000
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

void tokenize();
void init_tokenizer();

extern int* token_type;
extern char** token;
extern int token_num;
extern char** token_type_str;

#endif
