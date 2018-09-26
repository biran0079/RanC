#include "codegen_x86.h"

int* global_var_node;
int global_var_num;

int* local_var_node;
int local_var_num;

char** enum_key;
int* enum_value;
int enum_num;

char** function_name;
int* function_node;
int function_num;

int in_function;

// Params node for current function.
// Used for param look up when generating code for function body.
int function_params; 
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

char* get_symbol(int cur) {
  check(node_type[cur] == symbol_node, "get_symbol");
  return node_payload[cur];
}

int get_int(int cur) {
  check(node_type[cur] == int_node, "get_int");
  return atoi(node_payload[cur]);
}

char* get_string(int cur) {
  check(node_type[cur] == string_node, "get_string");
  return node_payload[cur];
}

void register_global_var(int node) {
  global_var_node[global_var_num++] = node;
  check(global_var_num <= MAX_GLOBAL_VARS, "too many global vars");
}

int lookup_global_var(char* s) {
  for (int i = 0; i < global_var_num; i++) {
    int node = global_var_node[i];
    if (!strcmp(s, get_symbol(node_child[node][1]))) {
      return i;
    }
  }
  return -1;
}

int lookup_function_idx(char* s) {
  for (int i = 0; i < function_num; i++) {
    int fun_node = function_node[i];
    char* name = get_symbol(node_child[fun_node][1]);
    if (!strcmp(s, name)) {
      return i;
    }
  }
  return -1;
}

void register_function(int node) {
  function_node[function_num++] = node;
  check(function_num <= MAX_FUNCTION_NUM, "too many functions");
}

void register_enum(char* s, int n) {
  int i = enum_num++;
  enum_key[i] = s;
  enum_value[i] = n;
}

int lookup_enum_idx(char* s) {
  for (int i = 0; i < enum_num; i++) {
    if (!strcmp(s, enum_key[i])) {
      return i;
    }
  }
  return -1;
}

void register_local_var(int node) {
  local_var_node[local_var_num] = node;
  local_var_num++;
  check(local_var_num <= MAX_LOCAL_VARS, "too many local vars");
}

int lookup_local_var(char* s) {
  if (!in_function) {
    return -1;
  }
  for (int i = 0; i < local_var_num; i++) {
    int node = local_var_node[i];
    char* name = get_symbol(node_child[node][1]);
    if (!strcmp(s, name)) {
      return i;
    }
  }
  return -1;
}

int lookup_param(char* s) {
  if (!in_function) {
    return -1;
  }
  for (int i = 0; i < node_child_num[function_params]; i++) {
    int param_node = node_child[function_params][i];
    if (!strcmp(s, get_symbol(node_child[param_node][1]))) {
      return i;
    }
  }
  return -1;
}

// search for all var_decl_node and var_init_node in subtree
void register_local_vars(int root) {
  for (int i = 0; i < node_child_num[root]; i++) {
    int cur = node_child[root][i];
    int t = node_type[cur];
    if (t == var_decl_node || t == var_init_node) {
      register_local_var(cur);
    } else {
      register_local_vars(cur);
    }
  }
}

