# When is it acceptable to eliminate undefined behaviour?
## ISO/IEC JTC1 SC22 WG21 - DXXXX

Working Group: Evolution, Library Evolution, Undefined Behaviour

Date: 2018-05-07

_Andrew Bennieston \<a.j.bennieston@gmail.com\>_

_Jonathan Coe \<jonathanbcoe@gmail.com\>_

_Daven Gahir \<daven.gahir@gmail.com\>_

_Thomas Russell \<thomas.russell97@gmail.com\>_

## TL;DR

Can people still rely on `ubsan` and library instrumentation to find bugs after
a compiler upgrade?

## _UB or not UB_
_To define or not to define? That is the question: whether ‘tis nobler in the
mind to suffer the slings and arrows of undefined behaviour, or to take arms
against a sea it troubles and by defining: end them._

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

## Preconditions and postconditions

Contract-based-programming is a software design method where formal
requirements and guarantees are given for functions. Contract design for C++ is
described in
[[REF]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0380r1.pdf)
and
[[REF]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0542r1.html).

"A precondition is a predicate that should hold upon entry into a function. It
expresses a function's expectation on its arguments and/or the state of objects
that may be used by the function."

"A postcondition is a predicate that should hold upon exit from a function. It
expresses the conditions that a function should ensure for the return value
and/or the state of objects that may be used by the function."

A function with preconditions is said to have a _narrow contract_. Violating
the preconditions on such a function results in undefined behaviour. 

When there are no preconditions on input, a function is said to have a _wide
contract_.  There may be input values for wide-contract functions that result
in exceptions being thrown (`std::vector::at(size_t i)` when
`i>std::vector::size()`) but such behaviour is well-defined.

## Sanitizers and assertions
The undefined behavior sanitizer from GCC and Clang
[[REF]](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) can be
used to produce an instrumented build in which some instances of undefined
behaviour will be detected and the program terminated with a helpful message.  

Standard library implementations can be augmented with debug checks and
assertions to ensure that preconditions are true.  For instance:
`std::vector::operator[](size_t i)` with `i` beyond the end of the vector will
be caught in a debug build using Microsoft's Standard Library implementation.

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
P0907 [[5]](http://wg21.link/p0907r1.html) originally proposed in R0 to make
signed integer overflow well-defined such that it behaves as unsigned integers
on overflowing operations (i.e. overflow in the positive direction wraps around
from the maximum integer value for the type back to the minimum and vice versa
for overflow in the opposite direction). This was subsequently removed from the
proposal following various concerns raised from EWG, SG6 and SG12. Below we
present a quick overview of the reasons for removal of the sub-proposal
defining signed integer overflow.

#### Performance
The primary complaint against defining overflow for signed integers was lost
optimizaton opportunities and the subsequent expected performance degredation.
Modern compilers take advantage of the currently undefined behaviour on signed
integer overflow for a variety of optimizations.

Possibly the most crucial of the currently permissed optimizations is loop
analysis. Even considering a simple inconspicuous seeming `for` loop such as
the following is affected:

```c++
signed int foo(signed int i) noexcept
{
  signed int j, k = 0;
  for (j = i; j < i + 10; ++j) ++k;
  return k;
}
```

A quick glance at this function would expect that this could be trivially
reduced to a simple `return 10` statement during a flow-analysis optimization
pass. Indeed, with the current rules, this is what most modern compilers will
emit. However, under the previously proposed changes, this would no longer be
valid as there are some inputs which would overflow. 

There are a plethora of other optimization opportunities that are similarly
reliant on the undefined behaviour of signed integer overflow. Below is an
(incomplete) summary of other optimizations gathered from
[[6]](https://kristerw.blogspot.co.uk/2016/02/how-undefined-signed-overflow-enables.html):

- `(x * c) == 0` can be optimized to `x == 0` eliding the multiplication.
- `(x * c_1) / c_2` can be optimized to `x * (c_1 / c_2)` if `c_1` is divisible by `c_2`.
- `(-x) / (-y)` can be optimized to `x / y`.
- `(x + c) < x` can be optimized to `false` if `c > 0` or `true` otherwise.
- `(x + c) <= x` can be optimized to `false` if `c >= 0` or `true` otherwise.
- `(x + c) > x` can be optimized to `true` if `c >= 0` and `false` otherwise.
- `(x + c) >= x` can be optimized to `true` if `c > 0` and `false` otherwise.
- `-x == -y` can be optimized to `x == y`. 
- `x + c > y` can be optimized to `x + (c - 1) >= y`.
- `x + c <= y` can be optimized to `x + (c - 1) < y`.
- `(x + c_1) == c_2` can be optimized to `x == (c_2 - c_1)`.
- `(x + c_1) == (y + c_2)` can be optimized to `x == (y + (c_2 - c_1))` if `c_1 <= c_2`.
- Various value-range specific optimizations such as:
  - Changing comparisons `x < y` to `true` or `false`. 
  - Changing `min(x,y)` or `max(x,y)` to `x` or `y` if the ranges do not overlap.
  - Changing `abs(x)` to `x` or `-x` if the range does not cross 0.
  - Changing `x / c` to `x >> log2(c)` if `x > 0` 
  - Changing `x % c` to `x & (c-1)` if `x > 0` and the constant `c` is a power of 2.

## Polls

- Undefined behavior should be preserved from one version of the C++ Standard
  to the next.

- Compiler / library diagnostics that allow undefined behaviour to be trapped
  should be considered when determining if a new feature is non-breaking.

- Compiler / library optimisations that exploit undefined behaviour should be
  considered when determining if a new feature is non-breaking.

## References

- [[1]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf) N4741 – Working Draft, Standard for Programming Language C++, Section 8.1 paragraph 4, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf
- [[2]](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) GCC Instrumentation Options, https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
- [[3]](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) Clang Undefined Behaviour Sanitizer, https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
- [[4]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0903r1.pdf) P0903R1 – Define `basic_string_view(nullptr)`, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0903r1.pdf
