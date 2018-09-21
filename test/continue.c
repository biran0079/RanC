void printf();
int main() {
  int i;
  i = 0;
  while (i < 10) {
    i = i + 1;
    if (i % 2 == 0) {
      continue;
    }
    printf("%d\n", i);
  }
  i = 0;
  while (i < 10) {
    i = i + 1;
    if (i % 2 == 0) {
      continue;
    }
    int j = 0;
    while (j < 10) {
      j = j + 1;
      if (j % 3 == 0) {
        continue;
      }
      printf("%d %d\n", i, j);
    }
  }
  i = 0;
  do {
    if (i++ % 2 == 0) {
      continue;
    }
    printf("%d\n", i);
  } while (i < 10);
  for (int j = 0; j < 10; j++) {
    if (j % 2 == 1) {
      printf("%d\n", j);
    }
  }
  return 0;
}
