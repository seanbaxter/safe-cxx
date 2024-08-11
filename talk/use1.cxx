#feature on safety
#include "std2.h"

using namespace std2;

void f(string s) safe;

int main() safe {
  // Declare an object.
  string s = "Hello safety";

  // (B) - borrow occurs here
  const string^ ref = s;

  // (A) - invalidating action
  drp s;

  // (U) - use that extends borrow
  println(*ref);
}