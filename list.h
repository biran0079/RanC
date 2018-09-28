#ifndef __list_h__
#define __list_h__

struct List {
  void** arr;
  int size;
  int cap;
};

struct List* new_list();

int list_size(struct List* lst); 

void* list_get(struct List* lst, int i);

void* list_add(struct List* lst, void* v);

void free_list(struct List* lst);

#endif
