#include "ast_processor.h"
#include "parser.h"

enum BaseType _get_data_type(struct Node* type_node) {
  switch (type_node->type) {
    case ptr_type_node: return PTR_TYPE;
    case int_type_node: return INT_TYPE;
    case char_type_node: return CHAR_TYPE;
    case void_type_node: return VOID_TYPE;
    case enum_type_node: return ENUM_TYPE;
    case struct_type_node: return STRUCT_TYPE;
    case function_type_node: return FUNCTION_TYPE;
    default: check(0, "unknown type");
  }
}

struct CType* new_ctype_with_base_type(enum BaseType type) {
  struct CType* res = malloc(sizeof(struct CType));
  res->type = type;
  switch (type) {
    case INT_TYPE:
    case PTR_TYPE:
    case FUNCTION_TYPE:
    case ENUM_TYPE:
      res->size = WORD_SIZE;
      break;
    case CHAR_TYPE:
    case VOID_TYPE:
      res->size = 1;
      break;
    case STRUCT_TYPE:
      res->size = 0; // caller is responsible to set correct size
      break;
  }
  return res;
}

struct FunctionTypeData* new_fun_type_data(struct GlobalSymbolTable* gst, struct Node* return_type, struct Node* params_type);

struct CType* new_ctype_from_type_node(struct GlobalSymbolTable* gst, struct Node* type_node) {
  check(is_type_node(type_node), "new_ctype only applies to type node");
  struct CType* res = new_ctype_with_base_type(_get_data_type(type_node));
  switch (res->type) {
    case PTR_TYPE:
      res->ptr_data = new_ctype_from_type_node(gst, get_child(type_node, 0));
      break;
    case FUNCTION_TYPE: 
      res->fun_data = new_fun_type_data(gst, get_child(type_node, 0), get_child(type_node, 1));
      break;
    case STRUCT_TYPE: {
      struct CType* struct_type = lookup_struct(gst, get_symbol(get_child(type_node, 0)));
      check(struct_type, "struct not found");
      return struct_type;
    }
  }
  return res;
}

struct FunctionTypeData* new_fun_type_data(struct GlobalSymbolTable* gst, struct Node* return_type, struct Node* params_type) {
  struct FunctionTypeData* fun = malloc(sizeof(struct FunctionTypeData));
  fun->return_type = new_ctype_from_type_node(gst, return_type);
  fun->param_types = new_list();
  for (int i = 0; i < child_num(params_type); i++) {
    struct Node* param_type = get_child(params_type, i);
    list_add(fun->param_types, new_ctype_from_type_node(gst, get_child(param_type, 0)));
  }
  return fun;
}

struct StructTypeData* new_struct_type_data(struct GlobalSymbolTable* gst, struct Node* node) {
  check(node->type == struct_node, "struct node definition expected");
  struct StructTypeData* res = malloc(sizeof(struct StructTypeData));
  res->name = get_symbol(get_child(node, 0));
  res->members = new_string_map();
  int offset = 0;
  for (int i = 1; i < child_num(node); i++) {
    struct Node* member = get_child(node, i);
    char* member_name = get_symbol(get_child(member, 1));
    struct StructMemberTypeData* member_data = malloc(sizeof(struct StructMemberTypeData));
    member_data->type = new_ctype_from_type_node(gst, get_child(member, 0));
    member_data->offset = offset;
    string_map_put(res->members, member_name, member_data);
    offset += member_data->type->size;
  }
  return res;
}

struct GlobalSymbolTable* new_gst(){
  struct GlobalSymbolTable* res = malloc(sizeof(struct GlobalSymbolTable));
  res->struct_defs = new_string_map();
  res->global_var_defs = new_string_map();
  res->functions = new_string_map();
  res->enums = new_string_map();
  res->lsts = new_string_map();
  return res;
}

struct LocalSymbolTable* new_lst() {
  struct LocalSymbolTable* res = malloc(sizeof(struct LocalSymbolTable));
  res->local_vars = new_string_map();
  res->function_params = new_string_map();
  return res;
}

// register empty struct ctype;
void register_struct(struct GlobalSymbolTable* gst, struct Node* node) {
  char* struct_name = get_symbol(get_child(node, 0));
  struct CType* res = new_ctype_with_base_type(STRUCT_TYPE);
  string_map_put(gst->struct_defs, struct_name, res);
  res->size = 0;
}

// set struct_data and size for struct CType
void set_struct_data(struct GlobalSymbolTable* gst, struct Node* node) {
  char* struct_name = get_symbol(get_child(node, 0));
  struct CType* res = lookup_struct(gst, struct_name);
  res->struct_data = new_struct_type_data(gst, node);
  for (int i = 1; i < child_num(node); i++) {
    struct Node* member = get_child(node, i);
    char* member_name = get_symbol(get_child(member, 1));
    struct StructMemberTypeData* member_data = string_map_get(res->struct_data->members, member_name);
    check(member_data, "member not found");
    res->size += max(WORD_SIZE, member_data->type->size);
  }
}

