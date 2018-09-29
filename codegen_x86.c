#include "codegen_x86.h"

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

struct StringMap* struct_defs;
struct StringMap* global_var_defs;
struct StringMap* functions;
struct StringMap* local_vars;
struct StringMap* enums;
// Params node for current function.
// Used for param look up when generating code for function body.
struct StringMap* function_params; 

int in_function;

// Label for function epilog. Needed for return statement.
int return_label;

// Label for breaking current loop.
int break_label;
// Label for continuing current loop.
int continue_label;

int tmp_label_count = 0;

int new_temp_label() {
  return tmp_label_count++;
}

// reserve n consecutive tmp labels, return the first one 
int reserve_tmp_labels(int n) {
  int res = tmp_label_count;
  tmp_label_count += n;
  return res;
}

char* get_symbol(struct Node* cur) {
  check(cur->type == symbol_node, "get_symbol");
  return cur->payload;
}

int get_int(struct Node* cur) {
  check(cur->type == int_node, "get_int");
  return atoi(cur->payload);
}

char* get_string(struct Node* cur) {
  check(cur->type == string_node, "get_string");
  return cur->payload;
}

void register_struct_def(struct Node* node) {
  char* struct_name = get_symbol(get_child(node, 0));
  string_map_put(struct_defs, struct_name, node);
}

struct Node* lookup_struct_def(char* s) {
  return string_map_get(struct_defs, s);
}

void register_global_var(struct Node* node) {
  char* name = get_symbol(get_child(node, 1));
  string_map_put(global_var_defs, name, node);
}

struct Node* lookup_global_var_def(char* s) {
  return string_map_get(global_var_defs, s);
}

struct Node* lookup_function(char* s) {
  return string_map_get(functions, s);
}

void register_function(struct Node* node) {
  char* name = get_symbol(get_child(node, 1));
  string_map_put(functions, name, node);
}

void register_enum(char* s, int n) {
  struct Enum* e = malloc(sizeof(struct Enum));
  e->value = n;
  string_map_put(enums, s, e);
}

struct Enum* lookup_enum(char* s) {
  return string_map_get(enums, s);
}

void register_local_var(struct Node* node) {
  char* name = get_symbol(get_child(node, 1));
  if (!string_map_contains(local_vars, name)) {
    // TODO disallow defining same var twice once scoping is supported
    struct LocalVar* v = malloc(sizeof(struct LocalVar));
    v->def = node;
    v->offset = WORD_SIZE * string_map_size(local_vars);
    string_map_put(local_vars, name, v);
  }
}

struct LocalVar* lookup_local_var(char* s) {
  if (!in_function) {
    return 0;
  }
  return string_map_get(local_vars, s);
}

void register_function_params(struct Node* fun) {
  struct Node* params = get_child(fun, 2);
  for (int i = 0; i < child_num(params); i++) {
    struct Node* def = get_child(params, i);
    char* name = get_symbol(get_child(def, 1));
    struct FunctionParam* value = malloc(sizeof(struct FunctionParam));
    value->def = def;
    value->offset = i * WORD_SIZE;
    check(!string_map_get(function_params, name), "duplicate function param name");
    string_map_put(function_params, name, value);
  }
}

struct FunctionParam* lookup_function_param(char* s) {
  if (!in_function) {
    return 0;
  }
  return string_map_get(function_params, s);
}

// search for all var_decl_node and var_init_node in subtree
void register_local_vars(struct Node* root) {
  for (int i = 0; i < child_num(root); i++) {
    struct Node* cur = get_child(root, i);
    enum NodeType t = cur->type;
    if (t == var_decl_node || t == var_init_node) {
      register_local_var(cur);
    } else {
      register_local_vars(cur);
    }
  }
}

char* get_set_cmp_inst(enum NodeType type) {
  if (type == eq_node) {
    return "sete";
  } else if (type == ne_node) {
    return "setne";
  } else if (type == lt_node) {
    return "setl";
  } else if (type == le_node) {
    return "setle";
  } else if (type == gt_node) {
    return "setg";
  } else if (type == ge_node) {
    return "setge";
  }
  return 0;
}

