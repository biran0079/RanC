void printf();
void* malloc();
int main() {
  char* s = malloc(10);
  s[0] = 'h';
  (s + 2)[0] = 'l';
  s[1] = 'e';
  (s + 4)[0] = 'o';
  s[3] = 'l';
  (s + 2)[3] = 0;
  printf("%s\n", s);
  return 0;
}
