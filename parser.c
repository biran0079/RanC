#include "tokenizer.h"
#include "parser.h"

char** node_type_str;
int* node_type;
int** node_child;
int* node_child_num;
int* node_child_cap;
char** node_payload;
int node_num = 0;

int next_token_idx;

void init_parser() {
  node_type = malloc(MAX_NODE_NUM * WORD_SIZE);
  node_child = malloc(MAX_NODE_NUM * WORD_SIZE);
  node_child_num = malloc(MAX_NODE_NUM * WORD_SIZE);
  node_child_cap = malloc(MAX_NODE_NUM * WORD_SIZE);
  node_payload = malloc(MAX_NODE_NUM * WORD_SIZE);
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
}

int new_node(int type) {
  int res = node_num++;
  check(node_num <= MAX_NODE_NUM, "too many nodes");
  node_type[res] = type;
  node_child_num[res] = 0;
  node_child_cap[res] = 2;
  node_child[res] = malloc(2 * WORD_SIZE);
  node_payload[res] = 0;
  return res;
}

int new_symbol_node(char* s) {
  int res = new_node(symbol_node);
  node_payload[res] = s;
  return res;
}

void append_child(int par, int child) {
  if (node_child_num[par] == node_child_cap[par]) {
    node_child_cap[par] = node_child_cap[par] * 2;
    node_child[par] = realloc(node_child[par], node_child_cap[par] * WORD_SIZE);
  }
  node_child[par][node_child_num[par]] = child;
  node_child_num[par]++;
}

int is_base_type(char* s) {
  return !strcmp(s, "char") || !strcmp(s, "int") || !strcmp(s, "void") || !strcmp(s, "enum");
}

void skip_comment_tokens() {
  while (next_token_idx < token_num && token_type[next_token_idx] == comment_token) {
    next_token_idx++;
  }
}

char* peek_token() {
  return token[next_token_idx];
}

int peek_token_type() {
  return token_type[next_token_idx];
}

void inc_next_token_idx() {
  next_token_idx++;
  skip_comment_tokens();
}

int matche_token(char* s) {
  if (!strcmp(s, peek_token())) {
    inc_next_token_idx();
    return 1;
  }
  return 0;
}

void check_and_ignore_token(char* s) {
  check(matche_token(s), "check_and_ignore_token failed\n");
}

void ignore_type() {
  check(is_base_type(peek_token()), "unknown type\n");
  matche_token("enum"); // skip enum keyword if any
  inc_next_token_idx(); // skip type/enum name
  while (matche_token("*")) { }
}

// Return node type for primitive token, or -1 if token is not primitive.
// No token is consumed.
int get_primitive_node_type() {
  int type = token_type[next_token_idx];
  if (type == int_token) {
    return int_node;
  } else if (type == string_token) {
    return string_node;
  } else if (type == char_token) {
    return char_node;
  }
  return -1;
}

int parse_expr();

int parse_object() {
  int res;
  if (token_type[next_token_idx] == symbol_token) {
    res = new_symbol_node(peek_token());
    inc_next_token_idx();
  } else if (matche_token("(")) {
    res = parse_expr();
    check_and_ignore_token(")");
  } else {
    check(0, "failed to parse object\n");
  }
  while (1) {
    if (matche_token("[")) {
      int arr_expr = res;
      int idx_expr = parse_expr();
      check_and_ignore_token("]");
      res = new_node(access_node);
      append_child(res, arr_expr);
      append_child(res, idx_expr);
    } else if (matche_token("(")) {
      int args = new_node(args_node);
      while (!matche_token(")")) {
        append_child(args, parse_expr());
        matche_token(","); // does not match for the last arg
      }
      int func = res;
      res = new_node(call_node);
      append_child(res, func);
      append_child(res, args);
    } else {
      break;
    }
  }
  return res;
}

int parse_expr0() {
  int res;
  if (matche_token("-")) {
    if (token_type[next_token_idx] == int_token) {
      // -1
      char* num = peek_token();
      char* negative = malloc(strlen(num) + 2);
      negative[0] = 0;
      strcat(negative, "-");
      strcat(negative, num);
      res = new_node(int_node);
      node_payload[res] = negative;
      inc_next_token_idx();
    } else {
      // -a
      res = new_node(negative_node);
      append_child(res, parse_expr0());
    }
    return res;
  }
  int type = get_primitive_node_type();
  if (type >= 0) {
    res = new_node(type);
    node_payload[res] = peek_token();
    inc_next_token_idx();
    return res;
  }
  if (matche_token("++")) {
    res = new_node(inc_prefix_node);
    append_child(res, parse_object());
  } else if (matche_token("--")) {
    res = new_node(dec_prefix_node);
    append_child(res, parse_object());
  } else {
    res = parse_object();
    int t;
    if (matche_token("++")) {
      t = res;
      res = new_node(inc_suffix_node);
      append_child(res, t);
    } else if (matche_token("--")) {
      t = res;
      res = new_node(dec_suffix_node);
      append_child(res, t);
    }
  }
  return res;
}

