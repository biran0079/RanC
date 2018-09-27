void printf();
int f() {
  printf("hello");
  return 123;
}
int main() {
  char* s ="hello world\n";
  printf("%d\n", sizeof s);
  printf("%d\n", sizeof 123);
  char c = 'c';
  printf("%d\n", sizeof c);
  int a;
  printf("%d\n", sizeof a);
  void* p;
  printf("%d\n", sizeof p);
  printf("%d\n", sizeof(1+1));
  printf("%d\n", sizeof(0&&1));
  printf("%d\n", sizeof f());
  printf("%d\n", sizeof(int));
  printf("%d\n", sizeof(int**));
  return 0;
}
