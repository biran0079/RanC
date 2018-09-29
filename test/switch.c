void printf();
enum NUM {
  TWO = 2,
  THREE
};

void g(int n) {
  switch(n) {
    case 1:
      printf("1\n");
      break;
    case TWO:
      printf("2\n");
    case THREE:
    case 4:
      printf("4\n");
      break;
    case 6:
      printf("6\n");
      break;
  }
}

void f(int n) {
  switch(n) {
    case 1:
      printf("1\n");
      break;
    case TWO:
      printf("2\n");
    case THREE:
    case 4:
      printf("4\n");
      break;
    default:
      printf("5\n");
      break;
    case 6:
      printf("6\n");
      break;
  }
}

int main() {
  for (int i = 0; i <= 7; i++) {
    f(i);
  }
  for (int i = 0; i <= 7; i++) {
    g(i);
  }
  return 0;
}
