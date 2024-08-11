#feature on safety
#include <cstdio>
#include "std2.h"

int main() safe {
  int array[10] { };

  // Out of bounds index.
  size_t index = 17;
  
  // Panic on out-of-bounds subscript.
  int x = array[index];
  unsafe printf("%d\n", x);
}