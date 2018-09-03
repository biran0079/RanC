void printf();
int add(int a, int b) {
  return a + b;
}
int main() {
  printf("%d %d\n", add(1, 2), add(add(1, 3), 2));
  return 0;
}
