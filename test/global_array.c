void printf();
void* malloc(int n);
int* a;
int main() {
  int n = 10;
  a = malloc(n * 4);
  int i = 0;
  while (i < n) {
    a[i] = i;
    i = i + 1;
  }
  int res = 0;
  i = 0;
  while (i < n) {
    res = res + a[i];
    i = i + 1;
  }
  printf("%d\n", res);
  return 0;
}
