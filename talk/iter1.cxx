#include <vector>
#include <cstdio>

int main() {
  // 90s C++: indices are *safe*.
  // VALUE SEMANTICS ARE GOOD.
  puts("Loop with indices:");
  std::vector<int> vec1 { 11, 15, 20 };
  size_t count = vec1.size();
  for(size_t i = 0; i < count; ++i) {
    int x = vec1[i];
    if(x % 2)
      vec1.push_back(x);
    printf("%d\n", x);
  }

  // Modern C++: ranged-for iterators are *unsafe*.
  // UNCHECKED REFERENCE SEMANTICS ARE BAD.
  puts("\nLoop with iterators:");
  std::vector<int> vec2 { 11, 15, 20 };
  for(int x : vec2) {
    // UB - push_back invalidates iterators in ranged-for.
    if(x % 2)
      vec2.push_back(x);

    printf("%d\n", x);
  }
}