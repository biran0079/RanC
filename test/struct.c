void printf();
void* malloc();
struct A {
  int a;
  char b;
};
struct B {
  struct A* a;
  int c;
};
int main() {
  struct B* t = malloc(sizeof(struct B));
  t->a = malloc(sizeof(struct A));
  printf("%d\n", sizeof(t->a->b));
  printf("%d\n", sizeof((*t).c));
  t->c = 1;
  t->a->a = 2;
  (*t->a).b = 'x';
  printf("%d %d %c\n", (*t).c++, (*t->a).a++, t->a->b++);
  printf("%d %d %c\n", (*t).c, (*t->a).a, t->a->b);
  return 0;
}
