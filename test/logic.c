void printf();
int f(int n) {
  printf("%d\n", n);
  return n > 10;
}
int main() {
  printf("%d\n", f(3) || f(15));
  printf("%d\n", f(2) || f(4));
  printf("%d\n", f(14) || f(6));
  printf("%d\n", f(15) || f(17));

  printf("%d\n", f(3) && f(15));
  printf("%d\n", f(2) && f(4));
  printf("%d\n", f(14) && f(6));
  printf("%d\n", f(15) && f(17));

  printf("%d\n", 0 || 0 || 1);
  printf("%d\n", !0);
  printf("%d\n", !1);
  return 0;
}
