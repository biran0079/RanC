#ifndef __CODEGEN_X86_H__
#define __CODEGEN_X86_H__

#include "tokenizer.h"
#include "parser.h"

#define MAX_LOCAL_VARS 100

void init_codegen();
void generate_code(int ast);

#endif
