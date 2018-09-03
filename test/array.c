void printf();
void* malloc(int n);
int sum(int* a, int n) {
  int res = 0;
  int i = 0;
  while (i < n) {
    res = res + a[i];
    i = i + 1;
  }
  return res;
}
int main() {
  int n = 10;
  int* a = malloc(n * 4);
  int i = 0;
  while (i < n) {
    a[i] = i;
    i = i + 1;
  }
  printf("%d\n", sum(a, n));
  return 0;
}
