#include "base.h"
#include "list.h"
#include "string_map.h"

struct StringMap* new_string_map_with_bucket_num(int n) {
  struct StringMap* res = malloc(sizeof(struct StringMap));
  res->size = 0;
  res->buckets = new_list();
  while (n--) {
    list_add(res->buckets, new_list());
  }
  return res;
}

struct StringMap* new_string_map() {
  return new_string_map_with_bucket_num(7);
}

int _string_hash(char* s) {
  int h = 0;
  int i = 0;
  while (1) {
    char c = s[i] % 256; // TODO: remove hack when size directiva is available
    c = c < 0 ? c + 256 : c;
    if (!c) {
      break;
    }
    h *= 131;
    h += c;
    s++;
  }
  return h;
}

struct Entry* _new_entry(char* k, void* v) {
  struct Entry* res = malloc(sizeof(struct Entry));
  res->k = k;
  res->v = v;
  return res;
}

struct List* _string_map_get_bucket(struct StringMap* map, char* s) {
  int i = abs(_string_hash(s)) % list_size(map->buckets);
  return list_get(map->buckets, i);
}

struct Entry* _string_map_get_entry(struct StringMap* map, char* s) {
  struct List* bucket = _string_map_get_bucket(map, s);
  for (int j = 0; j < list_size(bucket); j++) {
    struct Entry* e = list_get(bucket, j);
    if (!strcmp(e->k, s)) {
      return e;
    }
  }
  return 0;
}

int string_map_contains(struct StringMap* map, char* s) {
  return _string_map_get_entry(map, s) != 0;
}

void _string_map_add_entry(struct StringMap* map, struct Entry* e) {
  struct List* bucket = _string_map_get_bucket(map, e->k);
  list_add(bucket, e);
  map->size++;
}

void _rehash(struct StringMap* map) {
  int new_bucket_num = (list_size(map->buckets) + 1) * 2 - 1;
  struct List* new_buckets = new_list();
  for (int i = 0; i < new_bucket_num; i++) {
    list_add(new_buckets, new_list());
  }
  for (int i = 0; i < list_size(map->buckets); i++) {
    struct List* bucket = list_get(map->buckets, i);
    for (int j = 0; j < list_size(bucket); j++) {
      struct Entry* e = list_get(bucket, j);
      int k = abs(_string_hash(e->k)) % new_bucket_num;
      struct List* new_bucket = list_get(new_buckets, k);
      list_add(new_bucket, e);
    }
    free_list(bucket);
  }
  free_list(map->buckets);
  map->buckets = new_buckets;
}

void string_map_put(struct StringMap* map, char* k, void* v) {
  struct Entry* e = _string_map_get_entry(map, k);
  if (e) {
    e->v = v;
  } else {
    _string_map_add_entry(map, _new_entry(k, v));
    if (map->size > list_size(map->buckets)) {
      _rehash(map);
    }
  }
}

void* string_map_get(struct StringMap* map, char* k) {
  struct Entry* e = _string_map_get_entry(map, k);
  return e ? e->v : 0;
}

void string_map_clear(struct StringMap* map) {
  for (int i = 0; i < list_size(map->buckets); i++) {
    struct List* bucket = list_get(map->buckets, i);
    for (int j = 0; j < list_size(bucket); j++) {
      struct Entry* e = list_get(bucket, j);
      free(e); 
    }
    clear_list(bucket);
  }
  map->size = 0;
}

int string_map_size(struct StringMap* map) {
  return map->size;
}

struct List* string_map_keys(struct StringMap* map) {
  struct List* res = new_list();
  for (int i = 0; i < list_size(map->buckets); i++) {
    struct List* bucket = list_get(map->buckets, i);
    for (int j = 0; j < list_size(bucket); j++) {
      struct Entry* e = list_get(bucket, j);
      list_add(res, e->k);
    }
  }
  return res;
}

struct List* string_map_values(struct StringMap* map) {
  struct List* res = new_list();
  for (int i = 0; i < list_size(map->buckets); i++) {
    struct List* bucket = list_get(map->buckets, i);
    for (int j = 0; j < list_size(bucket); j++) {
      struct Entry* e = list_get(bucket, j);
      list_add(res, e->v);
    }
  }
  return res;
}
