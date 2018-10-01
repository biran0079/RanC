#include "base.h"

void check(int state, char* msg) {
  if (!state) {
    printf(msg);
    exit(1);
  }
}

int max(int a, int b) {
  return a > b ? a : b;
}

int min(int a, int b) {
  return a < b ? a : b;
}

int abs(int a) {
  return a > 0 ? a : -a;
}
