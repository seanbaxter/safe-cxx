#feature on safety
#include <cstdio>

int main() safe {
  // It's ill-formed to do pointer dereference in safe contexts.
  // Use borrows, which are checked!
  int* p;
  {
    int x = 100;
    p = &x;
  }

  int value = *p;
  unsafe printf("%d\n", value);
}