int parse_expr1() {
  int res = parse_expr0();
  while (1) {
    int op;
    if (matche_token("*")) {
      op = mul_node;
    } else if (matche_token("/")) {
      op = div_node;
    } else if (matche_token("%")) {
      op = mod_node;
    } else {
      break;
    }
    int left = res;
    res = new_node(op);
    // * % / are left associative
    append_child(res, left);
    append_child(res, parse_expr0());
  }
  return res;
}

int parse_expr2() {
  int res = parse_expr1();
  while (1) {
    int op;
    if (matche_token("+")) {
      op = add_node;
    } else if (matche_token("-")) {
      op = sub_node;
    } else {
      break;
    }
    int left = res;
    res = new_node(op);
    // + - are left associative
    append_child(res, left);
    append_child(res, parse_expr1());
  }
  return res;
}

// Consume comparison token and return corresponding node type.
// If no comparison token found then return -1 and no token is not consumed.
int get_cmp_node_type() {
  if (matche_token("==")) {
    return eq_node;
  } else if (matche_token("!=")) {
    return ne_node;
  } else if (matche_token("<")) {
    return lt_node;
  } else if (matche_token("<=")) {
    return le_node;
  } else if (matche_token(">")) {
    return gt_node;
  } else if (matche_token(">=")) {
    return ge_node;
  }
  return -1;
}

int parse_expr3() {
  int res;
  if (matche_token("!")) {
    res = new_node(not_node);
    append_child(res, parse_expr3());
  } else {
    int expr = parse_expr2();
    int cmp_node_type = get_cmp_node_type();
    if (cmp_node_type >= 0) {
      res = new_node(cmp_node_type);
      append_child(res, expr);
      append_child(res, parse_expr2());
    } else {
      res = expr;
    }
  }
  return res;
}

int parse_expr4() {
  int expr = parse_expr3();
  if (!strcmp("&&", peek_token())) {
    int res = new_node(and_node);
    append_child(res, expr);
    while (matche_token("&&")) {
      append_child(res, parse_expr3());
    }
    return res;
  }
  return expr;
}

int parse_expr5() {
  int expr = parse_expr4();
  if (!strcmp("||", peek_token())) {
    int res = new_node(or_node);
    append_child(res, expr);
    while (matche_token("||")) {
      append_child(res, parse_expr4());
    }
    return res;
  }
  return expr;
}

int parse_expr6() {
  int exp = parse_expr5();
  if (matche_token("?")) {
    int res = new_node(ternary_condition_node);
    append_child(res, exp);
    append_child(res, parse_expr6());
    check_and_ignore_token(":");
    append_child(res, parse_expr6());
    return res;
  }
  return exp;
}

int parse_expr7() {
  int expr = parse_expr6();
  if (matche_token("=")) {
    int res = new_node(assignment_node);
    append_child(res, expr);
    // right associative
    append_child(res, parse_expr7());
    return res;
  } else if (matche_token("+=")) {
    int res = new_node(add_eq_node);
    append_child(res, expr);
    append_child(res, parse_expr7());
    return res;
  } else if (matche_token("-=")) {
    int res = new_node(sub_eq_node);
    append_child(res, expr);
    append_child(res, parse_expr7());
    return res;
  } else if (matche_token("*=")) {
    int res = new_node(mul_eq_node);
    append_child(res, expr);
    append_child(res, parse_expr7());
    return res;
  } else if (matche_token("/=")) {
    int res = new_node(div_eq_node);
    append_child(res, expr);
    append_child(res, parse_expr7());
    return res;
  }
  return expr;
}

int parse_expr() {
  return parse_expr7();
}

int parse_stmt();
int parse_block();

int parse_if() {
  int res = new_node(if_node);
  check_and_ignore_token("(");
  append_child(res, parse_expr());
  check_and_ignore_token(")");
  append_child(res, parse_block());
  if (matche_token("else")) {
    if (!strcmp(peek_token(), "if")) {
      append_child(res, parse_stmt());
    } else {
      append_child(res, parse_block());
    }
  }
  return res;
}

int parse_while() {
  int res = new_node(while_do_node);
  check_and_ignore_token("(");
  append_child(res, parse_expr());
  check_and_ignore_token(")");
  append_child(res, parse_block());
  return res;
}

int parse_do_while() {
  int res = new_node(do_while_node);
  append_child(res, parse_block());
  check_and_ignore_token("while");
  check_and_ignore_token("(");
  append_child(res, parse_expr());
  check_and_ignore_token(")");
  check_and_ignore_token(";");
  return res;
}

int parse_decl();

int parse_stmt();

int noop_expr() {
  return new_node(noop_node);
}

