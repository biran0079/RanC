void printf();
void* malloc();
void swap(int* a, int* b) {
  int t = *a;
  *a = *b;
  *b = t;
}
int main() {
  int a = 123;
  int b = 321;
  swap(&a, &b);
  printf("%d %d\n", a, b);
  int** c = malloc(2 * sizeof(int*));
  c[0] = malloc(2 * sizeof(int));
  c[1] = malloc(2 * sizeof(int));
  **c = 456;
  *c[1] = 789;
  c[0][1] = 1;
  *(*(c + 1) + 1) = 1;
  printf("%d %d %d %d\n", c[0][0], c[0][1], c[1][0], c[1][1]);
  return 0;
}
