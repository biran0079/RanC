void printf();
int main() {
  int i;
  for (i = 0; i < 5; i++) {
    printf("%d\n", i);
  }
  for (int j = 0; j < 5; j++) {
    printf("%d\n", j);
  }
  for (; i < 10; i++) {
    printf("%d\n", i);
  }
  for (i = 10;; i++) {
    printf("%d\n", i);
    if (i == 15) {
      break;
    }
  }
  for (i = 15; i < 20;) {
    printf("%d\n", i++);
  }
  for (i = 20; i < 25;) {
    printf("%d\n", i++);
  }
  for (;;) {
    printf("%d\n", i++);
    if (i == 30) {
      break;
    }
  }
  return 0;
}
