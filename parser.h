#ifndef __PARSER_H__
#define __PARSER_H__

#include "base.h"
#include "list.h"
#include "tokenizer.h"

enum NodeType {
  invalid_node,

  prog_node,
  var_decl_node,
  function_decl_node,
  function_impl_node,
  params_node,
  args_node,
  stmts_node,
  if_node,
  while_do_node,
  assignment_node,
  or_node,
  and_node,
  add_node,
  sub_node,
  mul_node,
  div_node,
  mod_node,
  negative_node,
  int_node,
  string_node,
  char_node,
  symbol_node,
  access_node,
  call_node,
  eq_node,
  lt_node,
  le_node,
  gt_node,
  ge_node,
  return_node,
  not_node,
  ne_node,
  extern_var_decl_node,
  var_init_node,
  break_node,
  continue_node,
  inc_prefix_node,
  dec_prefix_node,
  inc_suffix_node,
  dec_suffix_node,
  do_while_node,
  for_node,
  noop_node,
  add_eq_node,
  sub_eq_node,
  mul_eq_node,
  div_eq_node,
  ternary_condition_node,
  enum_node,
  enum_value_node,
  param_node,
  sizeof_node,
  address_of_node,
  dereference_node,
  struct_access_node,
  struct_ptr_access_node,
  struct_node,

  ptr_type_node,
  int_type_node,
  char_type_node,
  void_type_node,
  enum_type_node,
  struct_type_node,
  function_type_node,
  switch_node,
  case_node,
  default_node,

  node_type_num,
};

struct Node {
  enum NodeType type;
  struct List* child;
  char* payload;
};

extern char** node_type_str;

void init_parser();

struct Node* parse(struct List* tokens);

struct Node* new_node(enum NodeType type);

struct Node* new_node_with_payload(enum NodeType type, char* s);

struct Node* get_child(struct Node* root, int i);

int child_num(struct Node* root);

void append_child(struct Node* p, struct Node* c);

void print_ast(struct Node* root);


#endif
