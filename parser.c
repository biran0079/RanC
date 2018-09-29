#include "tokenizer.h"
#include "parser.h"

char** node_type_str;

struct ParserContext {
  struct List* tokens;
  int idx;
};

void init_parser() {
  // segfalut without -m32 because node_type_str will overflow
  node_type_str = malloc(node_type_num * WORD_SIZE);
  node_type_str[prog_node] = "prog";
  node_type_str[var_decl_node] = "var";
  node_type_str[function_decl_node] = "function";
  node_type_str[function_impl_node] = "function";
  node_type_str[params_node] = "params";
  node_type_str[args_node] = "args";
  node_type_str[stmts_node] = "stmts";
  node_type_str[if_node] = "if";
  node_type_str[while_do_node] = "while";
  node_type_str[assignment_node] = "assignment";
  node_type_str[or_node] = "or";
  node_type_str[and_node] = "and";
  node_type_str[add_node] = "add";
  node_type_str[sub_node] = "sub";
  node_type_str[mul_node] = "mul";
  node_type_str[div_node] = "div";
  node_type_str[mod_node] = "mod";
  node_type_str[negative_node] = "negative";
  node_type_str[int_node] = "int";
  node_type_str[string_node] = "string";
  node_type_str[char_node] = "char";
  node_type_str[symbol_node] = "symbol";
  node_type_str[access_node] = "access";
  node_type_str[call_node] = "call";
  node_type_str[eq_node] = "eq";
  node_type_str[lt_node] = "lt";
  node_type_str[le_node] = "le";
  node_type_str[gt_node] = "gt";
  node_type_str[ge_node] = "ge";
  node_type_str[return_node] = "return";
  node_type_str[not_node] = "not";
  node_type_str[ne_node] = "ne";
  node_type_str[extern_var_decl_node] = "extern_var";
  node_type_str[var_init_node] = "var_init";
  node_type_str[break_node] = "break_node";
  node_type_str[continue_node] = "continue_node";
  node_type_str[inc_prefix_node] = "inc_prefix";
  node_type_str[dec_prefix_node] = "dec_prefix";
  node_type_str[inc_suffix_node] = "inc_suffix";
  node_type_str[dec_suffix_node] = "dec_suffix";
  node_type_str[do_while_node] = "do_while";
  node_type_str[for_node] = "for";
  node_type_str[noop_node] = "noop";
  node_type_str[add_eq_node] = "+=";
  node_type_str[sub_eq_node] = "-=";
  node_type_str[mul_eq_node] = "*=";
  node_type_str[div_eq_node] = "/=";
  node_type_str[ternary_condition_node] = "?:";
  node_type_str[enum_node] = "enum";
  node_type_str[enum_value_node] = "enum_value";
  node_type_str[param_node] = "param";
  node_type_str[sizeof_node] = "sizeof";
  node_type_str[address_of_node] = "address_of";
  node_type_str[dereference_node] = "dereference";
  node_type_str[struct_access_node] = "struct_access";
  node_type_str[struct_ptr_access_node] = "struct_ptr_access";
  node_type_str[struct_node] = "struct";
  node_type_str[switch_node] = "switch";
  node_type_str[case_node] = "case";
  node_type_str[default_node] = "default";

  node_type_str[ptr_type_node] = "ptr_type";
  node_type_str[int_type_node] = "int_type";
  node_type_str[char_type_node] = "char_type";
  node_type_str[void_type_node] = "void_type";
  node_type_str[function_type_node] = "function_type";
  node_type_str[struct_type_node] = "struct_type";

  node_type_str[invalid_node] = "invalid";
}

struct Node* new_node(enum NodeType type) {
  struct Node* res = malloc(sizeof(struct Node));
  res->type = type;
  res->child = new_list();
  res->payload = 0;
  return res;
}

