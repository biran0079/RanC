#include "ast_processor.h"

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

void register_struct_def(struct GlobalSymbolTable* gst, struct Node* node) {
  char* struct_name = get_symbol(get_child(node, 0));
  string_map_put(gst->struct_defs, struct_name, node);
}

struct Node* lookup_struct_def(struct GlobalSymbolTable* gst, char* s) {
  return string_map_get(gst->struct_defs, s);
}

void register_global_var(struct GlobalSymbolTable* gst, struct Node* node) {
  char* name = get_symbol(get_child(node, 1));
  printf("#register %s\n", name);
  string_map_put(gst->global_var_defs, name, node);
}

struct Node* lookup_global_var_def(struct GlobalSymbolTable* gst, char* s) {
  printf("#lookup %s %p\n", s, string_map_get(gst->global_var_defs, s));
  return string_map_get(gst->global_var_defs, s);
}

void register_function(struct GlobalSymbolTable* gst, struct Node* node) {
  char* name = get_symbol(get_child(node, 1));
  string_map_put(gst->functions, name, node);
}

struct Node* lookup_function(struct GlobalSymbolTable* gst, char* s) {
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

void register_local_var(struct LocalSymbolTable* lst, struct Node* node) {
  char* name = get_symbol(get_child(node, 1));
  if (!string_map_contains(lst->local_vars, name)) {
    // TODO disallow defining same var twice once scoping is supported
    struct LocalVar* v = malloc(sizeof(struct LocalVar));
    v->def = node;
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

void register_function_params(struct LocalSymbolTable* lst, struct Node* params) {
  for (int i = 0; i < child_num(params); i++) {
    struct Node* def = get_child(params, i);
    char* name = get_symbol(get_child(def, 1));
    struct FunctionParam* value = malloc(sizeof(struct FunctionParam));
    value->def = def;
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
void register_local_vars(struct LocalSymbolTable* lst, struct Node* root) {
  for (int i = 0; i < child_num(root); i++) {
    struct Node* cur = get_child(root, i);
    enum NodeType t = cur->type;
    if (t == var_decl_node || t == var_init_node) {
      register_local_var(lst, cur);
    } else {
      register_local_vars(lst, cur);
    }
  }
}

struct LocalSymbolTable* get_lst(struct GlobalSymbolTable* gst, char* fun_name) {
  return string_map_get(gst->lsts, fun_name);
}

int local_var_count(struct LocalSymbolTable* lst) {
  return string_map_size(lst->local_vars);
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
        register_struct_def(res->gst, cur);
        break;
      case function_decl_node: 
        register_function(res->gst, cur);
        break;
      case function_impl_node: {
        register_function(res->gst, cur);
        struct LocalSymbolTable* lst = new_lst();
        char* name = get_symbol(get_child(cur, 1));
        string_map_put(res->gst->lsts, name, lst);
        register_function_params(lst, get_child(cur, 2));
        register_local_vars(lst, get_child(cur, 3));
        break;
      }
    }
  }
  return res;
}
