#feature on safety
#include <cstdio>
#include <cstdint>

int main() safe {
  // Panic on divide by 0.
  int num = 1;
  int den = 0;
  int x = num / den;

  // Panic on INT_MIN / -1.
  num = INT_MIN;
  den = -1;
  int y = num / den;
}