void generate_expr(struct Node* expr);

int size_of_type(struct Node* type_node) {
  int t = type_node->type;
  if (t == int_type_node || t == function_type_node || t == ptr_type_node || t == enum_type_node) {
    return WORD_SIZE;
  } else if (t == char_type_node || t == void_type_node) {
    return 1;
  } else if (t == struct_type_node) {
    char* name = get_symbol(get_child(type_node, 0));
    struct Node* node = lookup_struct_def(name);
    check(node, "unknown struct");
    int res = 0;
    for (int i = 1; i < child_num(node); i++) {
      struct Node* decl = get_child(node, i);
      res += max(WORD_SIZE, size_of_type(get_child(decl, 0)));
    }
    return res;
  }
  check(0, "unknown type node");
}

struct Node* get_symbol_type_node(char* s) {
  struct LocalVar* local = lookup_local_var(s);
  if (local) {
    return get_child(local->def, 0);
  }
  struct FunctionParam* param = lookup_function_param(s);
  if (param) {
    return get_child(param->def, 0);
  }
  struct Node* function = lookup_function(s);
  if (function) {
    struct Node* res = new_node(function_type_node);
    append_child(res, get_child(function, 0));
    append_child(res, get_child(function, 2));
    return res;
  }
  if (lookup_enum(s)) {
    return new_node(enum_type_node);
  }
  struct Node* node = lookup_global_var_def(s);
  if (node) {
    return get_child(node, 0);
  }
  return 0;
}

struct Node* lookup_struct_member_type_node(struct Node* struct_type, char* name) {
  check(struct_type->type == struct_type_node, "lookup_struct_member_type_node");
  struct Node* def = lookup_struct_def(get_symbol(get_child(struct_type, 0)));
  check(def, "struct def not found");
  for (int i = 1; i < child_num(def); i++) {
    struct Node* t = get_child(def, i);
    if (!strcmp(name, get_symbol(get_child(t, 1)))) {
      return get_child(t, 0);
    }
  }
  check(0, "struct member not found");
}

int get_struct_member_offset(struct Node* struct_type, char* name) {
  check(struct_type->type == struct_type_node, "get_struct_member_offset");
  struct Node* def = lookup_struct_def(get_symbol(get_child(struct_type, 0)));
  check(def, "struct def not found");
  int res = 0;
  for (int i = 1; i < child_num(def); i++) {
    struct Node* t = get_child(def, i);
    if (!strcmp(name, get_symbol(get_child(t, 1)))) {
      return res;
    }
    res += size_of_type(get_child(t, 0));
  }
  check(0, "struct member not found");
}