struct CType* lookup_struct(struct GlobalSymbolTable* gst, char* s) {
  return string_map_get(gst->struct_defs, s);
}

void register_global_var(struct GlobalSymbolTable* gst, struct Node* node) {
  char* name = get_symbol(get_child(node, 1));
  struct CType* res = new_ctype_from_type_node(gst, get_child(node, 0));
  string_map_put(gst->global_var_defs, name, res);
}

struct CType* lookup_global_var_def(struct GlobalSymbolTable* gst, char* s) {
  return string_map_get(gst->global_var_defs, s);
}

void register_function(struct GlobalSymbolTable* gst, struct Node* node) {
  struct CType* res = new_ctype_with_base_type(FUNCTION_TYPE);
  struct Node* return_type = get_child(node, 0);
  char* name = get_symbol(get_child(node, 1));
  struct Node* params_type = get_child(node, 2);
  res->fun_data = new_fun_type_data(gst, return_type, params_type);
  string_map_put(gst->functions, name, res);
}

struct CType* lookup_function(struct GlobalSymbolTable* gst, char* s) {
  return string_map_get(gst->functions, s);
}

void register_enum(struct GlobalSymbolTable* gst, char* s, int n) {
  struct Enum* e = malloc(sizeof(struct Enum));
  e->value = n;
  string_map_put(gst->enums, s, e);
}

struct Enum* lookup_enum(struct GlobalSymbolTable* gst, char* s) {
  return string_map_get(gst->enums, s);
}

void register_local_var(struct GlobalSymbolTable* gst, struct LocalSymbolTable* lst, struct Node* node) {
  char* name = get_symbol(get_child(node, 1));
  if (!string_map_contains(lst->local_vars, name)) {
    // TODO disallow defining same var twice once scoping is supported
    struct LocalVar* v = malloc(sizeof(struct LocalVar));
    v->ctype = new_ctype_from_type_node(gst, get_child(node, 0));
    v->offset = WORD_SIZE * string_map_size(lst->local_vars);
    string_map_put(lst->local_vars, name, v);
  }
}

struct LocalVar* lookup_local_var(struct LocalSymbolTable* lst, char* s) {
  if (!lst) {
    return 0;
  }
  return string_map_get(lst->local_vars, s);
}

void register_function_params(struct GlobalSymbolTable* gst, struct LocalSymbolTable* lst, struct Node* params) {
  for (int i = 0; i < child_num(params); i++) {
    struct Node* param = get_child(params, i);
    char* name = get_symbol(get_child(param, 1));
    struct FunctionParam* value = malloc(sizeof(struct FunctionParam));
    value->ctype = new_ctype_from_type_node(gst, get_child(param, 0));
    value->offset = i * WORD_SIZE;
    check(!string_map_get(lst->function_params, name), "duplicate function param name");
    string_map_put(lst->function_params, name, value);
  }
}

struct FunctionParam* lookup_function_param(struct LocalSymbolTable* lst, char* s) {
  if (!lst) {
    return 0;
  }
  return string_map_get(lst->function_params, s);
}

// search for all var_decl_node and var_init_node in subtree
void register_local_vars(struct GlobalSymbolTable* gst, struct LocalSymbolTable* lst, struct Node* root) {
  enum NodeType t = root->type;
  if (t == var_decl_node || t == var_init_node) {
    register_local_var(gst, lst, root);
  } else {
    for (int i = 0; i < child_num(root); i++) {
      register_local_vars(gst, lst, get_child(root, i));
    }
  }
}

struct LocalSymbolTable* get_lst(struct GlobalSymbolTable* gst, char* fun_name) {
  return string_map_get(gst->lsts, fun_name);
}

int local_var_count(struct LocalSymbolTable* lst) {
  return string_map_size(lst->local_vars);
}

struct CType* lookup_symbol_ctype(struct GlobalSymbolTable* gst, struct LocalSymbolTable* lst, char* s) {
  struct LocalVar* local = lookup_local_var(lst, s);
  if (local) {
    return local->ctype;
  }
  struct FunctionParam* param = lookup_function_param(lst, s);
  if (param) {
    return param->ctype;
  }
  struct CType* function = lookup_function(gst, s);
  if (function) {
    return function;
  }
  struct CType* global = lookup_global_var_def(gst, s);
  if (global) {
    return global;
  }
  if (lookup_enum(gst, s)) {
    return new_ctype_with_base_type(ENUM_TYPE);
  }
  return 0;
}

