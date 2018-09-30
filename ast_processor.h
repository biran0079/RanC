#ifndef __AST_PROCESSOR_H__
#define __AST_PROCESSOR_H__

#include "parser.h"
#include "string_map.h"

struct Enum {
  int value;
};

struct LocalVar {
  struct CType* ctype;
  int offset; // offset in bytes from the first local var in stack
};

struct FunctionParam {
  struct CType* ctype;
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

enum BaseType {
  INT_TYPE,
  CHAR_TYPE,
  VOID_TYPE,
  PTR_TYPE,
  FUNCTION_TYPE,
  STRUCT_TYPE,
  ENUM_TYPE,
};

struct FunctionTypeData {
  struct CType* return_type;
  struct List* param_types; 
};

struct StructMemberTypeData {
  struct CType* type;
  int offset; // offset in bytes from the beginning of struct 
};

struct StructTypeData {
  char* name; // struct name
  struct StringMap* members;
};

struct CType {
  enum BaseType type;
  int size; // bytes

  struct CType* ptr_data;         // ptr will store data type this pointer points to if type is PTR_TYPE
  struct FunctionTypeData* fun_data;  // fun will be set if type is FUNCTION_TYPE
  struct StructTypeData* struct_data; // stru is set if type is STRUCT_TYPE
};

struct CType* new_ctype(struct GlobalSymbolTable* gst, struct Node* type_node);

struct CType* lookup_struct(struct GlobalSymbolTable* gst, char* s);

struct CType* lookup_global_var_def(struct GlobalSymbolTable* gst, char* s);

struct CType* lookup_function(struct GlobalSymbolTable* gst, char* s);

struct Enum* lookup_enum(struct GlobalSymbolTable* gst, char* s);

struct LocalVar* lookup_local_var(struct LocalSymbolTable* lst, char* s);

struct FunctionParam* lookup_function_param(struct LocalSymbolTable* lst, char* s);

struct LocalSymbolTable* get_lst(struct GlobalSymbolTable* gst, char* fun_name);

int local_var_count(struct LocalSymbolTable* lst);

int size_of_ctype(struct GlobalSymbolTable* gst, struct CType* ctype);

struct ProcessedAst* process(struct Node* root);

struct CType* get_expr_ctype(struct GlobalSymbolTable* gst, struct LocalSymbolTable* lst, struct Node* expr);

struct CType* new_ctype_from_type_node(struct GlobalSymbolTable* gst, struct Node* type_node);

int get_struct_member_offset(struct GlobalSymbolTable* gst, struct CType* struct_type, char* member_name);

#endif
