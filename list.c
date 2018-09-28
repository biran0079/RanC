#include "list.h"
#include "base.h"

struct List* new_list() {
  struct List* lst = malloc(sizeof(struct List));
  lst->cap = 4;
  lst->arr = malloc(lst->cap * sizeof(void*));
  lst->size = 0;
  return lst;
}

int list_size(struct List* lst) {
  return lst->size;
}

void* list_get(struct List* lst, int i) {
  check(i < lst->size, "index out of bound");
  return lst->arr[i];
}

void* list_add(struct List* lst, void* v) {
  if (lst->cap == lst->size) {
    lst->cap *= 2;
    lst->arr = realloc(lst->arr, lst->cap * WORD_SIZE);
  }
  lst->arr[lst->size++] = v;
}

void free_list(struct List* lst) {
  free(lst->arr);
  free(lst);
}
