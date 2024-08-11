#feature on safety

#include <atomic>
#include <thread>
#include <mutex>
#include <new>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <concepts>
#include <iostream>

namespace std2 {

using size_t = std::size_t;

////////////////////////////////////////////////////////////////////////////////
// Memory utils

template<typename T>
T replace(T* dest, T source) noexcept {
  T old = __rel_read(dest);
  __rel_write(dest, rel source);
  return old;
}

template<typename T>
T replace(T^ dest, T source) noexcept safe {
  unsafe return replace(addr *dest, rel source);
}

template<typename T>
T take(T* dest) noexcept {
  T old = __rel_read(dest);
  __rel_write(dest, T { });
  return old;
}

template<typename T>
T take(T^ dest) noexcept safe {
  unsafe return take(addr *dest);
}

template<typename T>
void swap(T^ x, T^ y) noexcept safe {
  unsafe {
    T a = __rel_read(addr *x);
    T b = __rel_read(addr *y);
    __rel_write(addr *x, rel a);
    __rel_write(addr *y, rel a);
  }
}

template<typename Dst, typename Src>
Dst transmute(Src src) noexcept {
  static_assert(sizeof(Dst) == sizeof(Src));

//  return std::bit_cast<Src>(&src);
}

template<typename T>
void destroy_in_place([T; dyn]^ slice) {
  std::destroy_n((*slice)~as_pointer, (*slice)~length);
}
template<typename T>
void destroy_in_place([T; dyn]* slice) {
  std::destroy_n((*slice)~as_pointer, (*slice)~length);
}


//////////////////////////////////////////////////////////////////////////////// 

struct utf_check { };
struct no_utf_check { };

template<typename CharType>
class basic_string_view/(a);

// C++-style typedefs.
using string_view    = basic_string_view<char>;
using wstring_view   = basic_string_view<wchar_t>;
using u8string_view  = basic_string_view<char8_t>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;

// Rust-style typedefs.
using str    = basic_string_view<char>;
using wstr   = basic_string_view<wchar_t>;
using u8str  = basic_string_view<char8_t>;
using u16str = basic_string_view<char16_t>;
using u32str = basic_string_view<char32_t>;

template<typename CharType>
class 
  __attribute__((preferred_name(string_view)))
  __attribute__((preferred_name(wstring_view)))
  __attribute__((preferred_name(u8string_view)))
  __attribute__((preferred_name(u16string_view)))
  __attribute__((preferred_name(u32string_view)))
basic_string_view/(a) {
public:
  using value_type = CharType;

  // Only allow construction with a string constant. No runtime check
  // because the compiler does that.
  basic_string_view(std2::string_constant<value_type> sc) noexcept safe :
    text(sc.text()) { }

  // Allow construction from any char slice with a runtime check for UTF
  // validity.
  basic_string_view(const [value_type; dyn]^/a init, utf_check) noexcept safe;

  // Unsafe method to construct a view from a slice without a UTF check.
  basic_string_view(const [value_type; dyn]^/a init, no_utf_check) noexcept : 
    text(init) { }

  const value_type* char_pointer(self) noexcept safe {
    return (*self.text)~as_pointer;
  }
  size_t char_length(self) noexcept safe {
    return (*self.text)~length;
  }

  bool operator==(self, basic_string_view rhs) noexcept safe {
    if(self.char_length() != rhs.char_length())
      return false;

    unsafe return !memcmp(self.char_pointer(), rhs.char_pointer(),
      sizeof(value_type) * self.char_length());
  }

private:
  const [value_type; dyn]^/a text;
};

////////////////////////////////////////////////////////////////////////////////
// Safe version of std::source_location

class source_location {
public:
  static constexpr source_location current(
    const char* file_name     = __builtin_FILE(),
    const char* function_name = __builtin_FUNCTION(),
    uint32_t    line          = __builtin_LINE(),
    uint32_t    column        = __builtin_COLUMN()) noexcept safe {

    source_location loc { };
    loc._file_name     = file_name;
    loc._function_name = function_name;
    loc._line          = line;
    loc._column        = column;
    return loc;
  }

  constexpr const char* file_name(const self^) noexcept safe {
    return self->_file_name;
  }

  constexpr const char* function_name(const self^) noexcept safe {
    return self->_function_name;
  }

  constexpr uint32_t line(const self^) noexcept safe {
    return self->_line;
  }

