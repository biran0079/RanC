#include "tokenizer.h"
#include "string_buffer.h"

struct TokenizerContex {
  int cur_char;
  enum TokenType cur_token_type;
  struct StringBuffer* token_buffer;
  struct StringBuffer* line_buffer;

  int line_number;
  int col_number;
  struct TokenizerOutput* result;
};

char** token_type_str;

void init_tokenizer() {
  token_type_str = malloc(token_type_num * WORD_SIZE);
  token_type_str[int_token] = "int";
  token_type_str[string_token] = "string";
  token_type_str[char_token] = "char";
  token_type_str[symbol_token] = "symbol";
  token_type_str[operator_token] = "op";
  token_type_str[comment_token] = "comment";
  token_type_str[other_token] = "other";
  token_type_str[eof_token] = "eof";
}

int peek_char(struct TokenizerContex* ctx) {
  if (ctx->cur_char == 0) {
    ctx->cur_char = getchar();
  }
  return ctx->cur_char;
}

void append_char(struct TokenizerContex* ctx, char c) {
  string_buffer_append(ctx->token_buffer, c);
}

void consume_current_char(struct TokenizerContex* ctx) {
  check(ctx->cur_char, "peek_char() should be called before consume_current_char()");
  if (ctx->cur_char == '\n' || ctx->cur_char == EOF) {
    list_add(ctx->result->code, string_buffer_to_string_and_clear(ctx->line_buffer));
    ctx->line_number++;
    ctx->col_number = 1;
  } else {
    string_buffer_append(ctx->line_buffer, ctx->cur_char);
    ctx->col_number++;
  }
  ctx->cur_char = 0;  
}

void eat_char(struct TokenizerContex* ctx) {
  append_char(ctx, peek_char(ctx));
  consume_current_char(ctx);
}

void ignore_char(struct TokenizerContex* ctx) {
  peek_char(ctx);
  consume_current_char(ctx);
}

void ignore_line(struct TokenizerContex* ctx) {
  while (peek_char(ctx) != '\n') {
    ignore_char(ctx);
  }
  ignore_char(ctx);
}

int is_space(char c) {
  return c == ' ' || (c == '\t' || c == '\n');
}

int is_digit(char c) {
  return c >= '0' && c <= '9';
}

int is_letter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void print_recent_code(struct TokenizerContex* ctx, int n) {
  int st = max(0, list_size(ctx->result->code) - n);
  for (int i = st; i < list_size(ctx->result->code); i++) {
    char* line = list_get(ctx->result->code, i);
    printf("%-6d%s\n", i + 1, line);
  }
  printf("%-6d%s%c\n", ctx->line_number,
      string_buffer_to_string_and_clear(ctx->line_buffer), peek_char(ctx));
}

void check_and_eat_char(struct TokenizerContex* ctx, char c) {
  if (peek_char(ctx) != c) {
    printf("Tokenizer failed: expect '%c', got '%c'\n", c, peek_char(ctx));
    print_recent_code(ctx, 3);
    exit(-1);
  }
  eat_char(ctx);
}

