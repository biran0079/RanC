int printf();
struct B {
  char* s1;
  char* s2;
};

struct A {
  int* a;
  char b;
  int c;
  struct B d;
};

int main() {
  printf("%d %d\n", sizeof(struct A), sizeof(struct B));
  return 0;
}
