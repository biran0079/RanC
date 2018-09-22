void printf();
int max(int a, int b) {
  return a > b ? a : b;
}
int max3(int a, int b, int c) {
  return a > b ? a > c ? a : c : b > c ? b : c;
}
int main() {
  printf("%d\n", max(1,2));
  printf("%d\n", max(2,1));
  printf("%d\n", max3(1,2,3));
  printf("%d\n", max3(1,3,2));
  printf("%d\n", max3(3,1,2));
  return 0;
}