  constexpr uint32_t column(const self^) noexcept safe {
    return self->_column;
  }

private:
  const char* _file_name;
  const char* _function_name;
  uint32_t _line;
  uint32_t _column;
};

////////////////////////////////////////////////////////////////////////////////
// Abort the program 
// Panic functions are categorized and marked with an safety::panic(N) attribute.
// This makes it easy for the frontend to toggle on or off panic calls on a 
// per-file basis.

// These must be coordinated with the compiler.
enum class panic_code {
  generic,
  bounds,
  divide_by_zero,
  lifetime,
};

[[noreturn, safety::panic(panic_code::generic)]] inline void panic(str msg, 
  source_location loc = source_location::current()) noexcept safe {

  unsafe __assert_fail(
    std::string(msg.char_pointer(), msg.char_length()).c_str(),
    loc.file_name(),
    loc.line(), 
    loc.function_name()
  );
}

[[noreturn, safety::panic(panic_code::bounds)]] inline void panic_bounds(str msg,
  source_location loc = source_location::current()) noexcept safe {

  unsafe __assert_fail(
    std::string(msg.char_pointer(), msg.char_length()).c_str(),
    loc.file_name(),
    loc.line(), 
    loc.function_name()
  );
}

////////////////////////////////////////////////////////////////////////////////
// expected and optional 

template<typename T+, typename Err+>
choice expected {
  [[safety::unwrap]] ok(T),
  err(Err)
};

template<typename T+>
choice optional {
  default none,
  [[safety::unwrap]] some(T);

  template<typename Err>
  expected<T, Err> ok_or(self, Err err) noexcept safe {
    return match(self) -> expected<T, Err> {
      .some(x) => .ok(rel x);
      .none    => .err(rel err);
    };
  }

  T expect(self, str msg) noexcept safe {
    return match(self) {
      .some(x) => rel x;
      .none    => panic(msg);
    };
  }

  T unwrap(self) noexcept safe {
    return self rel.expect("{} is none".format(optional~string));
  }
};

////////////////////////////////////////////////////////////////////////////////
// Utilties

template<typename T> // requires(T~is_integral)
optional<T> checked_add(T x, T y) noexcept safe {
  T result = 0;
  // A true result means the operation overflowed.
  if(unsafe __builtin_add_overflow(x, y, addr result))
    return .none;
  else
    return .some(result);
}

template<typename T> // requires(T~is_integral)
optional<T> checked_sub(T x, T y) noexcept safe {
  T result = 0;
  // A true result means the operation overflowed.
  if(unsafe __builtin_sub_overflow(x, y, addr result))
    return .none;
  else
    return .some(result);
}

template<typename T> // requires(T~is_integral)
optional<T> checked_mul(T x, T y) noexcept safe {
  T result = 0;
  // A true result means the operation overflowed.
  if(unsafe __builtin_mul_overflow(x, y, addr result))
    return .none;
  else
    return .some(result);
}

////////////////////////////////////////////////////////////////////////////////
// Permit implicit conversion from string constants to specializations of 
// string_constant with the same character type.
// The constructor is private, so only the compiler can call it.

template<typename CharType>
class string_constant {
  const [CharType; dyn]^/static _text;

  // The compiler will provide this deleted constructor.
  string_constant() = delete;
  
public:
  const [CharType; dyn]^/static text(self) noexcept safe {
    return self._text;
  }
};

////////////////////////////////////////////////////////////////////////////////

template<typename T>
class [[unsafe::sync(false)]] unsafe_cell {
public:
  unsafe_cell() = default;
  unsafe_cell(T value) noexcept safe : value(rel value) { }

  // Even though it performs a const_cast, get_ptr is marked safe because
  // it returns a pointer which can only be dereferenced in an unsafe
  // context.
  T* get_ptr(const self^) noexcept safe {
    return const_cast<T*>(addr self->value);
  }

  // get_ref is marked safe because it takes a mutable borrow for `self`.
  // The law of exclusivity means no other callers can access the unsafe_cell
  // while the returned borrow is live, so we don't have to worry about the
  // interior mutability aspect of preventing data races.
  T^ get_ref(self^) noexcept safe {
    return ^*self.get_ptr();
  }

private:
  T value;
};

template<typename T>
class cell {
public:
  cell(T value) noexcept safe : inner(rel value) { }

  // Safe load and store into the unsafe_cell.
  void set(const self^, T val) noexcept safe {
    self->replace(rel val);
  }

