# St Louis ISO talk

================================================================================
## safe1.cxx

Runtime tests against out-of-bounds array subscript.

================================================================================
## safe2.cxx

Runtime tests against illegal integer math.

================================================================================
## safe3.cxx

Dereferencing pointers is illegal. They're not checked, and you open
yourself up to use-after-free bugs.

1. circle safe3.cxx
2. Replace with borrows. int^ p; p = ^x;

================================================================================
## safe4.cxx

The safe-specifier is part of the function's type. It's just like noexcept.
When converting function pointers, you can implicitly strip off the safe-specifier, just like you can with noexcept, but you can't ipmlicitly add one on, because that would be unsound.

================================================================================
## safe5.cxx

You want to be able to constrain expressions on their safeness.

================================================================================
## unique1.cxx

You may be wondering about the `#feature on safety` line.
This does more than just enable keywords. It changes the object model to support relocation, aka destructive move.

This is critical for solving the null pointer type safety problem.
C++'s unique_ptr many other resource containers are defective because include a default state, where many operations are undefined.

unique_ptr has a default state, but it's undefined behavior to dereference it.
Why does it have a default state? To support move semantics. Move semantics steal the resource and leave old object in its default state. Unfortunately, it may be UB to use it in this default state.

Relocation leaves the old object in an uninitialized state. The compiler can check against that.

================================================================================
## Powers of borrow checking

================================================================================
## sv0.cxx
static analysis:
clang++ sv0.cxx -Wdangling-gsl -o sv0

-Wdangling-gsl uses heuristics to flag use-after-free bugs.
It functions poorly.

Now break into 2 lines:
```cpp
std::string_view sv;
sv = s + "World\n";
```

static analysis: 
clang++ sv0.cxx -Wdangling-gsl -o sv0
./sv0
�n�Uooo World

================================================================================
## sv1.cxx

There is a technology that is compatible with manual memory management 
and stops all lifetime safety bugs: borrow checking.

circle sv1.cxx

```
std2::string_view sv;
sv = s + "World\n";
```

circle sv1.cxx - basic borrow checking

Show `operator string_view` in std2.h. 

That hat is a borrow type. It's a checked reference.
It establishes constraints guaranteeing that the operand string
outlives any references to it. This is how the compiler enforces its
lifetime safety guarantee.

Open up std2.h and showing basic_string_view.

================================================================================
## use1.cxx - Borrow checker errors

A loan is the expression that forms a borrow to an lvalue place.
The borrow checker is a sophisticated form of live analysis, that tracks all borrow types back to their loans. A borrow checker violation is caused by an invalidating action on a place between its loan and its use.

1. Compile with drop.
2. Replace drop and use f(rel s);
3. Replace f(rel s) with s^.append(" Wow!");

================================================================================
## borrow1.cxx / borrow1.rs (shift+alt+8 to do horizontal split)

Super simple borrow checker demonstration.

Borrows are references that have associated lifetimes.
In a function declaration, or in a data member specifier, they must have lifetime parameters.

The lifetime parameter establish a contract between the caller and the callee.

The caller emits lifetime constraints using these lifetime parameters.
The callee emits lifetime constraints using these lifetime parameters.

This lets us get by with only local static analysis--no need for whole-program analysis, because we're using matching constraints at function boundaries to 
establish a provenence between objects and their references.

1. Compile. Show failure.
2. Modify y. That compiles. The loan on y is not in scope when y is modified.
3. Return y from func. That fails because the lifetime b is not related to a.
4. Write where b : a. Now return y; works. We can't modify either x or y. Both loans are in scope.

================================================================================
## lifetimes1.cxx

Lifetime parameters become part of a function's type. 

But there may be many ways to write the lifetimes.

During canonicalization, the graph of lifetimes, where the parameters are the nodes and the outlives-constraints the edges, is reduced to strongly connected components. Those SCCs serve as canonical lifetimes. Function types are mapped to their SCC canonicalizations before comparison.

================================================================================
## sv2.cxx - Non-lexical lifetimes

Borrow checking is control-flow sensitive.

Start by declaring one string_view object. Whenever you assign into it, that creates a new reference, with new constraints. What was previously assigned doesn't matter. 

Here, there's an error when printing this last sv value, because the string it refers to went out of scope.

But we can re-assign to an string that is in scope, and now this is a valid program.

================================================================================
## sv3.cxx - vector

1. Demonstrate with s2, which expires. Fails.
2. Comment out line 21. Compiles.
3. Uncomment line 31. Demonstrate with mutation. Fails.

Look at the ranged-for loops. How do these work? C++ iterators are a well known footgun, and create use after free problems.

We need new, safe iterators.

================================================================================
## iter1.cxx

People say that modern C++ is safer than 90s C++. I don't think that's true.

1. circle iter1.cxx 
./iter1

================================================================================
## iter2.cxx

Borrow checking lets us build iterators that have reference semantics but are also safe against all use-after-errors.

