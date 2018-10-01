#include "codegen_x86.h"
#include "ast_processor.h"
#include "string_map.h"

struct CodegenContext {
  // current LST when generating code for function body
  struct LocalSymbolTable* lst;
  struct GlobalSymbolTable* gst;

  // Label for function epilog. Needed for return statement.
  int return_label;

  // Label for breaking current loop.
  int break_label;
  // Label for continuing current loop.
  int continue_label;

  int tmp_label_count;
};

struct CodegenContext* new_codegen_context(struct ProcessedAst* processed_ast) {
  struct CodegenContext* res = malloc(sizeof(struct CodegenContext));  
  res->gst = processed_ast->gst;
  res->lst = 0;
  res->return_label = 0;
  res->break_label = 0;
  res->continue_label = 0;
  res->tmp_label_count = 0;
  return res;
}

// reserve n consecutive tmp labels, return the first one
int reserve_tmp_labels(struct CodegenContext* ctx, int n) {
  int res = ctx->tmp_label_count;
  ctx->tmp_label_count += n;
  return res;
}

int new_temp_label(struct CodegenContext* ctx) {
  return reserve_tmp_labels(ctx, 1);
}

char* get_set_cmp_inst(enum NodeType type) {
  switch (type) {
    case eq_node: return "sete";
    case ne_node: return "setne";
    case lt_node: return "setl";
    case gt_node: return "setg";
    case le_node: return "setle";
    case ge_node: return "setge";
    default: return 0;
  }
  return 0;
}

int get_ptr_step_size(struct CType* type) {
  if (type->type == PTR_TYPE) {
    return type->ptr_data->size;
  }
  return 1;
}

char* get_register_name(int size, char* base) {
  char* res = malloc(4);
  res[0] = 0;
  if (size == 1) {
    strcat(res, base);
    strcat(res, "l");
  } else if (size == WORD_SIZE) {
    strcat(res, "e");
    strcat(res, base);
    strcat(res, "x");
  } else {
    check(0, "bad size");
  }
  return res;
}

char* get_size_directive(int size) {
  if (size == 1) {
    return "byte";
  } else if (size == WORD_SIZE) {
    return "dword";
  } 
  check(0, "bad size");
}

void generate_expr(struct CodegenContext* ctx, struct Node* expr);