struct Node* get_expr_type_node(struct Node* expr) {
  enum NodeType t = expr->type;
  if (t == assignment_node || t == add_eq_node || t == sub_eq_node || t == mul_eq_node || t == div_eq_node
      || t == or_node || t == and_node || t == not_node || get_set_cmp_inst(t)
      || t == add_node || t == sub_node || t == mul_node || t == div_node || t == mod_node
      || t == negative_node || t == inc_prefix_node || t == inc_suffix_node || t == dec_prefix_node || t == dec_suffix_node) {
    return get_expr_type_node(get_child(expr, 0));
  } else if (t == int_node) {
    return new_node(int_type_node);
  } else if (t == string_node) {
    struct Node* res = new_node(ptr_type_node);
    append_child(res, new_node(char_type_node));
    return res;
  } else if (t == char_node) {
    return new_node(char_type_node);
  } else if(t == symbol_node) {
    struct Node* type_node = get_symbol_type_node(get_symbol(expr));
    check(type_node, "unknown symbol");
    return type_node;
  } else if(t == access_node) {
    struct Node* base = get_expr_type_node(get_child(expr, 0));
    check(base->type == ptr_type_node, "illegal array accessing");
    return get_child(base, 0);
  } else if (t == call_node) {
    struct Node* fun = get_expr_type_node(get_child(expr, 0));
    check(fun->type == function_type_node, "illegal function call");
    return get_child(fun, 0);
  } else if (t == ternary_condition_node) {
    return get_expr_type_node(get_child(expr, 1));
  } else if (t == address_of_node) {
    struct Node* res = new_node(ptr_type_node);
    append_child(res, get_expr_type_node(get_child(expr, 0)));
    return res;
  } else if (t == dereference_node) {
    struct Node* tmp = get_expr_type_node(get_child(expr, 0));
    check(tmp->type == ptr_type_node, "only ptr_type can be dereferenced");
    return get_child(tmp, 0);
  } else if (t == struct_access_node) {
    struct Node* tmp = get_expr_type_node(get_child(expr, 0));
    check(tmp->type == struct_type_node, ". only works for struct");
    return lookup_struct_member_type_node(tmp, get_symbol(get_child(expr, 1)));
  } else if (t == struct_ptr_access_node) {
    struct Node* ptr = get_expr_type_node(get_child(expr, 0));
    check(ptr->type == ptr_type_node, "-> only works for struct pointer");
    struct Node* tmp = get_child(ptr, 0);
    check(tmp->type == struct_type_node, "-> only works for struct pointer");
    return lookup_struct_member_type_node(tmp, get_symbol(get_child(expr, 1)));
  } else {
    check(0, "unknown expr node type");
  }
}

int get_expr_type_size(struct Node* expr) {
  return size_of_type(get_expr_type_node(expr));
}

int is_type_node(struct Node* expr) {
  int t = expr->type;
  return t == ptr_type_node
      || t == int_type_node
      || t == char_type_node
      || t == void_type_node
      || t == enum_type_node
      || t == function_type_node
      || t == struct_type_node;
}

int get_ptr_step_size(struct Node* expr) {
  struct Node* type_node = get_expr_type_node(expr);
  if (type_node->type == ptr_type_node) {
    return size_of_type(get_child(type_node, 0));
  }
  return 1;
}

