#include <iostream>
#include <string_view>

int main() {
  std::string s = "Hellooooooooooooooo ";

  // string_view points into temporary, which expires at
  // end of this statement.
  // diagnose with: clang++ -Wdangling-gsl
  std::string_view sv = s + "World\n";

  std::cout<< sv<< "\n";
}
