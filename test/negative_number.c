void printf();
int main() {
  printf("%d\n", -1);
  int a = 2;
  printf("%d\n", -a);
  int* b = malloc(4);
  b[0] = 3;
  printf("%d\n", -b[0]);
  return 0;
}