void generate_expr_internal(struct CodegenContext* ctx, struct Node* expr, int lvalue) {
  int end_label;
  struct CType* ctype= get_expr_ctype(ctx->gst, ctx->lst, expr);
  int size = ctype->size;
  switch (expr->type) {
    case assignment_node: {
      struct Node* left = get_child(expr, 0);
      generate_expr_internal(ctx, left, 1);
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      printf("pop ebx\n");
      printf("mov %s ptr [ebx], %s\n", get_size_directive(size), get_register_name(size, "a"));
      break;
    } 
    case add_eq_node: {
      generate_expr_internal(ctx, get_child(expr, 0), 1);
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      int step = get_ptr_step_size(ctype);
      if (step > 1) {
        printf("imul eax, %d\n", step);
      }
      printf("pop ebx\n");
      printf("add [ebx], eax\n");
      printf("mov eax, [ebx]\n");
      break;
    } 
    case sub_eq_node: {
      generate_expr_internal(ctx, get_child(expr, 0), 1);
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      int step = get_ptr_step_size(ctype);
      if (step > 1) {
        printf("imul eax, %d\n", step);
      }
      printf("pop ebx\n");
      printf("sub [ebx], eax\n");
      printf("mov eax, [ebx]\n");
      break;
    } 
    case mul_eq_node: {
      generate_expr_internal(ctx, get_child(expr, 0), 1);
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      printf("pop ebx\n");
      printf("imul eax, dword ptr [ebx]\n");
      printf("mov [ebx], eax\n");
      break;
    }
    case div_eq_node: {
      generate_expr_internal(ctx, get_child(expr, 0), 1);
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      printf("mov ebx, eax\n");
      printf("mov eax, dword ptr [esp]\n");
      printf("mov eax, [eax]\n");
      printf("cdq\n");
      printf("idiv ebx\n");
      printf("pop ebx\n");
      printf("mov [ebx], eax\n");
      break;
    }
    case or_node: {
      end_label = new_temp_label(ctx);
      for (int i = 0; i < child_num(expr); i++) {
        generate_expr(ctx, get_child(expr, i));
        printf("cmp eax, 0\n");
        printf("jnz _%d\n", end_label);
      }
      printf("_%d:\n", end_label);
      break;
    }
    case and_node: {
      end_label = new_temp_label(ctx);
      for (int i = 0; i < child_num(expr); i++) {
        generate_expr(ctx, get_child(expr, i));
        printf("cmp eax, 0\n");
        printf("jz _%d\n", end_label);
      }
      printf("_%d:\n", end_label);
      break;
    }
    case not_node: {
      generate_expr(ctx, get_child(expr, 0));
      printf("cmp eax, 0\n");
      printf("mov eax, 0\n");
      printf("sete al\n");
      break;
    }
    case eq_node: 
    case ne_node: 
    case lt_node: 
    case gt_node: 
    case le_node: 
    case ge_node: {
      generate_expr(ctx, get_child(expr, 0));
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      printf("pop ebx\n");
      printf("cmp ebx, eax\n");
      printf("mov eax, 0\n");
      printf("%s al\n", get_set_cmp_inst(expr->type));
      break;
    } 
    case add_node: {
      generate_expr(ctx, get_child(expr, 0));
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      int step = get_ptr_step_size(ctype);
      if (step > 1) {
        printf("imul eax, %d\n", step);
      }
      printf("pop ebx\n");
      printf("add eax, ebx\n");
      break;
    } 
    case sub_node: {
      generate_expr(ctx, get_child(expr, 0));
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      int step = get_ptr_step_size(ctype);
      if (step > 1) {
        printf("imul eax, %d\n", step);
      }
      printf("pop ebx\n");
      printf("sub ebx, eax\n");
      printf("mov eax, ebx\n");
      break;
    } 
    case mul_node: {
      generate_expr(ctx, get_child(expr, 0));
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      printf("pop ebx\n");
      printf("imul eax, ebx\n");
      break;
    } 
    case div_node: 
    case mod_node: {
      generate_expr(ctx, get_child(expr, 0));
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      printf("mov ebx, eax\n");
      printf("pop eax\n");
      printf("cdq\n");
      printf("idiv ebx\n");
      if (expr->type == mod_node) {
        printf("mov eax, edx\n");
      }
      break;
    } 
    case int_node:
    case char_node: {
      printf("mov eax, %s\n", expr->payload);
      break;
    } 
    case string_node: {
      int string_label = new_temp_label(ctx);
      printf(".section .rodata\n");
      printf("_%d:\n", string_label);
      printf(".ascii %s\n", get_string(expr));
      printf(".byte 0\n");
      printf(".section .text\n");
      printf("mov eax, offset _%d\n", string_label);
      break;
    } 
    case symbol_node: {
      char* name = get_symbol(expr);
      struct LocalVar* local = lookup_local_var(ctx->lst, name);
      struct FunctionParam* param = lookup_function_param(ctx->lst, name);
      struct Enum* enum_entry = lookup_enum(ctx->gst, name);
      char* op;
      if (lvalue) {
        op = "lea";
      } else if (size < WORD_SIZE){
        op = "movzx";
      } else {
        op = "mov";
      }
      if (local) {
        printf("%s eax, %s ptr [ebp - %d]\n", op, get_size_directive(size), WORD_SIZE + local->offset);
      } else if (param) {
        printf("%s eax, %s ptr [ebp + %d]\n", op, get_size_directive(size), 2 * WORD_SIZE + param->offset);
      } else if (enum_entry) {
        check(!lvalue, "enum cannot be lvalue\n");
        printf("%s eax, %d\n", op, enum_entry->value);
      } else {
        printf("%s eax, %s ptr [%s]\n", op, get_size_directive(size),  name);
      }
      break;
    } 
    case access_node: {
      generate_expr(ctx, get_child(expr, 0));
      printf("push eax\n");
      generate_expr(ctx, get_child(expr, 1));
      printf("mov ebx, eax\n");
      printf("pop ebx\n");
      char* op;
      if (lvalue) {
        op = "lea";
      } else if (size < WORD_SIZE) {
        op = "movzx"; // zero extend
      } else {
        op = "mov";
      }
      printf("%s eax, %s ptr [eax * %d + ebx]\n", op, get_size_directive(size), size);
      break;
    } 
    case call_node: {
      struct Node* fun = get_child(expr, 0);
      check(fun->type == symbol_node, "function has to be a symbol");
      char* fname = get_symbol(fun);
      struct Node* args = get_child(expr, 1);
      for (int i = child_num(args) - 1; i >= 0; i--) {
        generate_expr(ctx, get_child(args, i));
        printf("push eax\n");
      }
      printf("call %s\n", fname);
      printf("add esp, %d\n", child_num(args) * WORD_SIZE);
      break;
    } 
    case negative_node: {
      generate_expr(ctx, get_child(expr, 0));
      printf("not eax\n");
      printf("add eax, 1\n");
      break;
    } 
    case inc_prefix_node: {
      generate_expr_internal(ctx, get_child(expr, 0), 1);
      printf("add %s ptr [eax], %d\n", get_size_directive(size), get_ptr_step_size(ctype));
      printf("%s eax, %s ptr [eax]\n", size < WORD_SIZE ? "movzx" : "mov", get_size_directive(size));
      break;
    } 
    case dec_prefix_node: {
      generate_expr_internal(ctx, get_child(expr, 0), 1);
      printf("sub %s ptr [eax], %d\n", get_size_directive(size), get_ptr_step_size(ctype));
      printf("%s eax, %s ptr [eax]\n", size < WORD_SIZE ? "movzx" : "mov", get_size_directive(size));
      break;
    } 
    case inc_suffix_node: {
      int step = get_ptr_step_size(ctype);
      generate_expr_internal(ctx, get_child(expr, 0), 1);
      printf("add %s ptr [eax], %d\n", get_size_directive(size), step);
      printf("%s eax, %s ptr [eax]\n", size < WORD_SIZE ? "movzx" : "mov", get_size_directive(size));
      printf("sub eax, %d\n", step);
      break;
    } 
    case dec_suffix_node: {
      int step = get_ptr_step_size(ctype);
      generate_expr_internal(ctx, get_child(expr, 0), 1);
      printf("sub %s ptr [eax], %d\n", get_size_directive(size), step);
      printf("%s eax, %s ptr [eax]\n", size < WORD_SIZE ? "movzx" : "mov", get_size_directive(size));
      printf("add eax, %d\n", step);
      break;
    } 
    case ternary_condition_node: {
      int snd_label = new_temp_label(ctx);
      int end_label = new_temp_label(ctx);
      generate_expr(ctx, get_child(expr, 0));
      printf("cmp eax, 0\n");
      printf("je _%d\n", snd_label);
      generate_expr(ctx, get_child(expr, 1));
      printf("jmp _%d\n", end_label);
      printf("_%d:\n", snd_label);
      generate_expr(ctx, get_child(expr, 2));
      printf("_%d:\n", end_label);
      break;
    } 
    case sizeof_node: {
      struct Node* node = get_child(expr, 0);
      struct CType* type = is_type_node(node) ? new_ctype_from_type_node(ctx->gst, node) : get_expr_ctype(ctx->gst, ctx->lst, node);
      printf("mov eax, %d\n", type->size);
      break;
    } 
    case address_of_node: {
      check(!lvalue, "& operator does not generate left value");
      generate_expr_internal(ctx, get_child(expr, 0), 1);
      break;
    } 
    case dereference_node: {
      generate_expr(ctx, get_child(expr, 0));
      char* op = ctype->size < WORD_SIZE ? "movzx" : "mov";
      if (!lvalue) {
        printf("%s eax, %s ptr [eax]\n", op, get_size_directive(ctype->size));
      }
      break;
    } 
    case struct_access_node: {
      struct Node* left = get_child(expr, 0);
      struct Node* right = get_child(expr, 1);
      generate_expr_internal(ctx, left, 1);
      char* name = get_symbol(right);
      int offset = get_struct_member_offset(ctx->gst, get_expr_ctype(ctx->gst, ctx->lst, left), name);
      printf("add eax, %d\n", offset);
      if (!lvalue) {
        char* op = size < WORD_SIZE ? "movzx" : "mov";
        printf("%s eax, %s ptr [eax]\n", op, get_size_directive(size));
      }
      break;
    } 
    case struct_ptr_access_node: {
      struct Node* left = get_child(expr, 0);
      struct Node* right = get_child(expr, 1);
      generate_expr(ctx, left);
      char* name = get_symbol(right);
      struct CType* left_type = get_expr_ctype(ctx->gst, ctx->lst, left);
      check(left_type->type == PTR_TYPE && left_type->ptr_data->type == STRUCT_TYPE, "-> only applys to struct ptr.");
      int offset = get_struct_member_offset(ctx->gst, left_type->ptr_data, name);
      printf("add eax, %d\n", offset);
      if (!lvalue) {
        char* op = size < WORD_SIZE ? "movzx" : "mov";
        printf("%s eax, %s ptr [eax]\n", op, get_size_directive(size));
      }
      break;
    } 
    default:
      check(0, "unknown expr node type");
  }
}

