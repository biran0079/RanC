void exit();
int printf();

int WORD_SIZE = 4;

void check(int state, char* msg) {
  if (!state) {
    printf(msg);
    exit(1);
  }
}
