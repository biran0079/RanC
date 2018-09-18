void printf();
void* malloc();
int main() {
  int a = 0;
  printf("%d\n", a++);
  printf("%d\n", a++);
  printf("%d\n", ++a);
  printf("%d\n", ++a);
  printf("%d\n", a--);
  printf("%d\n", a--);
  printf("%d\n", --a);
  printf("%d\n", --a);
  int* b = malloc(4);
  b[0] = 1;
  printf("%d\n", b[0]++);
  printf("%d\n", b[0]++);
  printf("%d\n", ++b[0]);
  printf("%d\n", ++b[0]);
  printf("%d\n", b[0]--);
  printf("%d\n", b[0]--);
  printf("%d\n", --b[0]);
  printf("%d\n", --b[0]);
  return 0;
}