  T get(const self^) noexcept safe {
    unsafe return *self->inner.get();
  }

  T replace(const self^, T val) noexcept safe {
    unsafe return std2::replace(self->inner.get(), rel val);
  }

  T take(const self^) noexcept safe {
    return self->replace(T { });
  }

  void swap(self^, cell^ other) noexcept safe {
    // Taking mutable borrows prevents self-swap.
    unsafe std2::swap(^*self->inner.get(), ^*other->inner.get());
  }

private:
  unsafe_cell<T> inner;
};


template<typename inner_type_+>
class manually_drop {
public:
  using inner_type = inner_type_;

  // Take ownership of argument.
  manually_drop(inner_type value) noexcept safe : _value(rel value) { }

  // Ignore the destructor. A trivial destructor is noexcept and safe.
  ~manually_drop() = trivial;

  // It's only safe to strip the value because it's unsafe to destroy it.
  inner_type into_inner(self) noexcept safe {
    return rel self.inner_type;
  }

  // It's unsafe to manually destroy the object.
  void destroy(self^) noexcept { 
    self->_value^.~inner_type();
  }

  // get is safe because we're assuming the inner object is initialized.
  // If it's uninitialized, that's because it's been dropped with `destroy`,
  // which is unsafe.
  inner_type^ get(self^) noexcept safe {
    return ^self->_value;
  }

