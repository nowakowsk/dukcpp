# dukcpp

dukcpp is a C++ binding library for [Duktape](https://duktape.org/) ECMAScript engine. It aims to reduce the amount of boilerplate code needed to bind C++ types, objects and functions for use within Duktape context.

While dukcpp does some heavy-lifting when it comes to making binding easier, it is not meant to be a one-size-fits-all binding solution. Its interface closely follows Duktape API so users can use it to build their own binding systems, according to their needs.


# Features

- Unintrusive binding
- Support for fundamental types (integer, floating-point, `bool` and `enum`)
- Support for strings (`const char*`, `std::string`, `std::string_view` and custom string types)
- Support for user-defined types
- Function and functor bindings with parameter validation
- Support for virtual functions and inheritance
- Support for ranges and iterators
- Support for custom memory allocators


# Requirements

dukcpp requires C++20 or later. It is built and tested with the following compilers:

- GCC
- Clang
- MSVC


## Dependencies

- Boost (>= 1.66)
- Duktape (>= 2)


# Usage

After installation, dukcpp can be used like a typical CMake package.

```cmake
find_package(dukcpp)

target_link_libraries(MyTarget
  PRIVATE
    dukcpp::dukcpp
)
```

> **Note:** Using dukcpp in tree (with `add_subdirectory`) is discouraged. It may work, but it isn't officially supported.

After dukcpp target is linked, it can be used by including `<duk/duk.h>` header file.


## Including Duktape library

dukcpp doesn't come with Duktape library. Users are expected to bring their own Duktape. Failure to do so will result in build errors.

Since Duktape doesn't come with CMake integration, dukcpp provides a macro for easy creation of Duktape target based on user-specified path to Duktape source code.

```cmake
add_duktape_library(${DUKTAPE_TARGET_NAME} "${DUKTAPE_PATH}"
  -DDUK_USE_FATAL_HANDLER
  # ...
  # other Duktape config options
  # ...
)
```

After Duktape target is created, it can be linked along with `dukcpp::dukcpp`.


## Tests

In order to build tests, `DUKCPP_DUKTAPE_PATH` CMake variable must be set to Duktape source code directory in CMake configuration stage. It is optional, but CMake will issue a warning if it is not set.

```
cmake -DDUKCPP_DUKTAPE_PATH="path/to/duktape/source/code" ...
```


## Reserved names

dukcpp reserves `duk` namespace, and macro names starting with `DUKCPP_`.

`duk::detail` namespace and macros starting with `DUKCPP_DETAIL_` are reserved for implementation details, and should not be used in user code.


# Quick start

All examples assume we already have a Duktape context created.

```cpp
#include <duk/duk.h>

auto ctx = duk::context(duk_create_heap_default());
```

`duk::context` is a simple RAII, move-only wrapper for `duk_context*`. It exists purely for convenience. Using it is optional as dukcpp API works directly with `duk_context*`.

More usage examples can be found in unit tests (`test/`) and duk project (`duk/`).


## Interacting with Duktape value stack

Duktape provides a set of `duk_push_*` and `duk_get_*` functions for working with value stack. Similarly, dukcpp wraps and gathers those functions under overloaded `duk::push` and `duk::get` functions. Those overloads handle a wide set of types, including callable and user-defined types.

```cpp
duk::push(ctx, 10); // Pushed type is deduced from function parameters.
auto i = duk::get<int>(ctx, -1); // i equals 10

duk::push<std::string>(ctx, "string"); // Pushed type is specified explicitly.
auto s = duk::get<std::string>(ctx, -1); // s equals "string"
```

Using `duk::get` to retrieve a value not matching the type of the value on stack causes undefined behavior. In order to verify the type, use `duk::check_type` function. This function is guaranteed not to throw exceptions.

```cpp
duk::push(ctx, 10);
bool result = duk::check_type<int>(ctx, -1); // result equals true
```

Alternatively, user can retrieve value with `duk::safe_get`, which throws `duk::error` if types don't match.

```cpp
duk::push(ctx, 10);
auto i = duk::safe_get<std::string>(ctx, -1); // throws (wrong type)
```


## Built-in types

`duk::push` and `duk::get` support the following built-in types:

- Integers
- Floating-point
- Enums (dukcpp treats them as integers)
- `bool`


## Strings

By default, the following string types are supported:

- `std::string`
- `std::string_view`
- `const char*`

Users can add support for their own string types by specializing `duk::string_traits` class template in a way that meets the requirements of `duk::string_type` concept.

```cpp
struct MyString { /* ... */ };

namespace duk
{

template<>
struct string_traits<MyString>
{
  static MyString make_string(const char* str, duk_size_t size)
  {
    return MyString(str, size);
  }

  static std::string_view make_view(const MyString& str)
  {
    return str.toStringView();
  }
};

} // namespace duk

// Optional: Make sure MyString meets duk::string_type requirements.
static_assert(duk::string_type<MyString>);
```


## User types

With dukcpp, any copyable or movable object can be pushed to ES context, without any additional work on user's part.

```cpp
struct S {};

duk::push(ctx, S{}); // Push object on value stack.

auto s = duk::get<S>(ctx, -1); // Get object from value stack.
```

Above code creates an `Object` on ES value stack, and moves `S{}` into its internal property. When said ES object gets garbage collected, its finalizer will call destructor of `S`.

Object like this isn't very useful, other than for passing it to bound C++ functions. Adding methods and properties is explained in [Classes](#classes) section.


## Functions

Binding C++ functions is done with `duk::push_function`, which comes in two variants.

The first one, takes a callable object as a non-type template parameter. This variant is faster, but is also quite limited in what callable types it can accept. It is best suited for binding raw function and method pointers.

```cpp
static constexpr auto add = [](int a, int b) { return a + b; };

duk_push_global_object(ctx);
duk::push_function<add>(ctx);
duk_put_prop_literal(ctx, -2, "add");

duk_eval_string(ctx, "add(1, 2);");
auto result = duk::get<int>(ctx, -1); // result equals 3
```

The second one, takes a callable object as a function parameter. It will generally be slower, but can take a wide variety of callables, such as functors, lambdas with capture lists, etc. With this variant, callable objects need to be copyable or movable.

```cpp
auto add = [c = 10](int a, int b) { return a + b + c; };

duk_push_global_object(ctx);
duk::push_function(ctx, add);
duk_put_prop_literal(ctx, -2, "add");

duk_eval_string(ctx, "add(1, 2);");
auto result = duk::get<int>(ctx, -1); // result equals 13
```

For performance reasons, the first variant should be preferred whenever possible.

dukcpp defines `duk::put_prop_function` convenience function which puts functions directly into object properties. It also comes in two variants, just like `duk::push_function`.

```cpp
duk_push_global_object(ctx);
duk::put_prop_function(ctx, -1, "add", add);
```

Calling ES functions from C++ code is described in [Function handles](#function-handles) section.


### Function overloading

When binding overloaded functions, it is necessary to explicitly list all function signatures.

```cpp
void f(int);
void f(std::string);

// ...

duk_push_global_object(ctx);
duk::put_prop_function<
  static_cast<void(*)(int)>(f),
  static_cast<void(*)(std::string)>(f)
>(ctx, -1, "f");
```

In case of functors:

```cpp
struct Functor
{
  constexpr Functor();
  void operator()(int) const;
  void operator()(std::string) const;
};

// ...

// Binding in constexpr context
duk::put_prop_function<
  duk::function_descriptor<Functor{}, void(int), void(std::string)>
>(ctx, -1, "f");

// Binding in non-constexpr context
Functor f;

duk::put_prop_function<
  void(int), void(std::string)
>(ctx, -1, "f", f);
```

When overloaded function is called from ES context, dukcpp goes through all listed overloads in order, and calls the first one which matches all the parameters. Listing most often used overloads first can improve performance.

User should also take into account inheritance relations of function parameters. Functions with most derived parameters should be listed first. Otherwise, they may be shadowed by overloads with less derived parameters. It's a similar situation as in try-catch blocks.


### Functors vs objects

As mentioned in [User types](#user-types) section, by default, `duk::push` puts user objects inside ES `Object` instances. This, however, might not be a desired outcome when dealing with functor objects. It is reasonable for a user to expect that a functor would be represented as ES-callable `Function` instance. There are two basic ways to specify desired ES type for callable objects.

The first one has already been mentioned. Instead of using `duk::push` (which defaults to creating `Object`), user can use `duk::push_function`. It explicitly tells dukcpp to use `Function`. This works fine, but there are situations where we can't always be this explicit. For example, let's imagine that our functor is returned from a function.

```cpp
Functor makeFunctor()
{
  return {};
}

// ...

duk_push_global_object(ctx);
duk::put_prop_function<makeFunctor>(ctx, -1, "makeFunctor");
duk_eval_string(ctx, "makeFunctor() instanceof Function"); // evaluates to false
```

What if we want returned object to be treated as a `Function`?

dukcpp allows to specify that a given type will always be treated as a `Function` by specializing `duk::callable_traits_type` class template.

```cpp
namespace duk
{

template<>
struct callable_traits_type<Functor>
{
  using type = Functor;
};

} // namespace duk
```

With the specialization in place, above call to `makeFunctor` will produce `Function` instead of `Object`.


#### Overloaded functors

For functors with overloaded `operator()` it is necessary to list all desired overloads by specializing `duk::callable_traits_signature_pack` class template.

```cpp
namespace duk
{

template<>
struct callable_traits_signature_pack<Functor>
{
  using type = std::tuple<void(int), void(std::string)>;
};

} // namespace duk
```

`duk::callable_traits_signature_pack` is only required for overloaded callables. When there is no overloading, signature will be deduced automatically.

`duk::callable_traits_signature_pack` doesn't need to specify all overloads. User can list only some of them.


### Support for `std::function`

By default, instances of `std::function` are treated as `Object`. To change it, user needs to include `<duk/callable_std_function.h>`. It will define specialization of `std::callable_traits` for `std::function`.


## Enums

dukcpp treats enums just as if they were regular integer types. They are automatically cast to integers when pushed to ES context, and back to enums when pulled back to C++ code.

A helper `duk::def_prop_enum` function is available to make binding enum types easier.

```cpp
enum class Type
{
  A, B, C
};

// ...

duk_push_object(ctx);
duk::def_prop_enum(ctx, -1, "Type",
  "A", Type::A,
  "B", Type::B,
  "C", Type::C
);

// In ES context this results in:
//
// {
//   Type: {
//     A: 0,
//     B: 1,
//     C: 2
//   }
// }
```


## Classes

In this section we will bind a simple C++ class to ES context. Let's start with our class definition.

```cpp
struct Vector
{
  Vector() = default;
  Vector(float x, float y);

  float length() const;

  float x = 0;
  float y = 0;
};
```


### Constructor

We start by defining class constructor. We need to explicitly list all constructor overloads.

```cpp
duk::push_function<
  duk::ctor<Vector>,              // Vector::Vector()
  duk::ctor<Vector, float, float> // Vector::Vector(float, float)
>(ctx);
```

Next, we create our prototype object.

```cpp
duk_push_object(ctx);
```

The prototype now needs to be filled with properties and methods.


### Methods

Class methods are bound just as regular functions:

```cpp
duk::put_prop_function<&Vector::length>(ctx, -1, "length");
```

#### Free functions as methods

There are situations where it might be desirable to bind free functions as class members. Let's consider an operator defined as a free function.

```cpp
Vector operator-(const Vector& lhs, float rhs);
```

It's reasonable that user might want to use it in ES code in the following manner.

```javascript
var vec1 = new Vector(1, 1);
var vec2 = vec1.sub(1);
```

One way to bind such operator is by explicitly forcing desired member function signature:

```cpp
duk::put_prop_function<
  duk::function_descriptor<
    static_cast<Vector(*)(const Vector&, float)>(&operator-),  // pointer to operator function
    Vector(Vector::*)(float)                                   // desired member function signature
  >,
>(ctx_, -1, "sub");
```

With additional information about desired method signature, dukcpp will consider function's first parameter as `this` pointer to `Vector` object.

In many cases, method signature can be deduced automatically. For those situations, dukcpp provides two helper functions: `duk::push_method` and `duk::put_prop_method`. They come in constexpr and non-constexpr variants, and can be used in the following way.

```cpp
duk::put_prop_method<
  static_cast<Vector(*)(const Vector&, float)>(&operator-)
>(ctx, -1, "sub");
```

In above example, dukcpp knows it is dealing with a method, so it will automatically assume that the first parameter of a free function should be treated as `this` pointer. Using actual method pointers is also allowed.


### Properties

In our example we will use `duk::def_prop` which is used for binding readable and writable members.

```cpp
duk::def_prop<&Vector::x>(ctx, -1, "x");
duk::def_prop<&Vector::y>(ctx, -1, "y");
```

C++ objects used as writable properties need to be assignable.


#### Virtual properties

dukcpp allows binding class methods as virtual ES properties. Let's consider an alternative `Vector` implementation, which uses accessors instead of public members.

```cpp
struct Vector
{
  float x() const;
  void x(float value);

  // ...
};
```

`duk::def_prop_method` can be used to bind accessors as virtual ES properties.

```cpp
duk::def_prop_method<
  static_cast<float(Vector::*)() const>(&Vector::x),  // getter
  static_cast<void(Vector::*)(float)>(&Vector::x)     // setter
>(ctx, -1, "x");
```


### Wrapping it up

With prototype object ready, we complete the class binding process by placing it inside the constructor function.

```cpp
duk_put_prop_string(ctx, -2, "prototype");
```

That's it. After giving our constructor function a name, we can use it as if it was a regular ES "class".

```javascript
var v = new Vector(1, 2);
var length = v.length();
v.x = 1;
v.y = 2;
```

Example code for registering a similar class can be found in `test/vector.cpp`.


### Prototypes

As described in [User types](#user-types) section, when a new object is pushed from C++ code to ES context, it is automatically wrapped in an ES `Object`. By default, such object doesn't have a prototype defined, which limits what we can do with it. To address that problem, dukcpp allows user to explicitly specify a prototype object in `duk::push` call.

```cpp
struct S {};

// Duktape heap pointer to prototype ES Object for type S.
void* prototypeHeapPtr = ...;

duk::push(ctx, S{}, prototypeHeapPtr);
```


#### Static prototypes

But what about situations where we can't specify the prototype explicitly? For example, when an object is returned from a C++ function called in ES context. In this case, we don't have information about our object's prototype.

To solve that, dukcpp introduces static prototypes. They are defined by specializing `duk::class_traits_prototype` class template, and defining a static `get` function, which returns heap pointer to the prototype `Object` for a given C++ type.

```cpp
#include <duk/class.h>

struct S {};

namespace duk
{

template<>
struct class_traits_prototype<S>
{
  static void* get(duk_context* ctx);
};

} // namespace duk
```

Whenever dukcpp will need a prototype for type `S`, it will call `duk::class_traits_prototype<S>::get`.
Implementation of this function is left to the user. Example can be found in `test/common.h`.


### Inheritance

dukcpp supports single inheritance of arbitrary depth. Since there is no standard way to determine a base class of a C++ type, dukcpp relies on the user to provide information on the inheritance hierarchy. It is done by specializing `duk::class_traits` and providing base class information.

```cpp
#include <duk/class.h>

struct MyBaseClass {};

struct MyClass : MyBaseClass {};

namespace duk
{

template<>
struct class_traits<MyClass>
{
  using base = MyBaseClass;
};

} // namespace duk
```

Now whenever an ES function expects `MyBaseClass` parameter, it will also accept `MyClass` objects.

In ES context, it is up to the user to make sure the prototype chain of bound classes reflects their inheritance relations in C++ code.

See `test/inheritance.cpp` for an example.


### Finalization

Objects and callables bound with dukcpp can be finalized manually, before GC decides to do so. This may be useful in case of large objects or resource wrappers (e.g. files, threads, etc.). Early finalization can be forced with `duk::finalize` function. Accessing finalized object will result in an error.


## Iterable objects

dukcpp supports iteration over containers, generators and other iterable objects, both in ES and C++ code. On C++ side, it supports ES arrays and objects compliant with `[Symbol.iterator]` protocol. On ES side, C++ containers can be iterated by wrapping them in ES objects implementing `[Symbol.iterator]` protocol.


### Iteration over ES objects in C++

User can use `duk::get` to create an iterable range for an ES object.

```cpp
duk_peval_string(ctx_, "([1, 2, 3])");
auto r = duk::get<duk::safe_array_input_range<int>>(ctx_, -1);
auto sum = std::accumulate(r.begin(), r.end(), 0); // "sum" equals 6.
```

It is also possible to take iterable objects as function parameters.

```cpp
// Let's assume we want to call the following function from ES to sum some integer values.
// Notice we don't use any part of dukcpp in the interface.
static constexpr auto sum = [](const std::vector<int>& v)
{
  return std::accumulate(v.begin(), v.end(), 0);
};

// Now, we wrap it in a function that tells dukcpp we want to treat the parameter as a range.
// This allows us to bind user-defined functions in an non-intrusive way.
static constexpr auto sum_wrap = [](duk::safe_array_input_range<int> r)
{
  return sum({ r.begin(), r.end() });
};

// We bind the wrapper, and call it from ES.
duk::put_prop_function<sum_wrap>(ctx_, -1, "sum");
duk_peval_string(ctx_, "sum([1, 2, 3]);"); // Puts 6 on ES value stack.
```

dukcpp offers several range types, each suitable for a different use case:

- `duk::array_input_range`

  Random-access range for iterating over ES arrays.

- `duk::symbol_input_range`

  Forward range for iterating over ES objects compliant with `[Symbol.iterator]` protocol.

- `duk::input_range`

  Range defined as a combination of array and symbol range. It works for arrays and iterable objects, depending on the type, making it the most versatile.

In most cases, there is no reason to use `duk::array_input_range` or `duk::symbol_input_range` since `duk::input_range` offers more functionality with minimum overhead.

Internally, dukcpp ranges and iterators keep [handles](#handles) to iterated objects. Just as with dukcpp handles, they come in two variants - safe and unsafe. Unsafe ranges (listed above) use `duk::handle`, and need to be used with care, not to end up with a dangling range.

Additionally, dukcpp offers safe variants of these ranges, which use `duk::safe_handle` internally:

- `duk::safe_array_input_range` 
- `duk::safe_symbol_input_range`
- `duk::safe_input_range`

It is generally advised to use safe ranges, unless user is certain their internal handles can never dangle.

Using dukcpp ranges and iterators directly in user interfaces could be definitely considered intrusive. Their primary use is creating non-intrusive adapter functions to user interfaces.


### Iteration over C++ containers in ES

dukcpp makes it easy to pass an iterable C++ objects to ES code, so they can be iterated over with `[Symbol.iterator]`. Such C++ objects should satisfy requirements of `std::ranges::forward_range` concept, however this isn't currently formally required.

Passing an iterable object to ES is done as follows.

```cpp
// Push an iterable object like any other.
duk::push(ctx_, std::vector<int>{1, 2, 3});

// Define [Symbol.iterator] for the pushed object.
duk::make_iterable<std::vector<int>>(ctx_, -1);
```

When it comes to returning iterable C++ objects from functions, we have two methods of doing that.

```cpp
// Method 1
//
// Explicitly specify signature of the bound function, and wrap the returned
// type in "duk::as_iterable" template.

static constexpr auto make_range = []()
{
  return std::vector<int>{ 1, 2, 3 };
};

duk::put_prop_function<
  duk::function_descriptor<make_range, duk::as_iterable<std::vector<int>>()>
>(ctx_, -1, "make_range");
```

Alternatively, if the first method seems too verbose, user can designate selected types to be always treated as iterable, by specializing `duk::iterable_traits_type`.

```cpp
// Method 2

namespace duk
{

template<typename T>
struct iterable_traits_type<std::vector<T>>
{
  using type = std::vector<T>;
};

} // namespace duk

static constexpr auto make_range = []()
{
  return std::vector<int>{ 1, 2, 3 };
};

duk::put_prop_function<make_range>(ctx_, -1, "make_range");
```

This way, whenever dukcpp encounters `std::vector<T>` being returned from a function, it will automatically assume it should be treated as an iterable object, and appropriate `[Symbol.iterator]` will be defined.


## Handles

Handles are objects pointing to Duktape heap-allocated values. dukcpp comes with two handle types:

- `duk::handle`

  Non-owning handle. Fast, but unsafe. User needs to make sure that pointed value won't be garbage collected before the handle is used. Failure to do so will result in a dangling handle, and in undefined behavior when the handle is used.

- `duk::safe_handle`

  Owning handle. Safe, but slower. It prevents pointed value from being garbage collected by putting it into Duktape global stash. It keeps track of the number of references to the pointed value in C++ code by reference counting.

Both types are copyable and movable. Moving from a handle, invalidates it.

```cpp
duk_push_object(ctx);

// Non-owning handle to the pushed object. It doesn't prevent GC from reclaiming the object. Use with caution.
auto handle = duk::handle(ctx, -1);

// Now we actually take ownership of the object. It won't be garbage collected as long as the safe handle exists.
auto safeHandle = duk::safe_handle(handle);
```

Users can create their own handle types. Such types need to meet the requirements of `duk::handle_type` concept.


## Function handles

Function handles are wrappers around handles pointing to ES functions. They can be called from C++ code. Same as with regular handles, dukcpp comes with two function handle types, with same characteristics in terms of safety and performance:

- `duk::function_handle`
- `duk::safe_function_handle`

Function handles have a single template parameter which specifies function's expected return type. Multiple return types are currently not supported.

Custom function handles can be defined by specializing `duk::function_handle` class template with a custom handle type.

```cpp
duk_eval_string(ctx, "function f(a, b) { return a + b; }; (f);");
auto handle = duk::handle(ctx, -1);
auto safeHandle = duk::safe_handle(handle);
auto f = duk::safe_function_handle<int>(safeHandle);
auto result = f(1, 2); // result equals 3
```

Above code can be simplified:

```cpp
duk_eval_string(ctx, "function f(a, b) { return a + b; }; (f);");
auto f = duk::get<std::function<int(int, int)>>(ctx, -1);
auto result = f(1, 2); // result equals 3
```

It creates a `std::function` objects that wraps `duk::safe_function_handle`. The downside is that it always produces `duk::safe_function_handle`, even if `duk::function_handle` would be enough.

When calling a function handle, it is up to the user to make sure that function parameters match whatever parameters ES function is expecting.


## Type adapters

The primary motivation for type adapters in dukcpp are smart pointers.

In most cases, users will want a guarantee that an ES object referenced in C++ code won't get silently garbage collected, leaving a dangling reference. The same is true in the opposite direction - we don't want C++ code to silently free an object which might be accessible from ES code. The easiest way to guarantee it is to use smart pointers.

```cpp
struct S {};

// ...

auto s = std::make_shared<S>();

duk::push(ctx, s);
```

Now, as long as either C++ or ES code holds a shared pointer to our object, we are safe in both contexts.

Next, let's consider the following C++ code, in which we want to call a function taking a reference, but our object is wrapped in a smart pointer.

```cpp
void func(S& s) {}

// ...

func(s);  // error, because s needs to dereferenced first
func(*s); // ok
```

In order to call `func`, `s` pointer needs to be dereferenced. However, dukcpp doesn't know that - ES `Object` holds smart pointer, and simply tries to pass it to `func`. Clearly, we need some sort of adapter here, and that's where type adapters come in.

Type adapter is a function which converts one type (in our case, a smart pointer) to another one (reference to object pointed by the smart pointer) during dukcpp function call. It is defined by specializing `duk::type_adapter` class template.

```cpp
namespace duk
{

template<typename T>
struct type_adapter<std::shared_ptr<T>>
{
  using type = T; // required

  template<typename U>
  static U get(std::shared_ptr<T> ptr) // required
  {
    return *ptr;
  }
};

} // namespace duk
```

Now, dukcpp knows that when `S&` is requested (`U`), it can be produced by dereferencing `ptr`.


### Type adapter "inheritance"

Since smart pointers don't form an inheritance hierarchy, this further complicates type matching during function calls. Similarly as with [class inheritance](#inheritance), user needs to specify what is the "base class" of our adapted type. It is done by defining `base` type in `duk::type_adapter` specialization.

In our case, we will reuse inheritance information that the user has already provided in `duk::class_traits`.

```cpp
namespace duk
{

template<typename T>
struct type_adapter<std::shared_ptr<T>>
{
  using base = std::conditional_t<
    has_class_traits_base<T>,
    std::shared_ptr<class_traits_base_t<T>>,
    void
  >;
};

} // namespace duk
```

Not defining `base` or defining it as `void` means that there is no base adapter.


# duk project

duk project (`duk/`) is a minimalist Duktape execution environment built with dukcpp. It executes user scripts read from a file, and supports debugging via TCP/IP network socket. It can be used as a basis for more complex projects, and shows how to get things started with dukcpp.

For now, it provides only a single function for printing to standard output:

```javascript
print("Hello World!");
```


# Memory management

dukcpp allocates all of its dynamic memory with `duk_alloc`. User can specify a custom allocation mechanism when creating Duktape context with `duk_create_heap`.

To make using C++ allocators with Duktape easier, dukcpp provides `duk::allocator_adapter`.

```cpp
// Allocator satifsying C++ Allocator named requirements
struct CustomAllocator;

using Alloc = duk::allocator_adapter<CustomAllocator>;

auto alloc = Alloc(CustomAllocator(/* optional custom allocator parameters */));
auto ctx = duk_create_heap(Alloc::alloc, Alloc::realloc, Alloc::free, &alloc, errorHandler);
```


# Threading

Since dukcpp relies on Duktape, all Duktape's threading considerations apply to dukcpp too. General rule is to avoid simultaneous access to a single Duktape heap from multiple threads.

While dukcpp strives to keep little to no static state, some synchronization may be required in selected areas (e.g. [static prototypes](#static-prototypes)).

In contrast to `std::shared_ptr`, [dukcpp handles](#handles) are not thread-safe, and require additional synchronization. Users are free to define their own handles, with stronger threading guarantees.


# Configuration flags

## `DUKCPP_USE_CUSTOM_RTTI`

By default, dukcpp uses `typeid` operator for runtime type validation. If, for any reason, this approach isn't feasible, it may be changed by defining `DUKCPP_USE_CUSTOM_RTTI` macro. In such case, user will need to provide an alternative RTTI mechanism.

It is done by defining `duk::type_id` function, returning a unique id number for any user type exposed to Duktape with dukcpp.

```cpp
namespace duk
{

template<typename T>
std::size_t type_id();

} // namespace duk
```

Example implementations can be found in:
- `include/duk/type_id_typeid.h` (default)
- `include/duk/type_id_static_ptr.h`

If `DUKCPP_USE_CUSTOM_RTTI` macro is set, `duk::type_id` must be defined before including any dukcpp header.


# Limitations

- No support for multiple inheritance
