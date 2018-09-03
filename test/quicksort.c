int printf();
void* malloc();
void swap(int* a, int i, int j) {
  int t = a[i];
  a[i] = a[j];
  a[j] = t;
}
int partition(int* a, int i, int j) {
  int p = i;
  i = i + 1;
  while (i <= j) {
    if (a[i] > a[p]) {
      swap(a, i, j);
      j = j - 1;
    } else {
      i = i + 1;  
    }
  }
  swap(a, p, j);
  return j;
}
void sort(int* a, int i, int j) {
  if (i >= j) {
    return;
  }
  int k = partition(a, i, j);
  sort(a, i, k-1);
  sort(a, k+1, j);
}
int main() {
  int n = 20;
  int* a = malloc(n * 4);
  int i = 0; 
  while (i < 20) {
    a[i] = 20 - i;
    i = i + 1;
  }
  sort(a, 0, n - 1);
  i = 0;
  while (i < 20) {
    printf("%d\n", a[i]);
    i = i + 1;
  }
  return 0;
}
