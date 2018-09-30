void printf();
enum A {
  V0,
  V1,
  V3=3,
  V4,
  V8=8,
  V9,
};

void f(enum A v) {
  printf("%d\n", v);
}

int main() {
  printf("%d %d %d %d %d\n", V0, V1, V3, V4, V8);
  f(V9);
  return 0;
}
