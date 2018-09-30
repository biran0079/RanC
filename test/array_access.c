void printf();
void malloc();
char* g() {
  return "biran";
}
int* f() {
  int* a = malloc(4 * 2);
  a[0] = 1;
  a[1] = 2;
  return a;
}
int main() {
  printf("%c\n", "hello world\n"[1]);
  char* s = "test";
  printf("%c %c\n", (s+1)[0], (s+2)[1]);
  printf("%c %c %c %c\n", s[1], s[2], s[0], s[3]); 
  s+=2;
  printf("%c %c\n", (s+1)[0], (s+2)[1]);
  int* a = malloc(4 * 3);
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  printf("%d %d %d\n", a[0], a[1], a[2]);
  printf("%d %d\n", (a+1)[0], (a+2)[0]);
  printf("%d %d\n", f()[0], f()[1]);
  printf("%c %c\n", g()[1], g()[3]);
  return 0;
}