int parse_for() {
  int res = new_node(for_node);
  check_and_ignore_token("(");
  append_child(res, parse_stmt());
  if (!strcmp(peek_token(), ";")) {
    append_child(res, noop_expr());
  } else {
    append_child(res, parse_expr());
  }
  check_and_ignore_token(";");
  if (!strcmp(peek_token(), ")")) {
    append_child(res, noop_expr());
  } else {
    append_child(res, parse_expr());
  }
  check_and_ignore_token(")");
  append_child(res, parse_block());
  return res;
}

int parse_return() {
  int res = new_node(return_node);
  if (!matche_token(";")) {
    append_child(res, parse_expr());
    check_and_ignore_token(";");
  }
  return res;
}

int parse_stmt() {
  if (matche_token("if")) {
    return parse_if();
  } else if (matche_token("while")) {
    return parse_while();
  } else if (matche_token("do")) {
    return parse_do_while();
  } else if (matche_token("for")) {
    return parse_for();
  } else if (matche_token("return")) {
    return parse_return();
  } else if (matche_token("break")) {
    int res = new_node(break_node);
    check_and_ignore_token(";");
    return res;
  } else if (matche_token("continue")) {
    int res = new_node(continue_node);
    check_and_ignore_token(";");
    return res;
  } else if (is_base_type(peek_token())) {
    // If expression starts with a type, then parse as declaration.
    // parse_decl() handle function declaration as well, 
    // though only variable declaration is allowed
    return parse_decl();
  } else if (matche_token(";")) {
    return noop_expr();
  }
  int res = parse_expr();
  check_and_ignore_token(";");
  return res;
}

int parse_block() {
  int res = new_node(stmts_node);
  node_type[res] = stmts_node;
  check_and_ignore_token("{");
  while (!matche_token("}")) {
    append_child(res, parse_stmt());
  }
  return res;
}

int parse_params() {
  check_and_ignore_token("(");
  int params = new_node(params_node);
  node_type[params] = params_node;
  while (!matche_token(")")) {
    ignore_type();
    char* param_name = peek_token();
    inc_next_token_idx();
    append_child(params, new_symbol_node(param_name));
    matche_token(","); // Won't match for the last param.
  }
  return params;
}

int parse_decl() {
  int extern_decl = 0;
  if (matche_token("extern")) {
    extern_decl = 1;
  }
  if (matche_token("enum")) {
    int res = new_node(enum_node);
    append_child(res, new_symbol_node(peek_token()));
    inc_next_token_idx();
    check_and_ignore_token("{");
    while (!matche_token("}")) {
      int value = new_node(enum_value_node);
      append_child(res, value);
      append_child(value, new_symbol_node(peek_token()));
      inc_next_token_idx();
      if (matche_token("=")) {
        check(peek_token_type() == int_token, "int token expected for enum initialization");
        int int_value = new_node(int_node);
        node_payload[next_token_idx] = peek_token();
        append_child(value, int_value);
        inc_next_token_idx();
      }
      matche_token(",");
    }
    check_and_ignore_token(";");
    return res;
  } else {
    ignore_type();
  }
  int res;
  char* name = peek_token();
  inc_next_token_idx();
  if (!strcmp(peek_token(), "(")) {
    int params = parse_params();
    if (!strcmp(peek_token(), "{")) {
      res = new_node(function_impl_node);
      append_child(res, new_symbol_node(name));
      append_child(res, params);
      append_child(res, parse_block());
    } else {
      res = new_node(function_decl_node);
      append_child(res, new_symbol_node(name));
      append_child(res, params);
      check_and_ignore_token(";");
    }
  } else {
    if (extern_decl) {
      res = new_node(extern_var_decl_node);
    } else if (!strcmp(peek_token(), "=")) {
      res = new_node(var_init_node);
    } else {
      res = new_node(var_decl_node);
    }
    append_child(res, new_symbol_node(name));
    if (matche_token("=")) {
      append_child(res, parse_expr());
    } 
    check_and_ignore_token(";");
  }
  return res;
}

int parse_prog() {
  int res = new_node(prog_node);
  while (token_type[next_token_idx] != eof_token) {
    append_child(res, parse_decl());
  }
  return res;
}

int parse() {
  next_token_idx = 0;
  skip_comment_tokens();
  return parse_prog();
}

void print_space(int n) {
  while (n--) {
    printf(" ");
  }
}

void print_ast_internal(int root, int indent) {
  print_space(indent);
  printf("(%s", node_type_str[node_type[root]]);
  if (node_payload[root]) {
    printf(" %s", node_payload[root]);
  }
  for (int i = 0; i < node_child_num[root]; i++) {
    printf("\n");
    print_ast_internal(node_child[root][i], indent + 2);
  }
  printf(")");
}

void print_ast(int root) {
  print_ast_internal(root, 0); 
  printf("\n");
}
