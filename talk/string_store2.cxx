#feature on safety
#include "std2.h"

using std2::string;
using std2::string_view;
using std2::println;

class string_store_t {
public:
  // Insert is a "constant" operation.
  string_view insert(const self^, string s) safe {
    unsafe {
      std2::vector<string>* vec = self->_storage.get_ptr();
      unsafe vec^->push_back(rel s);
      return *vec->back();
    }
  }

  string_view insert(const self^, string_view sv) safe {
    return self->insert(string(sv));
  }

  string_view insert(const self^, std2::string_constant<char> sc) safe {
    return self->insert(string(sc));
  }

private:
  // Interior mutability!
  std2::unsafe_cell<std2::vector<string>> _storage;
};

int main() {
  // vector<string_view> has a lifetime parameter.
  // It is constrained to be outlived by all strings pushed to it.
  std2::vector<string_view> views { };
  views^.push_back("A constant string");

  string_store_t store1 { };
  views^.push_back(store1.insert("A store1 string"));

  // Pushing to store1 is not a "mutation." The previous string_views are
  // not invalidated.
  views^.push_back(store1.insert("Another store1 string"));

  println("Views with store1:");
  for(string_view sv : views)
    println(sv);  

  {
    string_store_t store2 { };
    views^.push_back(store2.insert("A store2 string"));
    views^.push_back(store2.insert("Another store2 string"));

    println("\nViews with store2:");
    for(string_view sv : views)
      println(sv);
  }

  // println("Views after store2 is out of scope:");
  // for(string_view sv : views) {
  //   println(sv);
  // } 
}