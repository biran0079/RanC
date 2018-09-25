void printf();
enum A {
  V0,
  V1,
  V3=3,
  V4,
  V8=8
};
int main() {
  printf("%d %d %d %d %d\n", V0, V1, V3, V4, V8);
  return 0;
}
