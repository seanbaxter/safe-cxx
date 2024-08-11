#feature on safety
#include "std2.h"

////////////////////////////////////////////////////////////////////////////////
// Interior mutability avoids aliasing.

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

struct Person/(a) {
  string_view/a name;
};

// Thread the lifetime parameter out.
struct Database/(a) {
  vector<Person/a> people;
};

int main() {

  // Store the views in this Database object on the stack.
  // Database has a lifetime parameter.
  Database db { };

  {
    // Make a string store on the stack.
    string_store_t store { };

    // Push from the store on the stack.
    db.people^.push_back({store.insert("Foo Programmer")});
    db.people^.push_back({store.insert("Bar Programmer")});

    // Push from constants.
    db.people^.push_back({"Constant Programmer"});
  }

  // Any access outside the scope is illegal.
  // db.people^.push_back(Person{"Constant2 Programmer"});

}