void printf();
void* malloc();
int main() {
  char* s = malloc(10);
  s[0] = 'h';
  (s + 1)[0] = 'e';
  (s + 2)[0] = 'l';
  (s + 3)[0] = 'l';
  (s + 4)[0] = 'o';
  (s + 5)[0] = 0;
  printf("%s\n", s);
  return 0;
}
