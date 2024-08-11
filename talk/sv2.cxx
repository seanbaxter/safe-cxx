#feature on safety
#include "std2.h"

using namespace std2;

int main() safe {
  string_view sv = "A view into a literal";
  println(sv);

  string s1 = "A heap string on the outside";
  sv = s1;
  println(sv);

  {
    string s2 = "A heap string on the inside";
    sv = s2;
    println(sv);

    // Make it point to the outer heap string again.
    // sv = s1;
  }

  // No borrow checker error, because the view is known to
  // refer to a still-initialized string.
  println(sv);
}