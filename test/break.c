void printf();
int main() {
  int i = 0;
  while (i < 10) {
    printf("%d\n", i);
    i = i + 1;
    if (i > 5) {
      break;
    }
  }
  i = 0;
  while (i < 10) {
    int j = 0;
    while (j < 10) {
      printf("%d %d\n", i,  j);
      if (j >= i) {
        break;
      }
      j = j + 1;
    }
    i = i + 1;
  }
  return 0;
}