void generate_expr_internal(struct Node* expr, int lvalue) {
  enum NodeType t = expr->type;
  int end_label;
  if (t == assignment_node) {
    generate_expr_internal(get_child(expr, 0), 1);
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    printf("pop ebx\n");
    printf("mov dword ptr [ebx], eax\n");
  } else if (t == add_eq_node) {
    generate_expr_internal(get_child(expr, 0), 1);
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    int step = get_ptr_step_size(get_child(expr, 0));
    if (step > 1) {
      printf("imul eax, %d\n", step);
    }
    printf("pop ebx\n");
    printf("add [ebx], eax\n");
    printf("mov eax, [ebx]\n");
  } else if (t == sub_eq_node) {
    generate_expr_internal(get_child(expr, 0), 1);
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    int step = get_ptr_step_size(get_child(expr, 0));
    if (step > 1) {
      printf("imul eax, %d\n", step);
    }
    printf("pop ebx\n");
    printf("sub [ebx], eax\n");
    printf("mov eax, [ebx]\n");
  } else if (t == mul_eq_node) {
    generate_expr_internal(get_child(expr, 0), 1);
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    printf("pop ebx\n");
    printf("imul eax, dword ptr [ebx]\n");
    printf("mov [ebx], eax\n");
  } else if (t == div_eq_node) {
    generate_expr_internal(get_child(expr, 0), 1);
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    printf("mov ebx, eax\n");
    printf("mov eax, dword ptr [esp]\n");
    printf("mov eax, [eax]\n");
    printf("cdq\n");
    printf("idiv ebx\n");
    printf("pop ebx\n");
    printf("mov [ebx], eax\n");
  } else if (t == or_node) {
    end_label = new_temp_label();
    for (int i = 0; i < child_num(expr); i++) {
      generate_expr(get_child(expr, i));
      printf("cmp eax, 0\n");
      printf("jnz _%d\n", end_label);
    }
    printf("_%d:\n", end_label);
  } else if (t == and_node) {
    end_label = new_temp_label();
    for (int i = 0; i < child_num(expr); i++) {
      generate_expr(get_child(expr, i));
      printf("cmp eax, 0\n");
      printf("jz _%d\n", end_label);
    }
    printf("_%d:\n", end_label);
  } else if (t == not_node) {
    generate_expr(get_child(expr, 0));
    printf("cmp eax, 0\n");
    printf("mov eax, 0\n");
    printf("sete al\n");
  } else if (get_set_cmp_inst(t)) {
    generate_expr(get_child(expr, 0));
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    printf("pop ebx\n");
    printf("cmp ebx, eax\n");
    printf("mov eax, 0\n");
    printf("%s al\n", get_set_cmp_inst(t));
  } else if (t == add_node) {
    generate_expr(get_child(expr, 0));
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    int step = get_ptr_step_size(get_child(expr, 0));
    if (step > 1) {
      printf("imul eax, %d\n", step);
    }
    printf("pop ebx\n");
    printf("add eax, ebx\n");
  } else if (t == sub_node) {
    generate_expr(get_child(expr, 0));
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    int step = get_ptr_step_size(get_child(expr, 0));
    if (step > 1) {
      printf("imul eax, %d\n", step);
    }
    printf("pop ebx\n");
    printf("sub ebx, eax\n");
    printf("mov eax, ebx\n");
  } else if (t == mul_node) {
    generate_expr(get_child(expr, 0));
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    printf("pop ebx\n");
    printf("imul eax, ebx\n");
  } else if (t == div_node || t == mod_node) {
    generate_expr(get_child(expr, 0));
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    printf("mov ebx, eax\n");
    printf("pop eax\n");
    printf("cdq\n");
    printf("idiv ebx\n");
    if (t == mod_node) {
      printf("mov eax, edx\n");
    }
  } else if (t == int_node || t == char_node) {
    printf("mov eax, %s\n", expr->payload);
  } else if (t == string_node) {
    int string_label = new_temp_label();
    printf(".section .rodata\n");
    printf("_%d:\n", string_label);
    printf(".ascii %s\n", get_string(expr));
    printf(".byte 0\n");
    printf(".section .text\n");
    printf("mov eax, offset _%d\n", string_label);
  } else if(t == symbol_node) {
    char* name = get_symbol(expr);
    struct LocalVar* local = lookup_local_var(name);
    struct FunctionParam* param = lookup_function_param(name);
    struct Enum* enum_entry = lookup_enum(name);
    char* op;
    if (lvalue) {
      op = "lea";
    } else {
      op = "mov";
    }
    if (local) {
      printf("%s eax, [ebp-%d]\n", op, WORD_SIZE + local->offset);
    } else if (param) {
      printf("%s eax, [ebp+%d]\n", op, 2 * WORD_SIZE + param->offset);
    } else if (enum_entry) {
      check(!lvalue, "enum cannot be lvalue\n");
      printf("%s eax, %d\n", op, enum_entry->value);
    } else {
      printf("%s eax, [%s]\n", op, name);
    }
  } else if(t == access_node) {
    int type_size = get_expr_type_size(expr);
    generate_expr(get_child(expr, 0));
    printf("push eax\n");
    generate_expr(get_child(expr, 1));
    printf("mov ebx, eax\n");
    printf("pop ebx\n");
    char* op;
    if (lvalue) {
      op = "lea";
    } else {
      op = "mov";
    }
    printf("%s eax, dword ptr [eax * %d + ebx]\n", op, type_size);
  } else if(t == call_node) {
    struct Node* fun = get_child(expr, 0);
    check(fun->type == symbol_node, "function has to be a symbol");
    char* fname = get_symbol(fun);
    struct Node* args = get_child(expr, 1);
    for (int i = child_num(args) - 1; i >= 0; i--) {
      generate_expr(get_child(args, i));
      printf("push eax\n");
    }
    printf("call %s\n", fname);
    printf("add esp, %d\n", child_num(args) * WORD_SIZE);
  } else if (t == negative_node) {
    generate_expr(get_child(expr, 0));
    printf("not eax\n");
    printf("add eax, 1\n");
  } else if (t == inc_prefix_node) {
    generate_expr_internal(get_child(expr, 0), 1);
    printf("add dword ptr [eax], %d\n", get_ptr_step_size(get_child(expr, 0)));
    printf("mov eax, dword ptr [eax]\n");
  } else if (t == dec_prefix_node) {
    generate_expr_internal(get_child(expr, 0), 1);
    printf("sub dword ptr [eax], %d\n", get_ptr_step_size(get_child(expr, 0)));
    printf("mov eax, dword ptr [eax]\n");
  } else if (t == inc_suffix_node) {
    int step = get_ptr_step_size(get_child(expr, 0));
    generate_expr_internal(get_child(expr, 0), 1);
    printf("add dword ptr [eax], %d\n", step);
    printf("mov eax, dword ptr [eax]\n");
    printf("sub eax, %d\n", step);
  } else if (t == dec_suffix_node) {
    int step = get_ptr_step_size(get_child(expr, 0));
    generate_expr_internal(get_child(expr, 0), 1);
    printf("sub dword ptr [eax], %d\n", step);
    printf("mov eax, dword ptr [eax]\n");
    printf("add eax, %d\n", step);
  } else if (t == ternary_condition_node) {
    int snd_label = new_temp_label();
    int end_label = new_temp_label();
    generate_expr(get_child(expr, 0));
    printf("cmp eax, 0\n");
    printf("je _%d\n", snd_label);
    generate_expr(get_child(expr, 1));
    printf("jmp _%d\n", end_label);
    printf("_%d:\n", snd_label);
    generate_expr(get_child(expr, 2));
    printf("_%d:\n", end_label);
  } else if (t == sizeof_node) {
    struct Node* node = get_child(expr, 0);
    if (!is_type_node(node)) {
      node = get_expr_type_node(node);
    }
    printf("mov eax, %d\n", size_of_type(node));
  } else if (t == address_of_node) {
    check(!lvalue, "& operator does not generate left value");
    generate_expr_internal(get_child(expr, 0), 1);
  } else if (t == dereference_node) {
    generate_expr(get_child(expr, 0));
    if (!lvalue) {
      printf("mov eax, [eax]\n");
    }
  } else if (t == struct_access_node) {
    struct Node* left = get_child(expr, 0);
    struct Node* right = get_child(expr, 1);
    generate_expr_internal(left, 1);
    char* name = get_symbol(right);
    int offset = get_struct_member_offset(get_expr_type_node(left), name);
    printf("add eax, %d\n", offset);
    if (!lvalue) {
      printf("mov eax, [eax]\n");
    }
  } else if (t == struct_ptr_access_node) {
    struct Node* left = get_child(expr, 0);
    struct Node* right = get_child(expr, 1);
    generate_expr(left);
    char* name = get_symbol(right);
    struct Node* left_type = get_expr_type_node(left);
    check(left_type->type == ptr_type_node, "-> only applys to struct ptr.");
    int offset = get_struct_member_offset(get_child(left_type, 0), name);
    printf("add eax, %d\n", offset);
    if (!lvalue) {
      printf("mov eax, [eax]\n");
    }
  } else {
    check(0, "unknown expr node type");
  }
}

