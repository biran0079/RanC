void exit();
int printf();

void check(int state, char* msg) {
  if (!state) {
    printf(msg);
    exit(1);
  }
}
