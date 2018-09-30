void* malloc();
void printf();
int main() {
  char *s = malloc(10);
  s[0] = 'a';
  s[1] = 'b';
  s[2] = 'c';
  s[3] = 0;
  for (int i = 0; i < 256; i++) {
    s[1]++;
  }
  printf("%s\n", s);
  for (int i = 0; i < 256; i++) {
    s[1]--;
  }
  printf("%s\n", s);
  for (int i = 0; i < 256; i++) {
    ++s[1];
  }
  printf("%s\n", s);
  for (int i = 0; i < 256; i++) {
    --s[1];
  }
  printf("%s\n", s);
  return 0;
}