void generate_expr(struct CodegenContext* ctx, struct Node* expr) {
  generate_expr_internal(ctx, expr, 0);
}

void generate_stmts(struct CodegenContext* ctx, struct Node* stmts);

void generate_stmt(struct CodegenContext* ctx, struct Node* stmt) {
  switch (stmt->type) {
    case if_node: {
      int else_label = new_temp_label(ctx);
      int endif_label = new_temp_label(ctx);

      generate_expr(ctx, get_child(stmt, 0));

      printf("cmp eax, 0\n");
      printf("je _%d\n", else_label);

      generate_stmts(ctx, get_child(stmt, 1));

      printf("jmp _%d\n", endif_label);
      printf("_%d:\n", else_label);

      if (child_num(stmt) == 3) {
        struct Node* else_node = get_child(stmt, 2);
        if (else_node->type == stmts_node) {
          generate_stmts(ctx, else_node);
        } else {
          // else-if
          generate_stmt(ctx, else_node);
        }
      }

      printf("_%d:\n", endif_label);
      break;
    } 
    case while_do_node: {
      int while_label = new_temp_label(ctx);
      int endwhile_label = new_temp_label(ctx);
      int old_break_label = ctx->break_label;
      ctx->break_label = endwhile_label;
      int old_continue_label = ctx->continue_label;
      ctx->continue_label = while_label;
      printf("_%d:\n", while_label);

      generate_expr(ctx, get_child(stmt, 0));

      printf("cmp eax, 0\n");
      printf("je _%d\n", endwhile_label);

      generate_stmts(ctx, get_child(stmt, 1));

      printf("jmp _%d\n", while_label);
      printf("_%d:\n", endwhile_label);

      ctx->break_label = old_break_label;
      ctx->continue_label = old_continue_label;
      break;
    } 
    case do_while_node: {
      int while_label = new_temp_label(ctx);
      int old_break_label = ctx->break_label;
      ctx->break_label = new_temp_label(ctx);
      int old_continue_label = ctx->continue_label;
      ctx->continue_label = new_temp_label(ctx);

      printf("_%d:\n", while_label);
      generate_stmts(ctx, get_child(stmt, 0));
      printf("_%d:\n", ctx->continue_label);
      generate_expr(ctx, get_child(stmt, 1));
      printf("cmp eax, 0\n");
      printf("jne _%d\n", while_label);
      printf("_%d:\n", ctx->break_label);

      ctx->break_label = old_break_label;
      ctx->continue_label = old_continue_label;
      break;
    } 
    case for_node: {
      int forloop_label = new_temp_label(ctx);
      int endfor_label = new_temp_label(ctx);
      int old_continue_label = ctx->continue_label;
      ctx->continue_label = new_temp_label(ctx);
      int old_break_label = ctx->break_label;
      ctx->break_label = endfor_label;
      generate_stmt(ctx, get_child(stmt, 0));
      printf("_%d:\n", forloop_label);
      if (get_child(stmt, 1)->type != noop_node) {
        generate_expr(ctx, get_child(stmt, 1));
        printf("cmp eax, 0\n");
        printf("je _%d\n", endfor_label);
      }
      generate_stmts(ctx, get_child(stmt, 3));
      printf("_%d:\n", ctx->continue_label);
      generate_stmt(ctx, get_child(stmt, 2));
      printf("jmp _%d\n", forloop_label);
      printf("_%d:\n", endfor_label);

      ctx->break_label = old_break_label;
      ctx->continue_label = old_continue_label;
      break;
    } 
    case return_node: {
      if (child_num(stmt) == 1) {
        generate_expr(ctx, get_child(stmt, 0));
      }
      printf("jmp _%d\n", ctx->return_label);
      break;
    } 
    case break_node: {
      printf("jmp _%d\n", ctx->break_label);
      break;
    } 
    case continue_node: {
      printf("jmp _%d\n", ctx->continue_label);
      break;
    } 
    case var_init_node: {
      struct LocalVar* local = lookup_local_var(ctx->lst, get_symbol(get_child(stmt, 1)));
      check(local, "local var not found");
      generate_expr(ctx, get_child(stmt, 2));
      printf("mov dword ptr [ebp-%d], eax\n", WORD_SIZE + local->offset);
      break;
    } 
    case switch_node: {
      generate_expr(ctx, get_child(stmt, 0));
      printf("push eax\n");
      int old_break_label = ctx->break_label;
      ctx->break_label = new_temp_label(ctx);
      int base_label = reserve_tmp_labels(ctx, child_num(stmt) - 1);
      int default_label = new_temp_label(ctx);
      int has_default = 0;
      for (int i = 1; i < child_num(stmt); i++) {
        struct Node* branch = get_child(stmt, i);
        if (branch->type == case_node) {
          generate_expr(ctx, get_child(branch, 0));
          printf("cmp eax, [esp]\n");
          printf("je _%d\n", base_label + i - 1);
        } else {
          has_default = 1;
        }
      }
      if (has_default) {
        printf("jmp _%d\n", default_label);
      } else {
        printf("jmp _%d\n", ctx->break_label);
      }
      for (int i = 1; i < child_num(stmt); i++) {
        struct Node* branch = get_child(stmt, i);
        if (branch->type == case_node) {
          printf("_%d:\n", base_label + i - 1);
          for (int j = 1; j < child_num(branch); j++) {
            generate_stmt(ctx, get_child(branch, j));
          }
        } else if (branch->type == default_node) {
          printf("_%d:\n", default_label);
          for (int j = 0; j < child_num(branch); j++) {
            generate_stmt(ctx, get_child(branch, j));
          }
        } else {
          check(0, "only case or default node expected in switch node");
        }
      }
      printf("_%d:\n", ctx->break_label);
      printf("add esp, %d\n", WORD_SIZE);
      ctx->break_label = old_break_label;
      break;
    } 
    case noop_node:
    case var_decl_node:
      // do nothing
      break;
    default:
      generate_expr(ctx, stmt);
      break;
  }
}

