#ifndef __string_map_h__
#define __string_map_h__

#include "list.h"

struct StringMap {
  struct List* buckets;
  int size;
};

struct Entry {
  char* k;
  void* v;
};

struct StringMap* new_string_map_with_bucket_num(int n);

struct StringMap* new_string_map();

int string_map_contains(struct StringMap* map, char* s);

void string_map_put(struct StringMap* map, char* k, void* v);

void* string_map_get(struct StringMap* map, char* k);

#endif
