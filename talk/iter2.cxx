#feature on safety
#include "std2.h"

int main() safe {
  std2::vector<int> vec { 11, 15, 20 };

  for(int x : vec) {
    // Ill-formed. mutate of vec invalidates iterator in ranged-for.
    if(x % 2)
      vec^.push_back(x);

    unsafe printf("%d\n", x);
  }
}