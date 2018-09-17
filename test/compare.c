void printf();
int main() {
  printf("%d\n", 1 == 1);
  printf("%d\n", 2 == 1);

  printf("%d\n", 1 != 1);
  printf("%d\n", 2 != 1);

  printf("%d\n", 1 > 1);
  printf("%d\n", 2 > 1);
  printf("%d\n", 0 > 1);

  printf("%d\n", 1 >= 1);
  printf("%d\n", 2 >= 1);
  printf("%d\n", 0 >= 1);

  printf("%d\n", 1 < 1);
  printf("%d\n", 2 < 1);
  printf("%d\n", 0 < 1);

  printf("%d\n", 1 <= 1);
  printf("%d\n", 2 <= 1);
  printf("%d\n", 0 <= 1);
  return 0;
}
