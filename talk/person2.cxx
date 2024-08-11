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

// Thread people's lifetime parameter through Database.
struct Database/(a) {
  vector<Person/a> people;
};

int main() {
  // Store the views on the heap.
  // Thread Database's lifetime parameter through unique_ptr.

  // Make a string store on the stack.
  string_store_t store { };

  auto db = unique_ptr<Database>::make();

  // Push from constants.
  db^->people.push_back({"Constant Programmer"});

  // Push from the store on the stack.
  db^->people.push_back({store.insert("Foo Programmer")});
  db^->people.push_back({store.insert("Bar Programmer")});
}