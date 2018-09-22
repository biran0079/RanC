#ifndef __PARSER_H__
#define __PARSER_H__

#include "base.h"

#define MAX_NODE_NUM 10000

#define prog_node  0
#define var_decl_node  1
#define function_decl_node  2
#define function_impl_node  3
#define params_node  4
#define args_node  5
#define stmts_node  6
#define if_node  7
#define while_do_node  8
#define assignment_node  9
#define or_node  10
#define and_node  11
#define add_node  12
#define sub_node  13
#define mul_node  14
#define div_node  15
#define mod_node  16
#define negative_node  17
#define int_node  18
#define string_node  19
#define char_node  20
#define symbol_node  21
#define access_node  22
#define call_node  23
#define eq_node  24
#define lt_node  25
#define le_node  26
#define gt_node  27
#define ge_node  28
#define return_node  29
#define not_node  30
#define ne_node  31
#define extern_var_decl_node  32
#define var_init_node  33
#define break_node  34
#define continue_node  35
#define inc_prefix_node  36
#define dec_prefix_node  37
#define inc_suffix_node  38
#define dec_suffix_node  39
#define do_while_node  40
#define for_node  41
#define noop_node  42
#define add_eq_node  43
#define sub_eq_node  44
#define mul_eq_node  45
#define div_eq_node  46
#define ternary_condition_node  47
#define node_type_num  48


extern char** node_type_str;
extern int* node_type;
extern int** node_child;
extern int* node_child_num;
extern int* node_child_cap;
extern char** node_payload;

int parse();
void init_parser();

#endif