void generate_expr(struct Node* expr) {
  generate_expr_internal(expr, 0);
}

void generate_stmts(struct Node* stmts);

void generate_stmt(struct Node* stmt) {
  int t = stmt->type;
  if (t == if_node) {
    int else_label = new_temp_label();
    int endif_label = new_temp_label();

    generate_expr(get_child(stmt, 0));

    printf("cmp eax, 0\n");
    printf("je _%d\n", else_label);

    generate_stmts(get_child(stmt, 1));

    printf("jmp _%d\n", endif_label);
    printf("_%d:\n", else_label);

    if (child_num(stmt) == 3) {
      struct Node* else_node = get_child(stmt, 2);
      if (else_node->type == stmts_node) {
        generate_stmts(else_node);
      } else {
        // else-if
        generate_stmt(else_node);
      }
    }

    printf("_%d:\n", endif_label);
  } else if (t == while_do_node) {
    int while_label = new_temp_label();
    int endwhile_label = new_temp_label();
    int old_break_label = break_label;
    break_label = endwhile_label;
    int old_continue_label = continue_label;
    continue_label = while_label;
    printf("_%d:\n", while_label);

    generate_expr(get_child(stmt, 0));

    printf("cmp eax, 0\n");
    printf("je _%d\n", endwhile_label);

    generate_stmts(get_child(stmt, 1));

    printf("jmp _%d\n", while_label);
    printf("_%d:\n", endwhile_label);

    break_label = old_break_label;
    continue_label = old_continue_label;
  } else if (t == do_while_node) {
    int while_label = new_temp_label();
    int old_break_label = break_label;
    break_label = new_temp_label();
    int old_continue_label = continue_label;
    continue_label = new_temp_label();

    printf("_%d:\n", while_label);
    generate_stmts(get_child(stmt, 0));
    printf("_%d:\n", continue_label);
    generate_expr(get_child(stmt, 1));
    printf("cmp eax, 0\n");
    printf("jne _%d\n", while_label);
    printf("_%d:\n", break_label);

    break_label = old_break_label;
    continue_label = old_continue_label;
  } else if (t == for_node) {
    int forloop_label = new_temp_label();
    int endfor_label = new_temp_label();
    int old_continue_label = continue_label;
    continue_label = new_temp_label();
    int old_break_label = break_label;
    break_label = endfor_label;
    generate_stmt(get_child(stmt, 0));
    printf("_%d:\n", forloop_label);
    if (get_child(stmt, 1)->type != noop_node) {
      generate_expr(get_child(stmt, 1));
      printf("cmp eax, 0\n");
      printf("je _%d\n", endfor_label);
    }
    generate_stmts(get_child(stmt, 3));
    printf("_%d:\n", continue_label);
    generate_stmt(get_child(stmt, 2));
    printf("jmp _%d\n", forloop_label);
    printf("_%d:\n", endfor_label);

    break_label = old_break_label;
    continue_label = old_continue_label;
  } else if (t == return_node) {
    if (child_num(stmt) == 1) {
      generate_expr(get_child(stmt, 0));
    }
    printf("jmp _%d\n", return_label);
  } else if (t == break_node) {
    printf("jmp _%d\n", break_label);
  } else if (t == continue_node) {
    printf("jmp _%d\n", continue_label);
  } else if (t == var_init_node) {
    struct LocalVar* local = lookup_local_var(get_symbol(get_child(stmt, 1)));
    check(local, "local var not found");
    generate_expr(get_child(stmt, 2));
    printf("mov dword ptr [ebp-%d], eax\n", WORD_SIZE + local->offset);
  } else if (t == switch_node) {
    generate_expr(get_child(stmt, 0));
    printf("push eax\n");
    int old_break_label = break_label;
    break_label = new_temp_label();
    int base_label = reserve_tmp_labels(child_num(stmt) - 1);
    int default_label = new_temp_label();
    int has_default = 0;
    for (int i = 1; i < child_num(stmt); i++) {
      struct Node* branch = get_child(stmt, i);
      if (branch->type == case_node) {
        generate_expr(get_child(branch, 0));
        printf("cmp eax, [esp]\n");
        printf("je _%d\n", base_label + i - 1);
      } else {
        has_default = 1;
      }
    }
    if (has_default) {
      printf("jmp _%d\n", default_label);
    } else {
      printf("jmp _%d\n", break_label);
    }
    for (int i = 1; i < child_num(stmt); i++) {
      struct Node* branch = get_child(stmt, i);
      if (branch->type == case_node) {
        printf("_%d:\n", base_label + i - 1);
        for (int j = 1; j < child_num(branch); j++) {
          generate_stmt(get_child(branch, j));
        }
      } else if (branch->type == default_node) {
        printf("_%d:\n", default_label);
        for (int j = 0; j < child_num(branch); j++) {
          generate_stmt(get_child(branch, j));
        }
      } else {
        check(0, "only case or default node expected in switch node");
      }
    }
    printf("_%d:\n", break_label);
    printf("add esp, %d\n", WORD_SIZE);
    break_label = old_break_label;
  } else if (t == noop_node || t == var_decl_node) {
    // do nothing
  } else {
    generate_expr(stmt);
  }
}

