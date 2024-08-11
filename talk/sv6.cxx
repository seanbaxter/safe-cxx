#feature on safety
#include "std2.h"

// Error if the parameter depends on anything with a non-static lifetime.
template<typename T>
void constrain_static/(where T : static)(const T^) noexcept safe { }

int main() {
  using namespace std2;

  string s = "A stack string";
  string_view sv = s;
  constrain_static(sv);
}