  const inner_type^ get(const self^) noexcept safe {
    return ^self->_value;
  }

private:
  inner_type _value;
};

template<typename T>
void forget(T obj) noexcept safe {
  // Destroy the object without running its destructor.
  manually_drop<T>(rel obj);
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
class allocator {
public:
  using value_type = T;

  value_type* allocate(self^, size_t count) safe {
    // Check that the size doesn't exceed max.
    size_t layout_size = checked_mul(count, sizeof(value_type)).expect(
      "allocation overflows byte capacity");

    unsafe {
      void* p = operator new(layout_size, std::align_val_t { alignof(value_type) });
      return static_cast<value_type*>(p);
    } 
  }

  void deallocate(self^, value_type* p, size_t count) {
    ::operator delete(p, std::align_val_t { alignof(value_type) });
  }
};

////////////////////////////////////////////////////////////////////////////////

template<typename CharType, typename Allocator = allocator<CharType> >
class basic_string;

using string    = basic_string<char>;
using wstring   = basic_string<wchar_t>;
using u8string  = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template<typename CharType, typename Allocator>
class [[unsafe::send, unsafe::sync]]
  __attribute__((preferred_name(string))) 
  __attribute__((preferred_name(wstring)))
  __attribute__((preferred_name(u8string)))
  __attribute__((preferred_name(u16string)))
  __attribute__((preferred_name(u32string)))
basic_string {
public:
  using value_type = CharType;
  using allocator_type = Allocator;
  using string_view = basic_string_view<value_type>;
  static_assert(value_type~is_trivially_destructible);

  basic_string() safe = default;

  // Delegate to the basic_string_view constructor.
  basic_string(std2::string_constant<value_type> sc) safe :
    basic_string(string_view(sc)) { }

  basic_string(string_view sv, Allocator alloc = Allocator()) safe :
    _alloc(rel alloc) {

    // Copy the text into the heap.
    if(size_t len = sv.char_length()) {
      _data = _alloc^.allocate(len);
      _len = sv.char_length();
      size_t len = sv.char_length() * sizeof(value_type);
      unsafe std::memcpy(_data, sv.char_pointer(), len);
    }
  }

  // Perform runtime UTF check.
  basic_string(const [value_type; dyn]^ init, utf_check) safe :
    basic_string(string_view(init, utf_check())) { }

  // Unsafe method to construct a string without a UTF check.
  basic_string(const [value_type; dyn]^ init, no_utf_check) :
    basic_string(string_view(init, no_utf_check())) { }

  // Give it a borrow constructor.
  basic_string(const basic_string^ rhs) safe :
    basic_string(rhs.str()) { }

  ~basic_string() safe {
    if( _data)
      unsafe _alloc^.deallocate(_data, _len);
  }

  const [value_type; dyn]^ slice(const self^) noexcept safe {
    unsafe return ^*__slice_pointer(self->_data, self->_len);
  }

  string_view str(const self^) noexcept safe {
    unsafe return string_view(self->slice(), no_utf_check());
  }

  operator string_view(const self^) noexcept safe {
    return self->str();
  }

  const value_type* char_pointer(const self^) noexcept safe {
    return self->_data;
  }

  size_t char_length(const self^) noexcept safe {
    return self->_len;
  }

  void append(self^, string_view rhs) safe {
    unsafe {
      size_t length1 = self->_len;
      size_t length2 = rhs.char_length();

      value_type* new_data = self->_alloc^.allocate(length1 + length2);
      std::memcpy(new_data, self->_data, length1);
      std::memcpy(new_data + length1, rhs.char_pointer(), length2);

      self->_alloc^.deallocate(self->_data, length1);

      self->_data = new_data;
      self->_len = length1 + length2;
    }
  }

  basic_string operator+(const self^, string_view rhs) safe {
    basic_string s = cpy self;
    s^.append(rhs);
    return rel s;
  }

private:
  

  value_type* _data = nullptr;
  size_t _len = 0;
  [[num_unique_address]] allocator_type _alloc = { };
};

using string    = basic_string<char>;
using u8string  = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

////////////////////////////////////////////////////////////////////////////////
// Allow subscript of ref/borrow to slice/array.
// That does implicit deref.
// Also works for things like vector? 
// Should be mutable if the original borrow is mutable.

inline void print(string_view s) noexcept safe { 
  unsafe printf("%*s", s.char_length(), s.char_pointer());
}

inline void println(string_view s) noexcept safe { 
  unsafe printf("%*s\n", s.char_length(), s.char_pointer());
}

inline std::ostream& operator<<(std::ostream& oss, string_view s) {
  oss&->write(s.char_pointer(), (std::streamsize)s.char_length()); 
  return oss;
}

////////////////////////////////////////////////////////////////////////////////
// Interfaces

interface iterator {
  typename item_type;
  optional<item_type> next(self^);
};

interface make_iter {
  typename iter_type;
  typename iter_mut_type;
  typename into_iter_type;

  iter_type      iter(const self^);
  iter_mut_type  iter(self^);
  into_iter_type iter(self);
};

interface sync { };
interface send { };

////////////////////////////////////////////////////////////////////////////////

template<typename T+>
class initializer_list/(a) {
  explicit initializer_list([T; dyn]^/a data) noexcept safe :
    _cur((*data)~as_pointer),
    _end((*data)~as_pointer + (*data)~length) { }

public:
  initializer_list() = delete;
  initializer_list(const initializer_list&) = delete;
  initializer_list& operator=(const initializer_list&) = delete;

  ~initializer_list() requires(T~is_trivially_destructible) = default;

  ~initializer_list() safe requires(!T~is_trivially_destructible) {
    // Destroy the objects.
    unsafe destroy_in_place(self^.slice());
  }

  [T; dyn]^ slice(self^) noexcept safe {
    unsafe return ^*__slice_pointer(self->_cur, self->size());
  }

  const [T; dyn]^ slice(const self^) noexcept safe {
    unsafe return ^*__slice_pointer(self->_cur, self->size());
  }

  optional<T> next(self^) noexcept safe {
    if(self->_cur != self->_end)
      unsafe return .some(__rel_read(self->_cur++));
    else
      return .none;
  }

  T* data_pointer(self^) noexcept safe {
    return self->_cur;
  }

  size_t size(const self^) noexcept safe {
    unsafe return (size_t)(self->_end - self->_cur);
  }

  // Unsafe call to advance. Use this after relocating data out of 
  // data_pointer().
  void advance(self^, size_t size) noexcept {
    self->_cur += size;
  }

private:

  // Point to byte data on the stack.
  T* _cur;
  T* _end;
  T^/a __phantom_data;
};

////////////////////////////////////////////////////////////////////////////////
// Special types known to the compiler.

template<typename... Ts+>
struct tuple {
  [[circle::no_unique_address_any]] Ts ...m;
};

// begin .. end
template<typename type_t>
struct range {
  type_t begin, end;
};

template<typename T>
struct range_iterator {
public:
  range_iterator(range<T> r) noexcept safe : _r(rel r) { }
  optional<T> next(self^) noexcept safe {
    if(self->_r.begin < self->_r.end) {
      return .some(self->_r.begin++);
    } else {
      return .none;
    }
  }

private:
  range<T> _r;
};

template<typename T>
impl range_iterator<T> : iterator {
  using item_type = T;

  optional<T> next(self^) noexcept safe override {
    return self.next();
  }
}

template<typename T>
impl range<T> : make_iter {
  using iter_type = range_iterator<T>;
  using iter_mut_type = range_iterator<T>;
  using into_iter_type = range_iterator<T>;

  iter_type iter(const self^) noexcept safe override {
    return range_iterator<T>(*self);
  }

  iter_mut_type iter(self^) noexcept safe override {
    return range_iterator<T>(*self);
  }
  
  into_iter_type iter(self) noexcept safe override {
    return range_iterator<T>(rel self);
  }
};

// begin ..
template<typename type_t>
struct range_from {
  type_t begin;
};

// .. end
template<typename type_t>
struct range_to {
  type_t end;
};

// ..
struct range_full { };

// begin ..= end
template<typename type_t>
struct range_inclusive {
  type_t begin, end;
};

template<typename T>
struct range_inclusive_iterator {
public:
  range_inclusive_iterator(range_inclusive<T> r) noexcept safe : _r(rel r) { }
  optional<T> next(self^) noexcept safe {
    if(self->_r.begin <= self->_r.end) {
      // TODO: Program wrap-around detection for when end is max.
      return .some(self->_r.begin++);
    } else {
      return .none;
    }
  }

private:
  range_inclusive<T> _r;
};

template<typename T>
impl range_inclusive_iterator<T> : iterator {
  using item_type = T;

  optional<T> next(self^) noexcept safe override {
    return self.next();
  }
}

template<typename T>
impl range_inclusive<T> : make_iter {
  using iter_type = range_inclusive_iterator<T>;
  using iter_mut_type = range_inclusive_iterator<T>;
  using into_iter_type = range_inclusive_iterator<T>;

  iter_type iter(const self^) noexcept safe override {
    return range_inclusive_iterator<T>(*self);
  }

  iter_mut_type iter(self^) noexcept safe override {
    return range_inclusive_iterator<T>(*self);
  }
  
  into_iter_type iter(self) noexcept safe override {
    return range_inclusive_iterator<T>(rel self);
  }
};


// ..= end
template<typename type_t>
struct range_to_inclusive {
  type_t end;
};

// N must be deduced by the implementation.
template<size_t N>
struct subarray_size { };

// Add to operator[] functions for faster calls in `unchecked` contexts.
struct no_runtime_check { };

////////////////////////////////////////////////////////////////////////////////
// Atomics

template<typename T>
class atomic {
public:
  atomic(T value = T()) noexcept safe : _value(rel value) { }
  atomic(const atomic&) = delete;
  operator rel(atomic) = delete;

  T fetch_add(self^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_fetch_add(addr self->_value, op, memory_order);
  }
  T fetch_sub(self^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_fetch_sub(addr self->_value, op, memory_order);
  } 

  T add_fetch(self^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_add_fetch(addr self->_value, op, memory_order);
  }
  T sub_fetch(self^, T op, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_sub_fetch(addr self->_value, op, memory_order);
  }

  T load(const self^, std::memory_order memory_order = std::memory_order_seq_cst) noexcept safe {
    unsafe return __atomic_load_n(addr self->_value, memory_order);
  }

  T operator++(self^) noexcept safe {
    return self^->add_fetch(1);
  }
  T operator++(self^, int) noexcept safe {
    return self^->fetch_add(1);
  }
  
  T operator--(self^) noexcept safe {
    return self^->sub_fetch(1);
  }
  T operator--(self^, int) noexcept safe {
    return self^->fetch_add(1);
  }

private:
  T _value;
};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

template<typename T+>
class unique_ptr {
public:

  static unique_ptr make(T obj) noexcept safe {
    unsafe return unique_ptr(new T(rel obj));
  }
  static unique_ptr make() noexcept safe {
    unsafe return unique_ptr(new T());
  }

  // Unsafe constructor takes ownership of a pointer.
  unique_ptr(T* pointer) noexcept : _pointer(pointer) { }

  // Delete the borrow ctor. This suppresses the copy and move ctors.
  unique_ptr(const unique_ptr^) = delete;

  ~unique_ptr() safe {
    delete _pointer;
  }

  T^ get(self^) noexcept safe {
    unsafe return ^*self->_pointer;
  }
  const T^ get(const self^) noexcept safe {
    unsafe return ^*self->_pointer;
  }

  T* pointer(self^) noexcept safe {
    return addr *(*self)^.get();
  }
  const T* pointer(const self^) noexcept safe {
    return addr *self->get();
  }

  T^ operator*(self^) noexcept safe {
    return self^->get();
  }
  const T^ operator*(const self^) noexcept safe {
    return self->get();
  }

  T^ operator->(self^) noexcept safe {
    return self^->get();
  }
  const T^ operator->(const self^) noexcept safe {
    return self->get();
  }

  T detach(self) noexcept safe {
    unsafe return take<T>(self._pointer);
  }

  T* leak(self) noexcept safe {
    T* data = self._pointer;
    forget(rel self);
    return data;
  }

private:

  T* _pointer;

  // TODO: T may not be complete yet. We want to defer completeness on
  // this member until we're in the MIR doing variance analysis. If it's
  // still incomplete, give its lifetime parameters invariance.
  T __phantom_data;
};

template<typename T+, typename... Ts>
unique_ptr<T> make_unique(Ts... args) safe {
  return unique_ptr<T>::make(T(rel args...));
}

// Atomic shared pointer.
template<typename T+>
class [[
  unsafe::send(T~is_send && T~is_sync), 
  unsafe::sync(T~is_send && T~is_sync)
]] shared_ptr {
  // May name template parameters.
  struct inner_t {
    manually_drop<T> data;
    atomic<size_t> strong;
    atomic<size_t> weak;
  };

public:

  static shared_ptr make(T obj) safe {
    unsafe return shared_ptr(new inner_t { rel obj, 0, 1 });
  }

  // Copy ctor
  shared_ptr(const shared_ptr^ rhs) noexcept safe : 
    unsafe shared_ptr(rhs->_inner) { }

  ~shared_ptr() safe {
    unsafe if(!--_inner->strong) { 
      _inner->data^.destroy();

      if(!--_inner->weak)
        delete _inner;
    }
  }

  // Always give out shared borrows to the inner data. Why? 
  // There may be multiple owners, and giving out mutable borrows violates
  // exclusivity.
  // This construct is intended to be used with interior mutability.
  const T^ operator*(const self^) noexcept safe {
    unsafe return self->_inner->data.get();
  }
  const T^ operator->(const self^) noexcept safe {
    return *(*self);
  }
  
private:

  // Unsafe constructor from inner_t.
  shared_ptr(inner_t* p) noexcept : _inner(p) {
    ++_inner->strong;
  }

  inner_t* _inner;
  T __phantom_data;
};

template<typename T+, typename... Ts>
shared_ptr<T> make_shared(Ts... args) safe {
  return shared_ptr<T>::make(T(rel args...));
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
void relocate_array(T* dest, const T* source, size_t count) noexcept {
  // TODO: Check for relocation and static_assert.
  // TODO: Check for overflow.
  // auto size = checked_mul(count, sizeof(T));
  size_t size = count * sizeof(T);
  std::memmove(dest, source, size);
}


template<typename T>
T min(const T x, const T y) noexcept(noexcept(bool(x < y))) safe {
  return x < y ? rel x : rel y;
}

template<typename T>
T max(const T x, const T y) noexcept(noexcept(bool(x > y))) safe {
  return x > y ? rel x : rel y;
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
class [[safety::niche_zero]] nonnull {
public:
  T* get_p(self^) noexcept {
    return const_cast<T*>(self.p);
  }

private:
  // Covariant in T. Is there a better way to mark covariance?
  const T* p;
};

////////////////////////////////////////////////////////////////////////////////
// slice_iterator

template<typename T>
class slice_iterator/(a) {
public:
  // Safe constructor.
  slice_iterator([T; dyn]^/a slice) noexcept safe : 
    unsafe slice_iterator((*slice)~as_pointer, (*slice)~length) { }

  optional<T^> next(self^) noexcept safe {
    if(self->_data != self->_end) {
      unsafe return .some(^*self->_data++);
    } else {
      return .none;
    }
  }

  size_t length(const self^) noexcept safe {
    unsafe return self->_end - self->_data;
  }

private:
  slice_iterator(T* data, size_t count) noexcept :
    _data(data), _end(data + count) { }

  T* _data;
  T* _end;
  T^/a __phantom_data;
};

template<typename T>
impl slice_iterator<T> : iterator {
  using item_type = T^;

  optional<item_type> next(self^) noexcept safe override {
    return self.next();
  }
};

template<typename T, typename Allocator>
class owning_iterator {
public:
  using value_type = T;
  using allocator_type = Allocator;

  owning_iterator(T* data, T* end) noexcept :
    _data(data), _end(end), _alloc(Allocator()) { }

  optional<T> next(self^) noexcept safe {
    if(self->_data != self->_end)
      unsafe return .some(__rel_read(self->_data++));
    else
      return .none;
  }

private:
  T* _data;
  T* _end;
  [[no_unique_address]] allocator_type _alloc;
};

template<typename T, typename Allocator>
impl owning_iterator<T, Allocator> : iterator {
  using item_type = T;

  optional<item_type> next(self^) noexcept safe override {
    return self^->next();
  }
};

////////////////////////////////////////////////////////////////////////////////
// vector

template<typename T+, typename Allocator+ = allocator<T> >
class vector {
public:
  using value_type = T;
  using allocator_type = Allocator;

  vector() safe :
    _data(nullptr),
    _size(0),
    _capacity(0),
    _alloc(allocator_type()) { }

  vector(std2::initializer_list<T> init_list) safe : vector() {
    size_t length = init_list.size();
    self^.reserve(length);

    unsafe {
      relocate_array(self._data, init_list^.data_pointer(), length);
      self._size = length; 
    }
  }

  size_t size(const self^) noexcept safe {
    return self->_size;
  }

  size_t capacity(const self^) noexcept safe {
    return self->_capacity;
  }
  
  value_type* data(self^) noexcept safe {
    return self->_data;
  }

  const value_type* data(const self^) noexcept safe {
    return self->_data;
  }

  [value_type; dyn]^ slice(self^) noexcept safe {
    unsafe return ^*__slice_pointer(self->_data, self->size());
  }

  const [value_type; dyn]^ slice(const self^) noexcept safe {
    unsafe return ^*__slice_pointer(self->_data, self->size());
  }

  value_type^ operator[](self^, size_t index, no_runtime_check) noexcept safe {
    unsafe return ^self->_data[index];    
  }

  const value_type^ operator[](const self^, size_t index, no_runtime_check) noexcept safe {
    unsafe return ^self->_data[index];
  }

  value_type^ operator[](self^, size_t index) noexcept safe {
    if(index >= self->_size)
      panic_bounds("vector subscript is out-of-bounds");

    unsafe return ^self->_data[index];    
  }

  const value_type^ operator[](const self^, size_t index) noexcept safe {
    if(index >= self->_size)
      panic_bounds("vector subscript is out-of-bounds");

    unsafe return ^self->_data[index];    
  }

  value_type^ back(self^) noexcept safe {
    size_t len = self->_size;
    if(!len)
      panic_bounds("vector::back() on an empty vector");

    unsafe return ^self->_data[len - 1];
  }

  const value_type^ back(const self^) noexcept safe {
    size_t len = self->_size;
    if(!len)
      panic_bounds("vector::back() on an empty vector");

    unsafe return ^self->_data[len - 1]; 
  }

  optional<value_type^> at(self^, size_t index) noexcept safe {
    if(index < self->_size)
      unsafe return .some(^self->_data[index]);
    else
      return .none;
  }

  optional<const value_type^> at(const self^, size_t index) noexcept safe {
    if(index < self->_size)
      unsafe return .some(^self->_data[index]);
    else
      return .none;
  }

  void push_back(self^, value_type value) safe {
    if(self->_size == self->_capacity)
      self^->grow();

    // Relocate into memory and advance _size.
    unsafe __rel_write(self->_data + self->_size++, rel value); 
  }

  template<typename... Ts>
  void emplace_back(self^, Ts... args) safe {
    // TODO: Hook up with construct_at to initialize directly into memory.
    self^->push_back(T(rel args...));
  }

  void insert(self^, size_t index, value_type value) safe {
    unsafe {
      if(self->_size == self->_capacity)
        self.realloc_grow();

      // Move data after the index back one slot.
      relocate_array(self->_data + index + 1, self->_data + index, 
        self->_size - index);

      // Relocate the value into the slot. 
      __rel_write(self->_data + index, rel value);
    }
  }

  void reserve(self^, size_t capacity) safe {
    if(capacity > self->_capacity) {
      // Allocate the new buffer.
      value_type* p = self->_alloc^.allocate(capacity);

      // Relocate the old elements into the new buffer.
      unsafe relocate_array(p, self->_data, self->_size);

      // Save the old data members.
      value_type* old_data = self->_data;
      size_t old_capacity = self->_capacity;

      // Swap in the new data members.
      self->_data = p;
      self->_capacity = capacity;

      // Dealocate the data. If this throws, that's okay, because self's
      // data members are good.
      unsafe self->_alloc^.deallocate(old_data, old_capacity);
    }
  }

  void clear(self^) safe {
    auto elems = static_cast<[value_type; dyn]*>(self^->slice());
    self->_size = 0;
    unsafe destroy_in_place(elems);
  }

private:

  void grow(self^) safe { 
    // Compute a new capacity.
    size_t capacity2 = self->_capacity ? 2 * self->_capacity : 1usize;
    self^->reserve(capacity2);
  }

  value_type* _data;
  size_t _size;
  size_t _capacity;
  [[no_unique_address]] allocator_type _alloc;

  // A critical thing for variances.
  // Should probably start erroring when any lifetime is not named
  // in a data member.
  T __phantom_data;
};

template<typename T, typename Allocator>
impl vector<T, Allocator> : make_iter {
  using iter_type = slice_iterator<const T>;
  using iter_mut_type = slice_iterator<T>;
  using into_iter_type = owning_iterator<T, Allocator>;

  iter_type iter(const self^) noexcept safe override {
    // Call slice_iterator's unsafe constructor.
    // Later we can change this to use a reference to slices.
    return slice_iterator<const T>(self->slice());
  }

  iter_mut_type iter(self^) noexcept safe override {
    return slice_iterator<T>(self^->slice());
  }

  into_iter_type iter(self) noexcept safe override {
    unsafe return owning_iterator<T, Allocator>(
      self^.data(),
      self^.data() + self.size()
    );
  }
};

////////////////////////////////////////////////////////////////////////////////
// Formatting

////////////////////////////////////////////////////////////////////////////////
// Threading

// Safe mutex encapsulates the data in an unsafe_cell.
template<typename T+>
class [[
  unsafe::send(T~is_send), 
  unsafe::sync(T~is_send)
]] mutex {
public:
  mutex(T val) noexcept safe : unsafe inner(), data(rel val) { }

  class lock_guard/(a) {
    friend class mutex;
  public:
    T^ borrow(self^) noexcept safe {
      unsafe return ^*self->_mutex->data.get_ptr();
    }

    const T^ borrow(const self^) noexcept safe {
      unsafe return ^*self->_mutex->data.get_ptr();
    }

    T^ operator*(self^) noexcept safe {
      return self^->borrow();
    }
    const T^ operator*(const self^) noexcept safe {
      return self->borrow();
    }

    ~lock_guard() noexcept safe {
      unsafe _mutex->inner.get_ptr()&->unlock();
    }

  private:
    lock_guard(const mutex^ ref) safe : _mutex(rel ref) { }
    const mutex<T>^/a _mutex;
  };

  lock_guard lock(const self^) noexcept safe {
    unsafe (*self->inner.get_ptr())&.lock();
    return lock_guard(self);
  }

private:
  unsafe_cell<std::mutex> inner;
  unsafe_cell<T> data;
};

class thread {
public:
  // TODO: Add a constraint to test if f() is safe.
  template<std2::send F, std2::send... Args>
  thread/(where F:static, Args...:static)(F f, Args... args) safe
  requires(safe(f(rel args...))) : unsafe _thread() {
    // Smash everything into a tuple.
    using Tup = (F, (Args...,));
    auto ptr = unique_ptr<optional<Tup>>::make(.some((rel f, (rel args... ,))));

    optional<Tup>* p = ptr^.pointer();
    unsafe _thread = std::thread(&thread_stub<optional<Tup>>, &p);

    // If the std::thread constructor exits with an exception, stack
    // unwinding will call ptr's dtor and free the memory.
    // Otherwise, the called thread has relocated the object and called 
    // deallocated the data.
    ptr rel.leak();
  }

  // Delete the copy ctor.
  thread(const thread^) = delete;

  void join(self^) safe {
    unsafe self->_thread&.join();
  }

private:
  template<typename Data>
  static void* thread_stub(Data* data) {   

    // Take ownership of the argument data.
    unique_ptr<Data> ptr(data);

    // Relocate the argument out of the buffer.
    auto tup = ptr rel.detach().expect("thread data was .none");

    // Invoke the callable.
    tup.0(rel tup.1.[:] ...);

    return nullptr;
  }

  std::thread _thread;
};

} // namespace std2