1. circle iter2.cxx

================================================================================
## iter3.cxx

1. circle iter3.cxx
If you want to mutate inside the ranged-for, that's cool. You just have to grab a mutable borrow to the current element.

================================================================================
## sv4.cxx - initlist

We can use initializer_list to associate a vector of views with their backing strings.

This is not the normal std::initializer_list. Thing about everything in C++ that has reference semantics. That's all unsafe. There's UB exposure when using those things.

We have to come up with borrow checked alternatives for everything in the standard library, at least everything that uses reference semantics.

================================================================================
## initlist0.cxx

One is std::initializer_list, which holds a pointer to a backing store. The backing store a separate array. It can expire, leaving the initializer list with a dangling pointer.

$ clang++ initlist0.cxx -stdlib=libc++ -O0 -o initlist && ./initlist

Without optimizations, this executes as expected.

$ clang++ initlist0.cxx -stdlib=libc++ -O2 -o initlist && ./initlist

With optimization, it can break in bizarre ways. You shouldn't be allowed to assign initializer_lists. But you are, and it's always a foot gun.

In this case, seeing the UB is real sensitive to compilers. I've only gotten this output when building with clang with -O2 and with libc++.

================================================================================
## initlist1.cxx  (Do shift+alt+8 split for initlist0.cxx and initlist1.cxx)

I have a std2::initializer_list which uses borrow checking. 
Right away it flags the backing store as expiring prior to its use.

================================================================================
## sv5.cxx - throw

To make C++ really memory safe, we have to consider all cases where reference semantics may be used. If we throw an object, how do we all 

================================================================================
## sv6.cxx - In Safe C++ world, anything with reference semantics has one or more lifetimes. These are signified by lifetime parameters on the type. 

To ensure correctness, sometimes it's necessary to constrain lifetimes of function parameters. In a template context you can name a type parameter as outliving lifetime parameters, including static.

1. Compile. Show stack error.
2. Point string_view to string constant. Recompile.

================================================================================
## sv7.cxx - string_view/static

When you return types with reference semantics, you have to constrain their lifetimes. This prevents returning dangling pointers. 

================================================================================
## sv8.cxx - Lifetime parameters on arguments

The lifetimes on the parameters must outlives the lifetimes on the return type, or else you risk returning dangling pointers. The borrow checker enforces this.

================================================================================
## thread3.cxx - A quick digression into thread safety

Thread safety in this context means no data races.
How does Rust famously guarantee thread safe 


================================================================================
# Limitations of borrow checking

================================================================================
## string_store1.cxx

I've been storing string_view inside vectors.
What if I also want to store the string data itself inside a vector?

There's both a vector of string views and a vector of strings.
The vector of strings is wrapped up in this string_store_t class.
You can register a string, or a string constant, and it may store it efficiently and return a view into it.

The problem is that a insert function mutates the string store.
That invalidates all the views into it.
Without extra help, the compiler can't know that the views aren't invalidated.
For all it knows, the insert could be stomping all over the string data.

1. circle string_store1.cxx

The invalidating action is the insert on line 44.
The loan that is invalidated is the insert on line 41. The loan is still alive, because it must outlive the view into 

================================================================================
## string_store2.cxx

Use interior mutability to permit insertion of new data without invalidating existing views. These strings don't small string optimization so there's no risk of soundness bugs. 

But now we have this new funny vector-like type. And we can't like const iteration on it, because that would be invalidated by the const insertion.

Rust has finer granularity between types, at least types that have reference semantics or may support reference semantics. The C++ style is kind of maximalist. We can't do that here, because we have additional safety invariants we have to uphold.

1. circle string_store2.cxx

================================================================================
## person1.cxx

Try to develop data structures that exihibit reference semantics.

Consider Person type that has a string_view in it as a data member.
The lifetime of the string_view has to be constrained by a lifetime parameter of the containing class. That's why I need this /(a) lifetime parameter on Person struct.

And since I'm loading Persons into a Database vector called people, the Database type needs a lifetime parameter. If I were to further compose, then I'd have to keep dragging the lifetime parameters out.

================================================================================
## person2.cxx

Place the views onto the heap. There's a memory-safe version of unique_ptr.
It's compatible with template argument types that have reference semantics.
Since Database has a lifetime parameter /a, the unique_ptr now has an implicit template lifetime parameter corresponding to Database's /a.
We can push views into the vector though unique_ptr. Now it's stored on the heap.

================================================================================
## person3.cxx

Let's try to store the string themselves on the heap.
Now we have a real dilemma. It's the problem of self references.

================================================================================
## 4.png

For tactical situations, where your references are only live over the scope of a single function, borrow checking is great.

For storage of data, longer term, you need to use value semantics. But there are still a lot of options.

================================================================================
## person4.cxx

We can form references to data stored on the heap... But the borrow originates not on the heap, but on a stack object. In this case, the unique_ptr.
