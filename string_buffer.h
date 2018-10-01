#ifndef __STRING_BUFFER_H__
#define __STRING_BUFFER_H__

struct StringBuffer {
  char* s;
  int size;
  int cap;
};

struct StringBuffer* new_string_buffer();

void string_buffer_append(struct StringBuffer* sb, char c);

char* string_buffer_to_string_and_clear(struct StringBuffer* sb);

void string_buffer_clear(struct StringBuffer* sb);

#endif
