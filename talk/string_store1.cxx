#feature on safety
#include "std2.h"

using std2::string;
using std2::string_view;

class string_store_t {
public:
  // Imagine deduplicating a string into a string_view.
  string_view insert(self^, string s) safe {
    self->_storage^.push_back(rel s);
    return *self->_storage.back();
  }

  string_view insert(self^, string_view sv) safe {
    return self^->insert(string(sv));
  }

  string_view insert(self^, std2::string_constant<char> sc) safe {
    return self^->insert(string(sc));
  }

private:
  std2::vector<string> _storage;
};

int main() {
  // views has a template lifetime parameter.
  // It must be outlived by all strings pushed into it.
  std2::vector<string_view> views { };

  // Ok. The constant has /static lifetime.
  views^.push_back("A constant string");

  // Put non-constant strings into the store.
  string_store_t store { };

  // OK.
  views^.push_back(store^.insert("A store1 string"));

  // Error. `shared borrow of store1 between its mutable borrow and its use`
  views^.push_back(store^.insert("Another store1 string"));

  for(string_view sv : views)
    println(sv);
}