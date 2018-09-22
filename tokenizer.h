#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "base.h"

#define MAX_TOKEN_NUM 100000
#define MAX_TOKEN_LEN 1023
#define EOF -1

#define int_token  0
#define string_token  1
#define char_token   2
#define symbol_token  3
#define operator_token  4
#define comment_token  5
#define other_token  6
#define eof_token  7
#define token_type_num  8

void tokenize();
void init_tokenizer();

extern int* token_type;
extern char** token;
extern int token_num;
extern char** token_type_str;

#endif
