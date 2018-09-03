char getchar();
int printf();
void exit();
int strcmp();
char* strdup();
void* malloc();
void free();
int strchr();

int EOF;

int int_token = 0;
int string_token = 1;
int char_token  = 2;
int symbol_token = 3;
int operator_token = 4;
int other_token = 5;
int MAX_TOKEN_LEN = 1023;
int MAX_LOCAL_COUNT = 1024;
int MAX_FUNCTION_COUNT = 10000;
int WORD_SIZE = 4;

char* token;
int token_len; 
int token_type; 
char* peeked_token = 0;
int peeked_token_type;
char cur_char = 0;
int tmp_label_count = 0;
int return_label;

char** function;
int function_count;
char** local;
int local_count;

char** param;
int param_count;

void check(int state, char* msg) {
  if (!state) {
    printf(msg);
    exit(1);
  }
}

void register_function(char* s) {
  check(function_count < MAX_FUNCTION_COUNT, "too many functions\n");
  function[function_count] = s;
  function_count = function_count + 1;
}

int is_function(char* s) {
  int i = 0;
  while (i < function_count) {
    if (strcmp(function[i], s) == 0) {
      return 1;
    }
    i = i + 1;
  }
  return 0;
}
int register_local(char* s) {
  check(local_count < MAX_LOCAL_COUNT, "too many local vars\n");
  local[local_count] = s;
  local_count = local_count + 1;
  return local_count - 1;
}

int register_param(char* s) {
  check(local_count < MAX_LOCAL_COUNT, "too many local vars\n");
  param[param_count] = s;
  param_count = param_count + 1;
  return param_count - 1;
}

void clear_local_param() {
  int i = 0;
  while (i < local_count) {
    free(local[i]);
    i = i + 1;
  }
  local_count = 0;
  i = 0;
  while (i < param_count) {
    free(param[i]);
    i = i + 1;
  }
  param_count = 0;
}

int lookup_local(char* s) {
  int i = 0;
  while (i < local_count) {
    if (strcmp(s, local[i]) == 0) {
      return i;
    }
    i = i + 1;
  }
  return 0 - 1;
}

int lookup_param(char* s) {
  int i = 0;
  while (i < param_count) {
    if (strcmp(s, param[i]) == 0) {
      return i;
    }
    i = i + 1;
  }
  return 0 - 1;
}

int new_temp_label() {
  int res = tmp_label_count;
  tmp_label_count = tmp_label_count + 1;
  return res;
}

char peek_char() {
  if (cur_char == 0) {
    cur_char = getchar();
  }
  return cur_char;
}

void append_char(char c) {
  (token + token_len)[0] = c;
  token_len = token_len + 1;
  check(token_len <= MAX_TOKEN_LEN, "max token len exceeded\n");
}

void eat_char() {
  append_char(peek_char());
  cur_char = 0;  
}

void ignore_char() {
  peek_char();
  cur_char = 0; 
}

void ignore_line() {
  while (peek_char() != '\n') {
    ignore_char();
  }
  ignore_char();
}

void ignore_includes() {
  while (peek_char() == '#') {
    ignore_line();
  }
}

void check_and_ignore_char(char c) {
  check(peek_char() == c, "check_and_ignore_char");
  cur_char = 0;
}

int is_space(char c) {
  return c == ' ' || (c == '\t' || c == '\n');
}

int is_digit(char c) {
  return c >= '0' && c <= '9';
}

