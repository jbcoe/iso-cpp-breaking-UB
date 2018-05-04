# When is it okay to eliminate undefined behaviour?
## ISO/IEC JTC1 SC22 WG21 - DXXXX

Working Group: Evolution, Library Evolution, Undefined Behaviour

Date: 2018-05-02

_Andrew Bennieston \<a.j.bennieston@gmail.com\>_

_Jonathan Coe \<jonathanbcoe@gmail.com\>_

_Daven Gahir \<daven.gahir@gmail.com\>_

_Thomas Russell \<thomas.russell97@gmail.com\>_

## TL;DR

Can people still rely on `ubsan` and library instrumentation to find bugs after
a compiler upgrade?

## Introduction

Should C++ guarantee that undefined behaviour remains undefined behaviour as
the language and library evolve?

We have recently seen papers that propose rendering currently undefined
behaviour as well-defined [REFS]. In the ensuing discussion, concerns were
raised about the possibility of degraded run-time performance (e.g. due to
missed optimisation opportunities) and lost ability to detect bugs (e.g. due to
tools like `ubsan` being increasingly constrained). Rather than have precedent
determined by a small number of concrete cases, we would like to discuss more
generally the issues of changes to the language and library that aim to
eliminate undefined behaviour.

In this paper, we invite the combined evolution groups to discuss, if not
determine, (non-binding) policy on preserving or eliminating undefined
behaviour.

Should the committee support users and implementers that rely on the invalidity
of currently invalid programs (i.e. relying on undefined behaviour) for
performance or testing purposes, in addition to the goal of retaining validity
of currently well-formed programs [REF]?

## A simple example - integer division by zero

Consider the following code:

```c++
int divide(int x, int y) {
  return x / y;
}
```

If `y == 0` then the result of the division cannot be represented as an `int`
and the result is undefined. This function requires the client (caller) to
provide valid input (i.e. non-zero `y`) to avoid triggering undefined
behaviour.

We could avoid undefined behaviour by implementing an explicit check and
handling invalid input with an exception:

```c++
int divide(int x, int y) {
  if (y == 0) {
    throw std::runtime_error("Bad argument: Division by zero");
  }
  return x / y;
}
```

or by returning an `expected` type [REF]:

```c++
Expected<int, std::string> divide(int x, int y) {
  if (y == 0) {
    return {unexpected, "Bad argument: Division by zero"};
  }
  return x / y;
}
```

In this implementation, an additional run-time check is made to validate the
input. Passing `y = 0` is now valid and triggers well-defined behaviour: an
exception is thrown. The price we pay for this is a compulsory check, which may
have performance implications. There is no way to opt out of this check if we
know already that the input is valid.

The first version of `divide` with no check is said to have a 'narrow contract'
[REF], there is a subset of input that is invalid and calling the function with
invalid input results in undefined behaviour. The second and third versions
have a wide contract: no input results in undefined behaviour but bad input
values (`y=0`) will result in an exception being thrown or an `unexpected`
return value.

Users of the `divide` function(s) would need to handle the exception, unpack
the `expected` value or guarantee that input is valid either with explicit
checks or by relying on function postconditions.

The eventual choice of which `divide` to use is an engineering compromise that
we are all qualified and happy to make given the particular constraints of our
production and development environments.  Here, we concern ourselves not with
the decision that is made, but the changes we are later allowed to make or are
forbidden from making.

## Sanitizers and assertions
[TODO]

## Preconditions and postconditions
[TODO]

## Which interface changes do we allow?

Given current guidance [REF], we consider the following changes to a function
interface:

* Convert a wide contract to a narrow contract
* Convert a narrow contract to a wide contract
* Widen a narrow contract
* Narrow a narrow contract
* Make a function `noexcept`
* Remove `noexcept` from a function definition
* Change the return type of a function 

### Convert a wide contract to a narrow contract
### Convert a narrow contract to a wide contract
### Widen a narrow contract
### Narrow a narrow contract
### Mark a function as `noexcept`
### Remove `noexcept` from a function definition
### Change the return type of a function from `T `to `expected<T>`
### Change the return type of a function from `expected<T>` to `T`

## Case studies

### Relaxing a precondition for `std::string_view`

P0903
[[4]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0903r1.pdf)
proposes to make `string_view((const char*)nullptr)` well-defined. In this
paper, we take no position on whether this should be adopted, but enumerate
some of the arguments raised for and against adoption from the perspective of
widening the interface.

#### In favour of widening
- It is a non-breaking change – all currently valid code will remain valid.
- There is potentially reduced cognitive load on consumers of `string_view` and
  maintainers of code using `string_view`.

#### Against widening
- Additional run-time checking, regardless of build mode (i.e. a performance
  impact).
- Prevents certain classes of potential bugs being detected by the
  implementation.

To explore these points further, consider the following code:

```c++
std::string processName(std::string_view username);

std::string getUserName(const UserConfig& user_config) {
  const char* username = nullptr;
  
  switch (user_config.user_type) {
    case UserType::FOO:
      getFooName(user_config.uuid, &username); // external library C function
      break;
    case UserType::BAR:
      // Programmer error: forgot to call getBarName
      break;
  }

  return processName(username);
}
```

Let us assume that `getFooName` and `getBarName` always set their second
parameter to point to a valid C string (possibly empty, i.e. `"\0"`).

In this case, the programmer forgot to handle the `BAR` case properly, and the
call to `processName` may construct a `string_view` with a `nullptr` argument.
In the current specification, passing `nullptr` to the single-parameter
constructor for `string_view` violates a precondition and may result in
undefined behaviour when attempting to calculate the length of the string view
(which will involve dereferencing the pointer). This provides an opportunity
for a library implementation to emit diagnostics that could guide the developer
  towards the source of the problem.

By making this constructor valid, and internally reinterpreting it as a call to
`string_view(nullptr, 0)`, we would eliminate the possibility for the library
to flag this for attention, and instead produce an empty string view, a
situation that is much harder to debug as the cause and effect may be separated
by some considerable distance or time.

### Defining the behaviour for signed integer overflow
The notion of making signed integer overflow well-defined was briefly
entertained [REF].  Currently signed integer overflow is undefined behaviour
and can be exploited by an optimising compiler to vectorize loops [GODBOLT] and
by an instrumented compiler to catch bugs caused by overflow [UBSAN].

## Polls

- Preconditions whose violation may result in undefined behaviour should be
  preserved between C++ standards.
- Postconditions should be preserved between C++ standards?
- New preconditions that may introduce new sources of undefined behaviour can
  be accepted into the C++ standard.
- Compiler / library diagnostics, optimisations and other technologies that
  make use of undefined behaviour should be considered when determining if a
  new feature is non-breaking.

## References

- [[1]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf) N4741 – Working Draft, Standard for Programming Language C++, Section 8.1 paragraph 4, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf
- [[2]](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) GCC Instrumentation Options, https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
- [[3]](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) Clang Undefined Behaviour Sanitizer, https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
- [[4]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0903r1.pdf) P0903R1 – Define `basic_string_view(nullptr)`, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0903r1.pdf
