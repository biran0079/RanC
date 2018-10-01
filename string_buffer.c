#include "base.h"
#include "string_buffer.h"

struct StringBuffer* new_string_buffer() {
  struct StringBuffer* res = malloc(sizeof(struct StringBuffer));
  res->cap = 16;
  res->size = 0;
  res->s = malloc(res->cap);
  return res;
}

void string_buffer_append(struct StringBuffer* sb, char c) {
  if (sb->cap == sb->size) {
    sb->cap *= 2;
    sb->s = realloc(sb->s, sb->cap);
  }
  sb->s[sb->size++] = c;
}

char* string_buffer_to_string_and_clear(struct StringBuffer* sb) {
  char* s = malloc(sb->size + 1);
  for (int i = 0; i < sb->size; i++) {
    s[i] = sb->s[i];
  }
  s[sb->size] = 0;
  string_buffer_clear(sb);
  return s;
}

void string_buffer_clear(struct StringBuffer* sb) {
  sb->size = 0;
}