void generate_stmts(struct CodegenContext* ctx, struct Node* stmts) {
  check(stmts->type == stmts_node, "generate_stmts");
  for (int i = 0; i < child_num(stmts); i++) {
    generate_stmt(ctx, get_child(stmts, i));
  }
}

void generate_code(struct ProcessedAst* input) {
  struct Node* root = input->ast;
  struct CodegenContext* ctx = new_codegen_context(input);
  check(root->type == prog_node, "prog_node expected");
  printf(".intel_syntax noprefix\n");
  printf(".section .data\n");
  // declare all global variables.
  for (int i = 0; i < child_num(root); i++) {
    struct Node* cur = get_child(root, i);
    switch (cur->type) {
      case function_impl_node:
      case function_decl_node:
      case var_init_node:
      case var_decl_node:
      case extern_var_decl_node: {
        // index 0 is type
        char* name = get_symbol(get_child(cur, 1));
        // expose function names and global var names
        printf(".globl %s\n", name);
        switch (cur->type) {
          case var_init_node: {
            // declare initialized global var
            check(child_num(cur) == 3 && get_child(cur, 2)->type == int_node,
                "only integer variable initialization is allowed\n");
            int value = get_int(get_child(cur, 2));
            printf("%s: .long %d\n", name, value);
            break;
          } 
          case var_decl_node: 
            // declare uninitialized global var
            printf("%s: .long 0\n", name);
            break;
        }
        break;
      }
    }
  }
  printf(".section .text\n");
  // generate code for functions
  for (int i = 0; i < child_num(root); i++) {
    struct Node* cur = get_child(root, i);
    switch (cur->type) {
      case function_impl_node: {
        char* name = get_symbol(get_child(cur, 1));
        struct Node* stmts = get_child(cur, 3);
        ctx->return_label = new_temp_label(ctx);

        ctx->lst = get_lst(ctx->gst, name);

        // function entry point
        printf("%s:\n", name);
        printf("push ebp\n");
        printf("mov ebp, esp\n");
        printf("sub esp, %d\n", local_var_count(ctx->lst) * WORD_SIZE);

        generate_stmts(ctx, stmts);
        // function epilog
        printf("_%d:\n", ctx->return_label);
        printf("mov esp, ebp\n");
        printf("pop ebp\n");
        printf("ret\n");

        ctx->lst = 0;

        break;
      }
    }
  }
}

void init_codegen() {
}
