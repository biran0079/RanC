void printf();
int fib(int n) {
  if (n <= 1) {
    return n;
  }
  return fib(n-1) + fib(n-2);
}
int main() {
  int i = 0;
  while (i < 8) {
    printf("%d\n", fib(i));
    i = i + 1;
  }
  return 0;
}