void generate_stmts(struct Node* stmts) {
  check(stmts->type == stmts_node, "generate_stmts");
  for (int i = 0; i < child_num(stmts); i++) {
    generate_stmt(get_child(stmts, i));
  }
}

void generate_code(struct Node* root) {
  check(root->type == prog_node, "prog_node expected");
  printf(".intel_syntax noprefix\n");
  printf(".section .data\n");
  // declare all global variables.
  for (int i = 0; i < child_num(root); i++) {
    struct Node* cur = get_child(root, i);
    if (cur->type == function_impl_node
        || cur->type == function_decl_node
        || cur->type == var_init_node
        || cur->type == var_decl_node
        || cur->type == extern_var_decl_node) {
      // index 0 is type
      char* name = get_symbol(get_child(cur, 1));
      // expose function names and global var names
      printf(".globl %s\n", name);
      enum NodeType t = cur->type;
      if (t == var_init_node) {
        // declare initialized global var
        check(child_num(cur) == 3 && get_child(cur, 2)->type == int_node,
            "only integer variable initialization is allowed\n");
        int value = get_int(get_child(cur, 2));
        printf("%s: .long %d\n", name, value);
        register_global_var(cur);
      } else if (t == var_decl_node) {
        // declare uninitialized global var
        printf("%s: .long 0\n", name);
        register_global_var(cur);
      } else if (t == extern_var_decl_node) {
        register_global_var(cur);
      } else if (t == function_impl_node || t == function_decl_node) {
        register_function(cur);
      }
    }
  }
  printf(".section .text\n");
  // generate code for functions
  for (int i = 0; i < child_num(root); i++) {
    struct Node* cur = get_child(root, i);
    if (cur->type == function_impl_node) {
      char* name = get_symbol(get_child(cur, 1));
      struct Node* stmts = get_child(cur, 3);
      register_function_params(cur);
      return_label = new_temp_label();

      string_map_clear(local_vars);
      // register local vars for later look up 
      register_local_vars(stmts);

      in_function = 1;

      // function entry point
      printf("%s:\n", name);
      printf("push ebp\n");
      printf("mov ebp, esp\n");
      printf("sub esp, %d\n", string_map_size(local_vars) * WORD_SIZE);

      generate_stmts(stmts);
      // function epilog
      printf("_%d:\n", return_label);
      printf("mov esp, ebp\n");
      printf("pop ebp\n");
      printf("ret\n");

      in_function = 0;
      string_map_clear(function_params);
    } else if (cur->type == enum_node) {
      int value = 0;
      // skip enum name
      for (int j = 1; j < child_num(cur); j++) {
        struct Node* val_node = get_child(cur, j);
        char* key = get_symbol(get_child(val_node, 0));
        if (child_num(val_node) == 2) {
          value = get_int(get_child(val_node, 1));
        }
        register_enum(key, value++);
      }
    } else if (cur->type == struct_node) {
      register_struct_def(cur);
    }
  }
}

void init_codegen() {
  functions = new_string_map();
  enums = new_string_map();
  local_vars = new_string_map();
  global_var_defs = new_string_map();
  struct_defs = new_string_map();
  function_params = new_string_map();
}