struct CType* get_expr_ctype(struct GlobalSymbolTable* gst, struct LocalSymbolTable* lst, struct Node* expr) {
  switch (expr->type) {
    case assignment_node:
    case add_eq_node:
    case sub_eq_node:
    case mul_eq_node:
    case div_eq_node:
    case or_node:
    case and_node:
    case not_node:
    case eq_node:
    case ne_node:
    case lt_node:
    case gt_node:
    case le_node:
    case ge_node:
    case add_node:
    case sub_node:
    case mul_node:
    case div_node:
    case mod_node:
    case negative_node:
    case inc_prefix_node:
    case inc_suffix_node:
    case dec_prefix_node:
    case dec_suffix_node:
      return get_expr_ctype(gst, lst, get_child(expr, 0));
    case string_node: {
      struct CType* res = new_ctype_with_base_type(PTR_TYPE);
      res->ptr_data = new_ctype_with_base_type(CHAR_TYPE);
      return res;
    }
    case int_node:
    case sizeof_node:
    case char_node:
      // char literal is treated the same as integer
      return new_ctype_with_base_type(INT_TYPE);
    case symbol_node:{
      struct CType* res = lookup_symbol_ctype(gst, lst, get_symbol(expr));
      check(res, "symbol not found");
      return res;
    }
    case dereference_node:
    case access_node: {
      struct CType* base = get_expr_ctype(gst, lst, get_child(expr, 0));
      check(base->type == PTR_TYPE, "illegal array accessing");
      return base->ptr_data;
    }
    case call_node: {
      struct CType* fun = get_expr_ctype(gst, lst, get_child(expr, 0));
      check(fun->type == FUNCTION_TYPE, "illegal function call");
      return fun->fun_data->return_type;
    }
    case ternary_condition_node:
      return get_expr_ctype(gst, lst, get_child(expr, 1));
    case address_of_node: {
      struct CType* res = new_ctype_with_base_type(PTR_TYPE);
      res->ptr_data = get_expr_ctype(gst, lst, get_child(expr, 0));
      return res;
    }
    case struct_access_node: {
      struct CType* struct_type = get_expr_ctype(gst, lst, get_child(expr, 0));
      check(struct_type->type == STRUCT_TYPE, ". only works for struct");
      char* member_name = get_symbol(get_child(expr, 1));
      struct StructMemberTypeData* member_data = string_map_get(struct_type->struct_data->members, member_name);
      check(member_data, "member not found");
      return member_data->type;
    }
    case struct_ptr_access_node: {
      struct CType* ptr = get_expr_ctype(gst, lst, get_child(expr, 0));
      check(ptr->type == PTR_TYPE, "-> only works for struct pointer");
      struct CType* struct_type = ptr->ptr_data;
      check(struct_type->type == STRUCT_TYPE, "-> only works for struct pointer");
      char* member_name = get_symbol(get_child(expr, 1));
      struct StructMemberTypeData* member_data = string_map_get(struct_type->struct_data->members, member_name);
      check(member_data, "member not found");
      return member_data->type;
    }
    default:
      check(0, "unknown expr node type");
  }
}

struct ProcessedAst* process(struct Node* root) {
  struct ProcessedAst* res = malloc(sizeof(struct ProcessedAst));
  res->ast = root;
  res->gst = new_gst();
  for (int i = 0; i < child_num(root); i++) {
    struct Node* cur = get_child(root, i);
    switch (cur->type) {
      case var_decl_node:
      case var_init_node:
      case extern_var_decl_node:
        register_global_var(res->gst, cur);
        break;
      case enum_node: {
        int value = 0;
        // skip enum name
        for (int j = 1; j < child_num(cur); j++) {
          struct Node* val_node = get_child(cur, j);
          char* key = get_symbol(get_child(val_node, 0));
          if (child_num(val_node) == 2) {
            value = get_int(get_child(val_node, 1));
          }
          register_enum(res->gst, key, value++);
        }
        break;
      }
      case struct_node:
        register_struct(res->gst, cur);
        break;
      case function_decl_node: 
        register_function(res->gst, cur);
        break;
      case function_impl_node: {
        register_function(res->gst, cur);
        struct LocalSymbolTable* lst = new_lst();
        char* name = get_symbol(get_child(cur, 1));
        string_map_put(res->gst->lsts, name, lst);
        register_function_params(res->gst, lst, get_child(cur, 2));
        register_local_vars(res->gst, lst, get_child(cur, 3));
        break;
      }
    }
  }
  // struct member data is set separately after registeration becase 
  // struct member allows cross reference in declaration
  for (int i = 0; i < child_num(root); i++) {
    struct Node* cur = get_child(root, i);
    if (cur->type == struct_node) {
      set_struct_data(res->gst, cur);
    }
  }
  return res;
}

int get_struct_member_offset(struct GlobalSymbolTable* gst, struct CType* struct_type, char* member_name) {
  check(struct_type->type == STRUCT_TYPE, "expect struct type");
  struct StructMemberTypeData* member = string_map_get(struct_type->struct_data->members, member_name);
  check(member, "member does not exist");
  return member->offset;
}

