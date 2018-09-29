#ifndef __AST_PROCESSOR_H__
#define __AST_PROCESSOR_H__

#include "parser.h"
#include "string_map.h"

struct LocalVar {
  struct Node* def;
  int offset; // offset in bytes from the first local var in stack
};

struct Enum {
  int value;
};

struct FunctionParam {
  struct Node* def;
  int offset; // offset in bytes from the first local var in stack
};

struct GlobalSymbolTable {
  struct StringMap* struct_defs;
  struct StringMap* global_var_defs;
  struct StringMap* functions;
  struct StringMap* enums;
  struct StringMap* lsts; // map from function name to LocalSymbolTable
};

struct LocalSymbolTable {
  struct StringMap* local_vars;
  struct StringMap* function_params; 
};

struct ProcessedAst {
  struct Node* ast;
  struct GlobalSymbolTable* gst;
};

void register_struct_def(struct GlobalSymbolTable* gst, struct Node* node);

struct Node* lookup_struct_def(struct GlobalSymbolTable* gst, char* s);

void register_global_var(struct GlobalSymbolTable* gst, struct Node* node);

struct Node* lookup_global_var_def(struct GlobalSymbolTable* gst, char* s);

void register_function(struct GlobalSymbolTable* gst, struct Node* node);

struct Node* lookup_function(struct GlobalSymbolTable* gst, char* s);

void register_enum(struct GlobalSymbolTable* gst, char* s, int n);

struct Enum* lookup_enum(struct GlobalSymbolTable* gst, char* s);

void register_local_var(struct LocalSymbolTable* lst, struct Node* node);

struct LocalVar* lookup_local_var(struct LocalSymbolTable* lst, char* s);

void register_function_params(struct LocalSymbolTable* lst, struct Node* fun);

struct FunctionParam* lookup_function_param(struct LocalSymbolTable* lst, char* s);

struct LocalSymbolTable* get_lst(struct GlobalSymbolTable* gst, char* fun_name);

int local_var_count(struct LocalSymbolTable* lst);

struct ProcessedAst* process(struct Node* root);

#endif
