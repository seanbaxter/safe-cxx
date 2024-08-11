#feature on safety
#include "std2.h"

using namespace std2;

auto make_vec/(a)(string_view/a s0, string_view/a s1) safe -> 
  vector<string_view/a> {

  return { s0, s1 };
}

int main() safe {
  string a = "Some string";

  vector<string_view> vec;
  {
    string b = "Some other string";
    vec = make_vec(a, b);

    println("Printing from the inner scope:");
    for(string_view sv : vec) {
      println(sv);
    }
  }

  println("Printing from the outer scope:");
  for(string_view sv : vec) {
    println(sv);
  }
}