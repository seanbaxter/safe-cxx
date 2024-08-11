#feature on safety
#include "std2.h"

using namespace std2;

vector<string_view/static> f() safe {
  string s = "A string object";
  return { "Hello world", s };
}

int main() safe {
  vector vec = f();

  for(string_view sv : vec) {
    println(sv);
  }
}
