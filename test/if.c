void printf();
int main() {
  int a;
  a = 1;
  if (a > 1) {
    printf("should not reach here\n");
  } else {
    printf("in else\n");
  }
  if (a <= 1) {
    printf("in if\n");
  }
  if (a != 1) {
    printf("should not reach here\n");
  }
  printf("ok\n");
  return 0;
}