struct Node* new_node_with_payload(enum NodeType type, char* s) {
  struct Node* res = new_node(type);
  res->payload = s;
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

int child_num(struct Node* root) {
  return list_size(root->child);
}

struct Node* get_child(struct Node* root, int i) {
  return list_get(root->child, i);
}

void append_child(struct Node* p, struct Node* c) {
  list_add(p->child, c);
}

int is_base_type(char* s) {
  return !strcmp(s, "char") || !strcmp(s, "int") || !strcmp(s, "void") 
      || !strcmp(s, "enum") || !strcmp(s, "struct");
}

char* peek_token(struct ParserContext* ctx) {
  struct Token* t = list_get(ctx->tokens, ctx->idx);
  return t->s;
}

char* look_ahead(struct ParserContext* ctx, int n) {
  struct Token* t = list_get(ctx->tokens, ctx->idx + n);
  return t->s;
}

enum TokenType peek_token_type(struct ParserContext* ctx) {
  struct Token* t = list_get(ctx->tokens, ctx->idx);
  return t->type;
}

// read and move forward
char* next_token(struct ParserContext* ctx) {
  struct Token* t = list_get(ctx->tokens, ctx->idx++);
  return t->s;
}

int match_token(struct ParserContext* ctx, char* s) {
  if (!strcmp(s, peek_token(ctx))) {
    ctx->idx++;
    return 1;
  }
  return 0;
}

void check_and_ignore_token(struct ParserContext* ctx, char* s) {
  check(match_token(ctx, s), "check_and_ignore_token failed\n");
}

struct Node* parse_type(struct ParserContext* ctx) {
  struct Node* res;
  if (match_token(ctx, "enum")) {
    res = new_node(enum_type_node);
    append_child(res, new_node_with_payload(symbol_node, next_token(ctx)));
  } else if (match_token(ctx, "struct")) {
    res = new_node(struct_type_node);
    append_child(res, new_node_with_payload(symbol_node, next_token(ctx)));
  } else if (match_token(ctx, "int")) {
    res = new_node(int_type_node);
  } else if (match_token(ctx, "char")) {
    res = new_node(char_type_node);
  } else if (match_token(ctx, "void")) {
    res = new_node(void_type_node);
  } else {
    check(0, "unknown type\n");
  }
  while (match_token(ctx, "*")) { 
    struct Node* t = res;
    res = new_node(ptr_type_node);
    append_child(res, t);  
  }
  return res;
}

// Return node type for primitive token, or -1 if token is not primitive.
// No token is consumed.
enum NodeType get_primitive_node_type(struct ParserContext* ctx) {
  // string is handled by parse_object() because string literal allows array access
  enum NodeType type = peek_token_type(ctx);
  if (type == int_token) {
    return int_node;
  } else if (type == char_token) {
    return char_node;
  }
  return invalid_node;
}

struct Node* parse_expr(struct ParserContext* ctx);

struct Node* parse_object(struct ParserContext* ctx) {
  struct Node* res;
  if (peek_token_type(ctx) == symbol_token) {
    res = new_node_with_payload(symbol_node, next_token(ctx));
  } else if (peek_token_type(ctx) == string_token) {
    res = new_node_with_payload(string_node, next_token(ctx));
  } else if (match_token(ctx, "(")) {
    res = parse_expr(ctx);
    check_and_ignore_token(ctx, ")");
  } else {
    check(0, "failed to parse object\n");
  }
  while (1) {
    if (match_token(ctx, "[")) {
      struct Node* arr_expr = res;
      struct Node* idx_expr = parse_expr(ctx);
      check_and_ignore_token(ctx, "]");
      res = new_node(access_node);
      append_child(res, arr_expr);
      append_child(res, idx_expr);
    } else if (match_token(ctx, "(")) {
      struct Node* args = new_node(args_node);
      while (!match_token(ctx, ")")) {
        append_child(args, parse_expr(ctx));
        match_token(ctx, ","); // does not match for the last arg
      }
      struct Node* func = res;
      res = new_node(call_node);
      append_child(res, func);
      append_child(res, args);
    } else if (match_token(ctx, "->")) {
      struct Node* t = new_node(struct_ptr_access_node);
      append_child(t, res);
      check(peek_token_type(ctx) == symbol_token, "-> only follows symbol token");
      append_child(t, new_node_with_payload(symbol_node, next_token(ctx)));
      res = t;
    } else if (match_token(ctx, ".")) {
      struct Node* t = new_node(struct_access_node);
      append_child(t, res);
      check(peek_token_type(ctx) == symbol_token, ". only follows symbol token");
      append_child(t, new_node_with_payload(symbol_node, next_token(ctx)));
      res = t;
    } else {
      break;
    }
  }
  return res;
}

struct Node* parse_expr0(struct ParserContext* ctx) {
  struct Node* res;
  if (match_token(ctx, "-")) {
    if (peek_token_type(ctx) == int_token) {
      // -1
      char* num = next_token(ctx);
      char* negative = malloc(strlen(num) + 2);
      negative[0] = 0;
      strcat(negative, "-");
      strcat(negative, num);
      res = new_node_with_payload(int_node, negative);
    } else {
      // -a
      res = new_node(negative_node);
      append_child(res, parse_expr0(ctx));
    }
    return res;
  }
  enum NodeType type = get_primitive_node_type(ctx);
  if (type != invalid_node) {
    res = new_node_with_payload(type, next_token(ctx));
    return res;
  }
  if (match_token(ctx, "++")) {
    res = new_node(inc_prefix_node);
    append_child(res, parse_object(ctx));
  } else if (match_token(ctx, "--")) {
    res = new_node(dec_prefix_node);
    append_child(res, parse_object(ctx));
  } else {
    res = parse_object(ctx);
  }
  struct Node* t;
  if (match_token(ctx, "++")) {
    t = res;
    res = new_node(inc_suffix_node);
    append_child(res, t);
  } else if (match_token(ctx, "--")) {
    t = res;
    res = new_node(dec_suffix_node);
    append_child(res, t);
  }
  return res;
}

struct Node* parse_expr1(struct ParserContext* ctx) {
  if (match_token(ctx, "sizeof")) {
    struct Node* res = new_node(sizeof_node);
    int open_paren = match_token(ctx, "(");
    if (is_base_type(peek_token(ctx))) {
      append_child(res, parse_type(ctx));
    } else {
      if (open_paren) {
        append_child(res, parse_expr(ctx));
      } else {
        append_child(res, parse_expr1(ctx));
      }
    }
    if (open_paren) {
      check_and_ignore_token(ctx, ")");
    }
    return res;
  } else if (match_token(ctx, "*")) {
    struct Node* res = new_node(dereference_node);
    append_child(res, parse_expr1(ctx));
    return res;
  } else if (match_token(ctx, "&")) {
    struct Node* res = new_node(address_of_node);
    append_child(res, parse_expr1(ctx));
    return res;
  }
  return parse_expr0(ctx);
}

struct Node* parse_expr2(struct ParserContext* ctx) {
  struct Node* res = parse_expr1(ctx);
  while (1) {
    enum NodeType op;
    if (match_token(ctx, "*")) {
      op = mul_node;
    } else if (match_token(ctx, "/")) {
      op = div_node;
    } else if (match_token(ctx, "%")) {
      op = mod_node;
    } else {
      break;
    }
    struct Node* left = res;
    res = new_node(op);
    // * % / are left associative
    append_child(res, left);
    append_child(res, parse_expr1(ctx));
  }
  return res;
}

struct Node* parse_expr3(struct ParserContext* ctx) {
  struct Node* res = parse_expr2(ctx);
  while (1) {
    enum NodeType op;
    if (match_token(ctx, "+")) {
      op = add_node;
    } else if (match_token(ctx, "-")) {
      op = sub_node;
    } else {
      break;
    }
    struct Node* left = res;
    res = new_node(op);
    // + - are left associative
    append_child(res, left);
    append_child(res, parse_expr2(ctx));
  }
  return res;
}

// Consume comparison token and return corresponding node type.
// If no comparison token found then return -1 and no token is not consumed.
enum NodeType get_cmp_node_type(struct ParserContext* ctx) {
  if (match_token(ctx, "==")) {
    return eq_node;
  } else if (match_token(ctx, "!=")) {
    return ne_node;
  } else if (match_token(ctx, "<")) {
    return lt_node;
  } else if (match_token(ctx, "<=")) {
    return le_node;
  } else if (match_token(ctx, ">")) {
    return gt_node;
  } else if (match_token(ctx, ">=")) {
    return ge_node;
  }
  return invalid_node;
}

struct Node* parse_expr4(struct ParserContext* ctx) {
  struct Node* res;
  if (match_token(ctx, "!")) {
    res = new_node(not_node);
    append_child(res, parse_expr4(ctx));
  } else {
    struct Node* expr = parse_expr3(ctx);
    enum NodeType cmp_node_type = get_cmp_node_type(ctx);
    if (cmp_node_type != invalid_node) {
      res = new_node(cmp_node_type);
      append_child(res, expr);
      append_child(res, parse_expr3(ctx));
    } else {
      res = expr;
    }
  }
  return res;
}

struct Node* parse_expr5(struct ParserContext* ctx) {
  struct Node* expr = parse_expr4(ctx);
  if (!strcmp("&&", peek_token(ctx))) {
    struct Node* res = new_node(and_node);
    append_child(res, expr);
    while (match_token(ctx, "&&")) {
      append_child(res, parse_expr4(ctx));
    }
    return res;
  }
  return expr;
}

struct Node* parse_expr6(struct ParserContext* ctx) {
  struct Node* expr = parse_expr5(ctx);
  if (!strcmp("||", peek_token(ctx))) {
    struct Node* res = new_node(or_node);
    append_child(res, expr);
    while (match_token(ctx, "||")) {
      append_child(res, parse_expr5(ctx));
    }
    return res;
  }
  return expr;
}

struct Node* parse_expr7(struct ParserContext* ctx) {
  struct Node* exp = parse_expr6(ctx);
  if (match_token(ctx, "?")) {
    struct Node* res = new_node(ternary_condition_node);
    append_child(res, exp);
    append_child(res, parse_expr7(ctx));
    check_and_ignore_token(ctx, ":");
    append_child(res, parse_expr7(ctx));
    return res;
  }
  return exp;
}

struct Node* parse_expr8(struct ParserContext* ctx) {
  struct Node* expr = parse_expr7(ctx);
  if (match_token(ctx, "=")) {
    struct Node* res = new_node(assignment_node);
    append_child(res, expr);
    // right associative
    append_child(res, parse_expr8(ctx));
    return res;
  } else if (match_token(ctx, "+=")) {
    struct Node* res = new_node(add_eq_node);
    append_child(res, expr);
    append_child(res, parse_expr8(ctx));
    return res;
  } else if (match_token(ctx, "-=")) {
    struct Node* res = new_node(sub_eq_node);
    append_child(res, expr);
    append_child(res, parse_expr8(ctx));
    return res;
  } else if (match_token(ctx, "*=")) {
    struct Node* res = new_node(mul_eq_node);
    append_child(res, expr);
    append_child(res, parse_expr8(ctx));
    return res;
  } else if (match_token(ctx, "/=")) {
    struct Node* res = new_node(div_eq_node);
    append_child(res, expr);
    append_child(res, parse_expr8(ctx));
    return res;
  }
  return expr;
}

struct Node* parse_expr(struct ParserContext* ctx) {
  return parse_expr8(ctx);
}

struct Node* parse_stmt(struct ParserContext* ctx);
struct Node* parse_block(struct ParserContext* ctx);

struct Node* parse_if(struct ParserContext* ctx) {
  struct Node* res = new_node(if_node);
  check_and_ignore_token(ctx, "(");
  append_child(res, parse_expr(ctx));
  check_and_ignore_token(ctx, ")");
  append_child(res, parse_block(ctx));
  if (match_token(ctx, "else")) {
    if (!strcmp(peek_token(ctx), "if")) {
      append_child(res, parse_stmt(ctx));
    } else {
      append_child(res, parse_block(ctx));
    }
  }
  return res;
}

struct Node* parse_while(struct ParserContext* ctx) {
  struct Node* res = new_node(while_do_node);
  check_and_ignore_token(ctx, "(");
  append_child(res, parse_expr(ctx));
  check_and_ignore_token(ctx, ")");
  append_child(res, parse_block(ctx));
  return res;
}

struct Node* parse_do_while(struct ParserContext* ctx) {
  struct Node* res = new_node(do_while_node);
  append_child(res, parse_block(ctx));
  check_and_ignore_token(ctx, "while");
  check_and_ignore_token(ctx, "(");
  append_child(res, parse_expr(ctx));
  check_and_ignore_token(ctx, ")");
  check_and_ignore_token(ctx, ";");
  return res;
}

struct Node* parse_switch(struct ParserContext* ctx) {
  struct Node* res = new_node(switch_node);
  check_and_ignore_token(ctx, "(");
  append_child(res, parse_expr(ctx));
  check_and_ignore_token(ctx, ")");
  check_and_ignore_token(ctx, "{");
  while (1) {
    struct Node* t;
    int default_count = 0;
    if (match_token(ctx, "case")) {
      t = new_node(case_node);
      append_child(t, parse_expr0(ctx)); // int, char literal or enum
      check_and_ignore_token(ctx, ":");
      int open_paren = match_token(ctx, "{");
      while (1) {
        char* token = peek_token(ctx);
        if (!strcmp(token, "case") || !strcmp(token, "default") || !strcmp(token, "}")) {
          break;
        }
        append_child(t, parse_stmt(ctx));
      }
      if (open_paren) {
        check_and_ignore_token(ctx, "}");
      }
    } else if (match_token(ctx, "default")) {
      check(default_count++ == 0, "only one default per switch");
      t = new_node(default_node);
      check_and_ignore_token(ctx, ":");
      int open_paren = match_token(ctx, "{");
      while (1) {
        char* token = peek_token(ctx);
        if (!strcmp(token, "}") || !strcmp(token, "case")) {
          break;
        }
        append_child(t, parse_stmt(ctx));
      }
      if (open_paren) {
        check_and_ignore_token(ctx, "}");
      }
    } else {
      break;
    }
    append_child(res, t);
  }
  check_and_ignore_token(ctx, "}");
  return res;
}

struct Node* parse_decl(struct ParserContext* ctx);

struct Node* parse_stmt(struct ParserContext* ctx);

struct Node* noop_expr() {
  return new_node(noop_node);
}

struct Node* parse_for(struct ParserContext* ctx) {
  struct Node* res = new_node(for_node);
  check_and_ignore_token(ctx, "(");
  append_child(res, parse_stmt(ctx));
  if (!strcmp(peek_token(ctx), ";")) {
    append_child(res, noop_expr());
  } else {
    append_child(res, parse_expr(ctx));
  }
  check_and_ignore_token(ctx, ";");
  if (!strcmp(peek_token(ctx), ")")) {
    append_child(res, noop_expr());
  } else {
    append_child(res, parse_expr(ctx));
  }
  check_and_ignore_token(ctx, ")");
  append_child(res, parse_block(ctx));
  return res;
}

struct Node* parse_return(struct ParserContext* ctx) {
  struct Node* res = new_node(return_node);
  if (!match_token(ctx, ";")) {
    append_child(res, parse_expr(ctx));
    check_and_ignore_token(ctx, ";");
  }
  return res;
}

struct Node* parse_stmt(struct ParserContext* ctx) {
  if (match_token(ctx, "if")) {
    return parse_if(ctx);
  } else if (match_token(ctx, "while")) {
    return parse_while(ctx);
  } else if (match_token(ctx, "do")) {
    return parse_do_while(ctx);
  } else if (match_token(ctx, "for")) {
    return parse_for(ctx);
  } else if (match_token(ctx, "return")) {
    return parse_return(ctx);
  } else if (match_token(ctx, "break")) {
    struct Node* res = new_node(break_node);
    check_and_ignore_token(ctx, ";");
    return res;
  } else if (match_token(ctx, "continue")) {
    struct Node* res = new_node(continue_node);
    check_and_ignore_token(ctx, ";");
    return res;
  } else if (match_token(ctx, "switch")) {
    return parse_switch(ctx);
  } else if (is_base_type(peek_token(ctx))) {
    // If expression starts with a type, then parse as declaration.
    // parse_decl(ctx) handle function declaration as well, 
    // though only variable declaration is allowed
    return parse_decl(ctx);
  } else if (match_token(ctx, ";")) {
    return noop_expr();
  }
  struct Node* res = parse_expr(ctx);
  check_and_ignore_token(ctx, ";");
  return res;
}

struct Node* parse_block(struct ParserContext* ctx) {
  struct Node* res = new_node(stmts_node);
  check_and_ignore_token(ctx, "{");
  while (!match_token(ctx, "}")) {
    append_child(res, parse_stmt(ctx));
  }
  return res;
}

struct Node* parse_params(struct ParserContext* ctx) {
  check_and_ignore_token(ctx, "(");
  struct Node* params = new_node(params_node);
  while (!match_token(ctx, ")")) {
    struct Node* param = new_node(param_node);
    append_child(params, param);
    struct Node* type_node = parse_type(ctx);
    check(type_node->type != struct_type_node, "only struct pointer param is supported for now");
    append_child(param, type_node);
    append_child(param, new_node_with_payload(symbol_node, next_token(ctx)));
    match_token(ctx, ","); // Won't match for the last param.
  }
  return params;
}

struct Node* parse_decl(struct ParserContext* ctx) {
  int extern_decl = 0;
  if (match_token(ctx, "extern")) {
    extern_decl = 1;
  }
  if (!strcmp(peek_token(ctx), "enum") && !strcmp(look_ahead(ctx, 2), "{")) {
    // enum def
    ctx->idx++; // skip "enum"
    struct Node* res = new_node(enum_node);
    append_child(res, new_node_with_payload(symbol_node, next_token(ctx)));
    check_and_ignore_token(ctx, "{");
    while (!match_token(ctx, "}")) {
      struct Node* value = new_node(enum_value_node);
      append_child(res, value);
      append_child(value, new_node_with_payload(symbol_node, next_token(ctx)));
      if (match_token(ctx, "=")) {
        check(peek_token_type(ctx) == int_token, "int token expected for enum initialization");
        struct Node* int_value = new_node_with_payload(int_node, next_token(ctx));
        append_child(value, int_value);
      }
      match_token(ctx, ",");
    }
    check_and_ignore_token(ctx, ";");
    return res;
  } else if (!strcmp(peek_token(ctx), "struct") && !strcmp(look_ahead(ctx, 2), "{")) {
    // struct def
    ctx->idx++; // skip "struct"
    struct Node* res = new_node(struct_node);
    append_child(res, new_node_with_payload(symbol_node, next_token(ctx)));
    check_and_ignore_token(ctx, "{");
    while (!match_token(ctx, "}")) {
      struct Node* decl = parse_decl(ctx);
      check(decl->type == var_decl_node, "only variable delaration is allowed in struct");
      append_child(res, decl);
    }
    check_and_ignore_token(ctx, ";");
    return res;
  } 
  struct Node* type_node = parse_type(ctx);
  check(type_node->type != struct_type_node, "only struct pointer is supported for now");
  struct Node* res;
  char* name = next_token(ctx);
  if (!strcmp(peek_token(ctx), "(")) {
    struct Node* params = parse_params(ctx);
    if (!strcmp(peek_token(ctx), "{")) {
      res = new_node(function_impl_node);
      append_child(res, type_node);
      append_child(res, new_node_with_payload(symbol_node, name));
      append_child(res, params);
      append_child(res, parse_block(ctx));
    } else {
      res = new_node(function_decl_node);
      append_child(res, type_node);
      append_child(res, new_node_with_payload(symbol_node, name));
      append_child(res, params);
      check_and_ignore_token(ctx, ";");
    }
  } else {
    if (extern_decl) {
      res = new_node(extern_var_decl_node);
    } else if (!strcmp(peek_token(ctx), "=")) {
      res = new_node(var_init_node);
    } else {
      res = new_node(var_decl_node);
    }
    append_child(res, type_node);
    append_child(res, new_node_with_payload(symbol_node, name));
    if (match_token(ctx, "=")) {
      append_child(res, parse_expr(ctx));
    } 
    check_and_ignore_token(ctx, ";");
  }
  return res;
}

struct Node* parse_prog(struct ParserContext* ctx) {
  struct Node* res = new_node(prog_node);
  while (peek_token_type(ctx) != eof_token) {
    append_child(res, parse_decl(ctx));
  }
  return res;
}

struct Node* parse(struct List* tokens) {
  struct ParserContext* ctx = malloc(sizeof(struct ParserContext));
  ctx->idx = 0;
  ctx->tokens = tokens;
  return parse_prog(ctx);
}

void print_space(int n) {
  while (n--) {
    printf(" ");
  }
}

void print_ast_internal(struct Node* root, int indent) {
  print_space(indent);
  printf("(%s", node_type_str[root->type]);
  if (root->payload) {
    printf(" %s", root->payload);
  }
  for (int i = 0; i < list_size(root->child); i++) {
    printf("\n");
    print_ast_internal(get_child(root, i), indent + 2);
  }
  printf(")");
}

void print_ast(struct Node* root) {
  print_ast_internal(root, 0); 
  printf("\n");
}
