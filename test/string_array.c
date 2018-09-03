void printf();
void* malloc();
char** a;
int main() {
  a = malloc(4 * 10);
  a[0] = "hello";
  a[1] = "world";
  printf("%s %s\n", a[0], a[1]);
  return 0;
}