int is_letter(char c) {
  return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

void ignore_spaces() {
  while (is_space(peek_char())) { 
    ignore_char();
  } 
}

void eat_quoted_char() {
}

int check_and_eat_char(char c) {
  check(peek_char() == c, "check_and_eat_char");
  eat_char();
}

char* read_token() {
  if (peeked_token) {
    char* res = peeked_token;
    peeked_token = 0;
    return res;
  }
  token_len = 0;
  ignore_spaces();
  ignore_includes();
  ignore_spaces();
  if (is_letter(peek_char())) {
    while (is_letter(peek_char()) 
        || is_digit(peek_char()) 
        || peek_char() == '_') {
      eat_char();
    }
    token_type = symbol_token;
  } else if (is_digit(peek_char())) {
    while (is_digit(peek_char())) {
      eat_char();
    }
    token_type = int_token;
  } else if (peek_char() == '\'') {
    eat_char();
    if (peek_char() == '\\') {
      eat_char();
    }
    eat_char(); 
    check_and_eat_char('\'');
    token_type = char_token;
  } else if (peek_char() == '"') {
    eat_char();
    while (peek_char() != '"') {
      if (peek_char() == '\\') {
        eat_char();
      }
      eat_char(); 
    }
    check_and_eat_char('"');
    token_type = string_token;
  } else if(peek_char() == '|') {
    eat_char();
    check_and_eat_char('|');
    token_type = operator_token;
  } else if(peek_char() == '&') {
    eat_char();
    check_and_eat_char('&');
    token_type = operator_token;
  } else if (peek_char() == '=' 
     || peek_char() == '!'
     || peek_char() == '<' 
     || peek_char() == '>') {
    eat_char();
    if (peek_char() == '=') {
      eat_char();
    }
    token_type = operator_token;
  } else if (strchr("+-*,", peek_char())) {
    eat_char();
    token_type = operator_token;
  } else if (strchr("[](){};", peek_char())) {
    eat_char();
    token_type = other_token;
  } else {
    check(0, "unknown token");
  }
  append_char(0);
  return token;
}

void load_peeked_token() {
  if (!peeked_token) {
    read_token();
    peeked_token = token;
    peeked_token_type = token_type;
  }
}

char* peek_token() {
  load_peeked_token();
  return peeked_token;
}

int peek_token_type() {
  load_peeked_token();
  return peeked_token_type;
}

int matche_token(char* s) {
  if (strcmp(peek_token(), s) == 0
      && (peek_token_type() != string_token
          && peek_token_type() != char_token)) {
    read_token();
    return 1;
  }
  return 0;
}

int end_of_file() {
  ignore_spaces();
  return peek_char() == EOF;
}

void check_and_ignore_token(char* s) {
  check(strcmp(s, read_token()) == 0, "check_and_ignore_token failed");
}

void ignore_type() {
  read_token();
  while (matche_token("*")) { }
}

void process_expr();

void process_object() {
  char* op;
  if (peek_token_type() == symbol_token) {
    char* symbol = strdup(read_token());
    int local_id = lookup_local(symbol);
    int param_id = lookup_param(symbol);
    if (!strcmp(peek_token(), "=") || is_function(symbol)) {
      op = "lea";
    } else {
      op = "mov";
    }
    if (local_id >= 0) {
      printf("%s eax, [ebp-%d]\n", op, (1 + local_id) * WORD_SIZE);
    } else if (param_id >= 0) {
      printf("%s eax, [ebp+%d]\n", op, (2 + param_id) * WORD_SIZE);
    } else {
      printf("%s eax, [%s]\n", op, symbol);
    }
  } else if (matche_token("(")) {
    process_expr();
    check_and_ignore_token(")");
  } else {
    check(0, "failed to parse object\n");
  }
  while (strcmp(peek_token(), "[") == 0 
      || strcmp(peek_token(), "(") == 0 ) {
    printf("push eax\n");
    if (matche_token("[")) {
      process_expr();
      check_and_ignore_token("]");
      printf("pop ebx\n");
      if (strcmp(peek_token(), "=") == 0) {
       op = "lea";
      } else {
        op = "mov";
      }
      printf("%s eax, dword ptr [eax * %d + ebx]\n", op, WORD_SIZE);
    } else {
      check_and_ignore_token("(");
      int start_label = new_temp_label();
      int call_label = new_temp_label();
      int prev_label = call_label;
      printf("jmp _%d\n", start_label);
      int count = 0;
      while (!matche_token(")")) {
        int new_prev_label = new_temp_label();
        printf("_%d:\n", new_prev_label);
        process_expr();
        printf("push eax\n");
        printf("jmp _%d\n", prev_label);
        prev_label = new_prev_label;
        matche_token(",");
        count = count + 1;
      }
      printf("_%d:\n", start_label);
      printf("jmp _%d\n", prev_label);
      printf("_%d:\n", call_label);
      printf("call dword ptr [esp + %d]\n", count * WORD_SIZE);
      printf("add esp, %d\n", (1 + count) * WORD_SIZE);
    }
  }
}

void process_expr0() {
  int type = peek_token_type();
  if (type == int_token) {
    printf("mov eax, %s\n", read_token());
  } else if (type == string_token) {
    int string_label = new_temp_label();
    printf(".section .rodata\n");
    printf("_%d:\n", string_label);
    printf(".ascii %s\n", read_token());
    printf(".byte 0\n");
    printf(".section .text\n");
    printf("mov eax, offset _%d\n", string_label);
  } else if (type == char_token) {
    printf("mov eax, %s\n", read_token());
  } else if (matche_token("-")) {
    check(peek_token_type() == int_token, "- followed by non-integer\n");
    printf("mov eax, -%s\n", read_token());
  } else {
    process_object();
  }
}

void process_expr1() {
  process_expr0();
  if (matche_token("*")) {
    printf("push eax\n");
    process_expr0();
    printf("pop ebx\n");
    printf("imul eax, ebx\n");
  }
}

void process_expr2() {
  process_expr1();
  if (matche_token("+")) {
    printf("push eax\n");
    process_expr1();
    printf("pop ebx\n");
    printf("add eax, ebx\n");
  } else if (matche_token("-")) {
    printf("push eax\n");
    process_expr1();
    printf("pop ebx\n");
    printf("sub ebx, eax\n");
    printf("mov eax, ebx\n");
  }
} 

void process_expr3() {
  if (matche_token("!")) {
    process_expr3();
    printf("cmp eax, 0\n");
    printf("mov eax, 0\n");
    printf("sete al\n");
  } else {
    process_expr2();
    char* inst;
    if (matche_token("==")) {
      inst = "sete";
    } else if (matche_token("!=")) {
      inst = "setne";
    } else if (matche_token("<")) {
      inst = "setl";
    } else if (matche_token("<=")) {
      inst = "setle";
    } else if (matche_token(">")) {
      inst = "setg";
    } else if (matche_token(">=")) {
      inst = "setge";
    } else {
      return;
    }
    printf("push eax\n");
    process_expr2();
    printf("pop ebx\n");
    printf("cmp ebx, eax\n");
    printf("mov eax, 0\n");
    printf("%s al\n", inst);
  }
}

void process_expr4() {
  process_expr3();
  if (matche_token("&&")) {
    int end_label = new_temp_label();
    printf("cmp eax, 0\n");
    printf("jz _%d\n", end_label);
    process_expr4();
    printf("_%d:\n", end_label);
  }
}

void process_expr5() {
  process_expr4();
  if (matche_token("||")) {
    int end_label = new_temp_label();
    printf("cmp eax, 0\n");
    printf("jnz _%d\n", end_label);
    process_expr5();
    printf("_%d:\n", end_label);
  }
}

void process_expr6() {
  process_expr5();
  if (matche_token("=")) {
    printf("push eax\n");
    process_expr();
    printf("pop ebx\n");
    printf("mov dword ptr [ebx], eax\n");
  }
}

void process_expr() {
  process_expr6();
}

void process_block();

int is_type(char* s) {
  return strcmp(s, "int") == 0 
      || strcmp(s, "char") == 0
      || strcmp(s, "void") == 0;
}

void process_stmt() {
  if (matche_token("if")) {
    int else_label = new_temp_label();
    int endif_label = new_temp_label();
    check_and_ignore_token("(");
    process_expr();
    check_and_ignore_token(")");

    printf("cmp eax, 0\n");
    printf("je _%d\n", else_label);

    process_block();

    printf("jmp _%d\n", endif_label);
    printf("_%d:\n", else_label);

    if (matche_token("else")) {
      if (strcmp(peek_token(), "if") == 0) {
        process_stmt();
      } else {
        process_block();
      }
    }

    printf("_%d:\n", endif_label);
  } else if (matche_token("while")) {
    int while_label = new_temp_label();
    int endwhile_label = new_temp_label();
    printf("_%d:\n", while_label);
    check_and_ignore_token("(");
    process_expr();
    check_and_ignore_token(")");

    printf("cmp eax, 0\n");
    printf("je _%d\n", endwhile_label);

    process_block();

    printf("jmp _%d\n", while_label);
    printf("_%d:\n", endwhile_label);
  } else if (matche_token("return")) {
    if (!matche_token(";")) {
      process_expr();
      check_and_ignore_token(";");
    }
    printf("jmp _%d\n", return_label);
  } else {
    if (is_type(peek_token())) {
      ignore_type();
      char* symbol = strdup(read_token());
      int idx = register_local(symbol);
      if (matche_token("=")) {
        process_expr();
        printf("mov dword ptr [ebp-%d], eax\n", (1 + idx) * WORD_SIZE);
      }
    } else {
      process_expr();
    }
    check_and_ignore_token(";");
  }
}

void process_block() {
  check_and_ignore_token("{");
  while (!matche_token("}")) {
    process_stmt();   
  }
}

void process_decl() {
  ignore_type();
  char* name = strdup(read_token());
  if (matche_token("=")) {
    printf(".section .data\n");
    check(peek_token_type() == int_token,
        "only integer variable initialization is allowed\n");
    printf("%s: .long %s\n", name, read_token());
    printf(".section .text\n");
    check_and_ignore_token(";");
  } else if (matche_token("(")) {
    register_function(name);
    clear_local_param();
    while (!matche_token(")")) {
      ignore_type();
      char* param_name = strdup(read_token());
      register_param(param_name);
      matche_token(",");
    }
    if (strcmp(peek_token(), "{") == 0) {
      int start_label = new_temp_label();
      printf("_%d:\n",start_label);

      return_label = new_temp_label();
      process_block();

      printf("_%d:\n", return_label);
      printf("mov esp, ebp\n");
      printf("pop ebp\n");
      printf("ret\n");

      printf(".globl %s\n", name);
      printf("%s:\n", name);
      printf("push ebp\n");
      printf("mov ebp, esp\n");
      printf("sub esp, %d\n", local_count * WORD_SIZE);
      printf("jmp _%d\n", start_label);

    } else {
      check_and_ignore_token(";");
    }
  } else if (matche_token(";")) {
    printf(".section .data\n");
    printf("%s: .long 0\n", name);
    printf(".section .text\n");
  } else {
    check(0, "illegal declaration syntax");
  }
}

void process_prog() {
  printf(".intel_syntax noprefix\n");
  while (!end_of_file()) {
    process_decl();
  }
}

void init() {
  EOF = 0 - 1;
  token = malloc(MAX_TOKEN_LEN + 1);  
  token_len = 0;
  local = malloc(MAX_LOCAL_COUNT * WORD_SIZE);
  local_count = 0;
  param = malloc(MAX_LOCAL_COUNT * WORD_SIZE);
  param_count = 0;
  function = malloc(MAX_FUNCTION_COUNT * WORD_SIZE);
  function_count = 0;
}

int main() {
  init();
  process_prog();
  return 0;
}
