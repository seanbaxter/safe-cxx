#feature on safety
#include "std2.h"

using namespace std2;

// You can have reference types for short-term tasks.
struct Person/(a) {
  string_view/a name;
  double balance;
};

// The long-term 
struct Database {
  struct PersonStorage {
    size_t name_index;
    double balance;
  };

  vector<string> strings;
  vector<PersonStorage> persons;

  // The const self^ lifetime must outlive Person's /a.
  Person get_person(const self^, size_t index) noexcept safe {
    PersonStorage s = self->persons[index];
    return Person { self->strings[index], s.balance };
  }

  // Person has reference semantics, but the lifetime parameter is elided.
  void insert_person(self^, Person person) safe {
    // Deduplicate the string.
    size_t index = self->strings.size();
    self->strings^.push_back(person.name);

    self->persons^.push_back({ index, person.balance });
  }

  optional<Person> find_person(const self^, string_view name) noexcept safe {
    for(PersonStorage s : self->persons) {
      if(name == self->strings[s.name_index].str())
        return .some(Person { self->strings[s.name_index], s.balance });
    }
    return .none;
  }
};

void print_person/(a)(Person/a person) safe {
  unsafe &std::cout<< person.name<< ": "<< person.balance<< "\n";
}

int main() safe {
  // Database has value semantics here.
  // We can store the data and the "views" in the same struct.
  auto db = unique_ptr<Database>::make();
  db^->insert_person(Person { "Sean", 10.11 });

  match(db->find_person("Sean")) {
    .some(p) => print_person(p);
    .none    => println("Nobody by that name");
  };

  match(db->find_person("Bob")) {
    .some(p) => print_person(p);
    .none    => println("Nobody by that name");
  };
}