char* get_set_cmp_inst(int type) {
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

void generate_expr(int expr);

int size_of_type(int type_node) {
  int t = node_type[type_node];
  if (t == int_type_node || t == function_type_node || t == ptr_type_node || t == enum_type_node) {
    return WORD_SIZE;
  } else if (t == char_type_node || t == void_type_node) {
    return 1;
  } 
  check(0, "unknown type node");
}

int get_symbol_type_node(char* s) {
  int idx = lookup_local_var(s);
  if (idx >= 0) {
    return node_child[local_var_node[idx]][0];
  }
  idx = lookup_param(s);
  if (idx >= 0) {
    return node_child[node_child[function_params][idx]][0];
  }
  idx = lookup_function_idx(s);
  if (idx >= 0) {
    int res = new_node(function_type_node);
    append_child(res, node_child[function_node[idx]][0]);
    append_child(res, node_child[function_node[idx]][2]);
    return res;
  }
  idx = lookup_enum_idx(s);
  if (idx >= 0) {
    return new_node(enum_type_node);
  }
  idx = lookup_global_var(s);
  if (idx >= 0) {
    return node_child[global_var_node[idx]][0];
  }
  return -1;
}

int get_expr_type_node(int expr) {
  int t = node_type[expr];
  if (t == assignment_node || t == add_eq_node || t == sub_eq_node || t == mul_eq_node || t == div_eq_node
      || t == or_node || t == and_node || t == not_node || get_set_cmp_inst(t)
      || t == add_node || t == sub_node || t == mul_node || t == div_node || t == mod_node
      || t == negative_node || t == inc_prefix_node || t == inc_suffix_node || t == dec_prefix_node || t == dec_suffix_node) {
    return get_expr_type_node(node_child[expr][0]);
  } else if (t == int_node) {
    return new_node(int_type_node);
  } else if (t == string_node) {
    int res = new_node(ptr_type_node);
    append_child(res, new_node(char_type_node));
    return res;
  } else if (t == char_node) {
    return new_node(char_type_node);
  } else if(t == symbol_node) {
    int type_node = get_symbol_type_node(get_symbol(expr));
    check(type_node >= 0, "unknown symbol");
    return type_node;
  } else if(t == access_node) {
    int base = get_expr_type_node(node_child[expr][0]);
    check(node_type[base] == ptr_type_node, "illegal array accessing");
    return node_child[base][0];
  } else if (t == call_node) {
    int fun = get_expr_type_node(node_child[expr][0]);
    check(node_type[fun] == function_type_node, "illegal function call");
    return node_child[fun][0];
  } else if (t == ternary_condition_node) {
    return get_expr_type_node(node_child[expr][1]);
  } else {
    check(0, "unknown expr node type");
  }
}

int get_expr_type_size(int expr) {
  return size_of_type(get_expr_type_node(expr));
}

void generate_expr_internal(int expr, int lvalue) {
  int t = node_type[expr];
  int end_label;
  if (t == assignment_node) {
    generate_expr_internal(node_child[expr][0], 1);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("pop ebx\n");
    printf("mov dword ptr [ebx], eax\n");
  } else if (t == add_eq_node) {
    generate_expr_internal(node_child[expr][0], 1);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("pop ebx\n");
    printf("add [ebx], eax\n");
    printf("mov eax, [ebx]\n");
  } else if (t == sub_eq_node) {
    generate_expr_internal(node_child[expr][0], 1);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("pop ebx\n");
    printf("sub [ebx], eax\n");
    printf("mov eax, [ebx]\n");
  } else if (t == mul_eq_node) {
    generate_expr_internal(node_child[expr][0], 1);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("pop ebx\n");
    printf("imul eax, dword ptr [ebx]\n");
    printf("mov [ebx], eax\n");
  } else if (t == div_eq_node) {
    generate_expr_internal(node_child[expr][0], 1);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("mov ebx, eax\n");
    printf("mov eax, dword ptr [esp]\n");
    printf("mov eax, [eax]\n");
    printf("cdq\n");
    printf("idiv ebx\n");
    printf("pop ebx\n");
    printf("mov [ebx], eax\n");
  } else if (t == or_node) {
    end_label = new_temp_label();
    for (int i = 0; i < node_child_num[expr]; i++) {
      generate_expr(node_child[expr][i]);
      printf("cmp eax, 0\n");
      printf("jnz _%d\n", end_label);
    }
    printf("_%d:\n", end_label);
  } else if (t == and_node) {
    end_label = new_temp_label();
    for (int i = 0; i < node_child_num[expr]; i++) {
      generate_expr(node_child[expr][i]);
      printf("cmp eax, 0\n");
      printf("jz _%d\n", end_label);
    }
    printf("_%d:\n", end_label);
  } else if (t == not_node) {
    generate_expr(node_child[expr][0]);
    printf("cmp eax, 0\n");
    printf("mov eax, 0\n");
    printf("sete al\n");
  } else if (get_set_cmp_inst(t)) {
    generate_expr(node_child[expr][0]);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("pop ebx\n");
    printf("cmp ebx, eax\n");
    printf("mov eax, 0\n");
    printf("%s al\n", get_set_cmp_inst(t));
  } else if (t == add_node) {
    generate_expr(node_child[expr][0]);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("pop ebx\n");
    printf("add eax, ebx\n");
  } else if (t == sub_node) {
    generate_expr(node_child[expr][0]);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("pop ebx\n");
    printf("sub ebx, eax\n");
    printf("mov eax, ebx\n");
  } else if (t == mul_node) {
    generate_expr(node_child[expr][0]);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("pop ebx\n");
    printf("imul eax, ebx\n");
  } else if (t == div_node || t == mod_node) {
    generate_expr(node_child[expr][0]);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
    printf("mov ebx, eax\n");
    printf("pop eax\n");
    printf("cdq\n");
    printf("idiv ebx\n");
    if (t == mod_node) {
      printf("mov eax, edx\n");
    }
  } else if (t == int_node || t == char_node) {
    printf("mov eax, %s\n", node_payload[expr]);
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
    int local_index = lookup_local_var(name);
    int param_index = lookup_param(name);
    int enum_index = lookup_enum_idx(name);
    char* op;
    if (lvalue) {
      op = "lea";
    } else {
      op = "mov";
    }
    if (local_index >= 0) {
      printf("%s eax, [ebp-%d]\n", op, (1 + local_index) * WORD_SIZE);
    } else if (param_index >= 0) {
      printf("%s eax, [ebp+%d]\n", op, (2 + param_index) * WORD_SIZE);
    } else if (enum_index >= 0) {
      check(!lvalue, "enum cannot be lvalue\n");
      printf("%s eax, %d\n", op, enum_value[enum_index]);
    } else {
      printf("%s eax, [%s]\n", op, name);
    }
  } else if(t == access_node) {
    int type_size = get_expr_type_size(expr);
    generate_expr(node_child[expr][0]);
    printf("push eax\n");
    generate_expr(node_child[expr][1]);
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
    int fun = node_child[expr][0];
    check(node_type[fun] == symbol_node, "function has to be a symbol");
    char* fname = get_symbol(fun);
    int args = node_child[expr][1];
    for (int i = node_child_num[args] - 1; i >= 0; i--) {
      generate_expr(node_child[args][i]);
      printf("push eax\n");
    }
    printf("call %s\n", fname);
    printf("add esp, %d\n", node_child_num[args] * WORD_SIZE);
  } else if (t == negative_node) {
    generate_expr(node_child[expr][0]);
    printf("not eax\n");
    printf("add eax, 1\n");
  } else if (t == inc_prefix_node) {
    generate_expr_internal(node_child[expr][0], 1);
    printf("add dword ptr [eax], 1\n");
    printf("mov eax, dword ptr [eax]\n");
  } else if (t == dec_prefix_node) {
    generate_expr_internal(node_child[expr][0], 1);
    printf("sub dword ptr [eax], 1\n");
    printf("mov eax, dword ptr [eax]\n");
  } else if (t == inc_suffix_node) {
    generate_expr_internal(node_child[expr][0], 1);
    printf("add dword ptr [eax], 1\n");
    printf("mov eax, dword ptr [eax]\n");
    printf("sub eax, 1\n");
  } else if (t == dec_suffix_node) {
    generate_expr_internal(node_child[expr][0], 1);
    printf("sub dword ptr [eax], 1\n");
    printf("mov eax, dword ptr [eax]\n");
    printf("add eax, 1\n");
  } else if (t == ternary_condition_node) {
    int snd_label = new_temp_label();
    int end_label = new_temp_label();
    generate_expr(node_child[expr][0]);
    printf("cmp eax, 0\n");
    printf("je _%d\n", snd_label);
    generate_expr(node_child[expr][1]);
    printf("jmp _%d\n", end_label);
    printf("_%d:\n", snd_label);
    generate_expr(node_child[expr][2]);
    printf("_%d:\n", end_label);
  } else if (t == sizeof_node) {
    printf("mov eax, %d\n", size_of_type(get_expr_type_node(node_child[expr][0])));
  } else {
    check(0, "unknown expr node type");
  }
}

void generate_expr(int expr) {
  generate_expr_internal(expr, 0);
}

void generate_stmts(int stmts);

void generate_stmt(int stmt) {
  int t = node_type[stmt];
  if (t == if_node) {
    int else_label = new_temp_label();
    int endif_label = new_temp_label();

    generate_expr(node_child[stmt][0]);

    printf("cmp eax, 0\n");
    printf("je _%d\n", else_label);

    generate_stmts(node_child[stmt][1]);

    printf("jmp _%d\n", endif_label);
    printf("_%d:\n", else_label);

    if (node_child_num[stmt] == 3) {
      int else_node = node_child[stmt][2];
      if (node_type[else_node] == stmts_node) {
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

    generate_expr(node_child[stmt][0]);

    printf("cmp eax, 0\n");
    printf("je _%d\n", endwhile_label);

    generate_stmts(node_child[stmt][1]);

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
    generate_stmts(node_child[stmt][0]);
    printf("_%d:\n", continue_label);
    generate_expr(node_child[stmt][1]);
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
    generate_stmt(node_child[stmt][0]);
    printf("_%d:\n", forloop_label);
    if (node_type[node_child[stmt][1]] != noop_node) {
      generate_expr(node_child[stmt][1]);
      printf("cmp eax, 0\n");
      printf("je _%d\n", endfor_label);
    }
    generate_stmts(node_child[stmt][3]);
    printf("_%d:\n", continue_label);
    generate_stmt(node_child[stmt][2]);
    printf("jmp _%d\n", forloop_label);
    printf("_%d:\n", endfor_label);

    break_label = old_break_label;
    continue_label = old_continue_label;
  } else if (t == return_node) {
    if (node_child_num[stmt] == 1) {
      generate_expr(node_child[stmt][0]);
    }
    printf("jmp _%d\n", return_label);
  } else if (t == break_node) {
    printf("jmp _%d\n", break_label);
  } else if (t == continue_node) {
    printf("jmp _%d\n", continue_label);
  } else if (t == var_init_node) {
    int index = lookup_local_var(get_symbol(node_child[stmt][1]));
    check(index >= 0, "local var not found");
    generate_expr(node_child[stmt][2]);
    printf("mov dword ptr [ebp-%d], eax\n", (1 + index) * WORD_SIZE);
  } else if (t == noop_node || t == var_decl_node) {
    // do nothing
  } else {
    generate_expr(stmt);
  }
}

void generate_stmts(int stmts) {
  check(node_type[stmts] == stmts_node, "generate_stmts");
  for (int i = 0; i < node_child_num[stmts]; i++) {
    generate_stmt(node_child[stmts][i]);
  }
}

void generate_code(int root) {
  check(node_type[root] == prog_node, "prog_node expected");
  printf(".intel_syntax noprefix\n");
  printf(".section .data\n");
  // declare all global variables.
  for (int i = 0; i < node_child_num[root]; i++) {
    int cur = node_child[root][i];
    if (node_type[cur] == function_impl_node
        || node_type[cur] == function_decl_node
        || node_type[cur] == var_init_node
        || node_type[cur] == var_decl_node
        || node_type[cur] == extern_var_decl_node) {
      // index 0 is type
      char* name = get_symbol(node_child[cur][1]);
      // expose function names and global var names
      printf(".globl %s\n", name);
      int t = node_type[cur];
      if (t == var_init_node) {
        // declare initialized global var
        check(node_child_num[cur] == 3 && node_type[node_child[cur][2]] == int_node,
            "only integer variable initialization is allowed\n");
        int value = get_int(node_child[cur][2]);
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
  for (int i = 0; i < node_child_num[root]; i++) {
    int cur = node_child[root][i];
    if (node_type[cur] == function_impl_node) {
      char* name = get_symbol(node_child[cur][1]);
      int stmts = node_child[cur][3];
      function_params = node_child[cur][2];
      return_label = new_temp_label();

      local_var_num = 0;
      // register local vars for later look up 
      register_local_vars(stmts);

      in_function = 1;

      // function entry point
      printf("%s:\n", name);
      printf("push ebp\n");
      printf("mov ebp, esp\n");
      printf("sub esp, %d\n", local_var_num * WORD_SIZE);

      generate_stmts(stmts);
      // function epilog
      printf("_%d:\n", return_label);
      printf("mov esp, ebp\n");
      printf("pop ebp\n");
      printf("ret\n");

      in_function = 0;
    } else if (node_type[cur] == enum_node) {
      int value = 0;
      // skip enum name
      for (int j = 1; j < node_child_num[cur]; j++) {
        int val_node = node_child[cur][j];
        char* key = get_symbol(node_child[val_node][0]);
        if (node_child_num[val_node] == 2) {
          value = get_int(node_child[val_node][1]);
        }
        register_enum(key, value++);
      }
    }
  }
}

void init_codegen() {
  local_var_node = malloc(MAX_LOCAL_VARS * WORD_SIZE);
  local_var_num = 0;
  enum_key = malloc(MAX_ENUM_VALUES * WORD_SIZE);
  enum_value = malloc(MAX_ENUM_VALUES * WORD_SIZE);
  enum_num = 0;
  function_node = malloc(MAX_FUNCTION_NUM * WORD_SIZE);
  function_num = 0;
  global_var_node = malloc(MAX_GLOBAL_VARS * WORD_SIZE);
  global_var_num = 0;
}
