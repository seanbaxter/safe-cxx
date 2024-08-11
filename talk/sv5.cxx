#feature on safety
#include "std2.h"

int main() {
  using namespace std2;

  try {
    string s = "A stack string";
    string_view sv = s;
    throw sv;

  } catch(string_view sv) {
    println(sv);
  }
}
