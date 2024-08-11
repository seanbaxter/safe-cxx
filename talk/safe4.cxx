#feature on safety
#include <type_traits>

// `safe` is part of the function type.
using F1 = void(int);
using F2 = void(int) safe;
using F3 = void(int) noexcept;
using F4 = void(int) noexcept safe;

// `safe` is part of the function type.
static_assert(F1 != F2);
static_assert(F3 != F4);

// You can strip off `safe` in function pointers.
static_assert(std::is_convertible_v<F2*, F1*>);
static_assert(std::is_convertible_v<F4*, F3*>);

// You can strip off both `noexcept` and `safe`.
static_assert(std::is_convertible_v<F4*, F1*>);

// You can't add safe. That's unsafe.
static_assert(!std::is_convertible_v<F1*, F2*>);
static_assert(!std::is_convertible_v<F3*, F4*>);

void f1();
void f2() safe;

int main() safe {
  // Ill-formed can only call safe functions from a safe context.
  f1();

  // Okay.
  f2();
}