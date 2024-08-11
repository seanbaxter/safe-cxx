#feature on safety
#include "std2.h"

using namespace std2;

class string_store_t {
public:
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
  unsafe_cell<vector<string>> _storage;
};

// Thread name's lifetime parameter through Person.
struct Person/(a) {
  string_view/a name;
};

// Do we still use a lifetime parameter?
struct Database/(?) {
  // Data.
  string_store_t store;
  
  // Views. Uhh--what's the lifetime argument for Person?
  // We want a view into store.
  // What's store's lifetime?
  vector<Person/?> people;
};
