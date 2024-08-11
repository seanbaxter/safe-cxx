#feature on safety
#include "std2.h"

int main() safe {
  std2::vector<int> vec { 10, 20, 30, 40 };

  // Uses Rust-style std2::iterator customization point.
  for(int^ x : ^vec)
    *x *= 3;

  for(int x : vec)
    unsafe printf("%d\n", x);
}