void read_single_token(struct TokenizerContex* ctx) {
  while (1) {
    if (peek_char(ctx) == '#') {
      // ignore text after #
      ignore_line(ctx);
    } else if (is_space(peek_char(ctx))) {
      // ignore spaces
      ignore_char(ctx);
    } else {
      break;
    }
  }
  int start_col = ctx->col_number;
  if (is_letter(peek_char(ctx)) || peek_char(ctx) == '_') {
    while (is_letter(peek_char(ctx)) 
        || is_digit(peek_char(ctx)) 
        || peek_char(ctx) == '_') {
      eat_char(ctx);
    }
    ctx->cur_token_type = symbol_token;
  } else if (is_digit(peek_char(ctx))) {
    while (is_digit(peek_char(ctx))) {
      eat_char(ctx);
    }
    ctx->cur_token_type = int_token;
  } else {
    switch (peek_char(ctx)) {
      case '\'': {
        eat_char(ctx);
        if (peek_char(ctx) == '\\') {
          eat_char(ctx);
        }
        eat_char(ctx); 
        check_and_eat_char(ctx, '\'');
        ctx->cur_token_type = char_token;
        break;
      } 
      case '"': {
        eat_char(ctx);
        while (peek_char(ctx) != '"') {
          if (peek_char(ctx) == '\\') {
            eat_char(ctx);
          }
          eat_char(ctx); 
        }
        check_and_eat_char(ctx, '"');
        ctx->cur_token_type = string_token;
        break;
      } 
      case '|': {
        eat_char(ctx);
        check_and_eat_char(ctx, '|');
        ctx->cur_token_type = operator_token;
        break;
      } 
      case '&': {
        eat_char(ctx);
        if (peek_char(ctx) == '&') {
          eat_char(ctx);
        }
        ctx->cur_token_type = operator_token;
        break;
      } 
      case '=':
      case '!':
      case '<':
      case '>': {
        eat_char(ctx);
        if (peek_char(ctx) == '=') {
          eat_char(ctx);
        }
        ctx->cur_token_type = operator_token;
        break;
      } 
      case '/': {
        eat_char(ctx);
        if (peek_char(ctx) == '/') {
          eat_char(ctx);
          ignore_line(ctx);
          ctx->cur_token_type = comment_token;
        } else if (peek_char(ctx) == '*') {
          eat_char(ctx);
          int state = 0;
          while (state != 2) {
            if (state == 0) {
              if (peek_char(ctx) == '*') {
                state = 1;
              } 
            } else {
              if (peek_char(ctx) == '/') {
                state = 2;
              } else if (peek_char(ctx) != '*') {
                state = 0;
              }
            }
            ignore_char(ctx);
          }
          ctx->cur_token_type = comment_token;
        } else if (peek_char(ctx) == '=') {
          eat_char(ctx);
          ctx->cur_token_type = operator_token;
        } else {
          ctx->cur_token_type = operator_token;
        }
        break;
      }
      case '+': case '-': case '*': case '%': 
      case ',': case '?': case ':': case '.': {
        char c = peek_char(ctx);
        eat_char(ctx);
        if ((c == '+' || c == '-') && peek_char(ctx) == c) {
          eat_char(ctx);
        } else if ((c == '+' || c == '-' || c == '*') && peek_char(ctx) == '=') {
          eat_char(ctx);
        } else if (c == '-' && peek_char(ctx) == '>') {
          eat_char(ctx);
        }
        ctx->cur_token_type = operator_token;
        break;
      } 
      case '[': case ']': case '(': case ')':
      case '{': case '}': case ';': {
        eat_char(ctx);
        ctx->cur_token_type = other_token;
        break;
      }
      case EOF:
        ctx->cur_token_type = eof_token; 
        break;
      default:
        printf("Tokenizer failed: unknown token\n");
        print_recent_code(ctx, 3);
        exit(-1);
    }
  }
  peek_char(ctx); // make sure cursor is pointing at the first char after current token
  char* s = string_buffer_to_string_and_clear(ctx->token_buffer);
  struct Token* token = new_token(ctx->cur_token_type, s, ctx->line_number, start_col, ctx->col_number);
  if (ctx->cur_token_type != comment_token) {
    list_add(ctx->result->tokens, token);
  }
}

struct Token* new_token(enum TokenType type, char* s, int line, int start_col, int end_col) {
  struct Token* res = malloc(sizeof(struct Token));
  res->type = type;
  res->s = s;
  res->line = line;
  res->start_col = start_col;
  res->end_col = end_col;
  return res;
}

struct TokenizerOutput* new_tokenizer_output() {
  struct TokenizerOutput* res = malloc(sizeof(struct TokenizerOutput));
  res->tokens = new_list();
  res->code = new_list();
  return res;
}

struct TokenizerContex* new_tokenizer_context() {
  struct TokenizerContex* res = malloc(sizeof(struct TokenizerContex));
  res->cur_char = 0;
  res->line_number = 1;
  res->col_number = 1;
  res->token_buffer = new_string_buffer();
  res->line_buffer = new_string_buffer();
  res->result = new_tokenizer_output();
  return res;
}

struct TokenizerOutput* tokenize() {
  struct TokenizerContex* ctx = new_tokenizer_context();
  do {
    read_single_token(ctx);
  } while (ctx->cur_token_type != eof_token);
  return ctx->result;
}
