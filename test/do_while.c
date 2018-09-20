void printf();
int main() {
  int i = 0;
  do {
    printf("%d\n", i);
  } while (0);
  do {
    printf("%d\n", i++);
    if (i == 5) {
      continue;
    }
  } while (i < 5);
  return 0;
}
