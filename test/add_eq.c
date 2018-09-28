int printf();
void* malloc();
int main() {
  int a=100;;
  printf("%d\n", a += 2);
  printf("%d\n", a -= 2);
  printf("%d\n", a *= 2);
  printf("%d\n", a /= 2);
  int* b = malloc(16);
  b[0]=1;
  b[1]=2;
  b[2]=3;
  b[3]=4;
  b += 2;
  printf("%d\n", b[0]);
  b -= 1;
  printf("%d\n", b[0]);
  return 0;
}
