#feature on safety
#include "std2.h"

using namespace std2;

void f(unique_ptr<string> p) safe { }

int main() safe {
  // No default construct. p is uninitialized.
  unique_ptr<string> p;
  // p = unique_ptr<string>::make("Hello");
  println(*p);

  // Try moving to another function and printing.
  // f(rel p);
  // println(*p);  // UB in C++!

  // unique_ptr<string> q = unique_ptr<string>::make("World");
  // p = rel q;
  // println(*p);
}