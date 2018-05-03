# When is it okay to eliminate undefined behaviour?
## ISO/IEC JTC1 SC22 WG21 - DXXXX

Working Group: Evolution, Library Evolution, Undefined Behaviour

Date: 2018-05-02

_Andrew Bennieston \<a.j.bennieston@gmail.com\>_

_Jonathan Coe \<jonathanbcoe@gmail.com\>_

_Daven Gahir \<daven.gahir@gmail.com\>_

_Roger Orr \<rogero@howzatt.demon.co.uk\>_

_Thomas Russell \<thomas.russell97@gmail.com\>_

## TL;DR

Can people still rely on `ubsan` and library instrumentation to find bugs after
a compiler upgrade?

## Introduction

Should C++ guarantee that undefined behaviour remains undefined behaviour as the language and library evolve?

We have recently seen papers that propose rendering currently undefined behaviour as well-defined. In the ensuing discussion, concerns were raised about the possibility of degraded run-time performance (e.g. due to missed optimisation opportunities) and lost ability to detect bugs (e.g. due to tools like `ubsan` being increasingly constrained). Rather than have precedent determined by a small number of concrete cases, we would like to discuss more generally the issues of changes to the language and library that aim to eliminate undefined behaviour.

In this paper, we invite the combined evolution groups to discuss, if not determine, (non-binding) policy on preserving or eliminating undefined behaviour.

A core question that we feel should be explored in more detail is:
> Should the committee support users and implementers that rely on the
> invalidity of currently invalid programs (i.e. relying on undefined
> behaviour) for performance or testing purposes, in addition to the goal of
> retaining validity of currently well-formed programs?

## An example - integer division by zero

Consider the following code:

```c++
int divide(int x, int y) {
  return x / y;
}
```

If `y == 0` then the result of the division cannot be represented as an `int` and the result is undefined. We say that this function requires the client (caller) to provide valid input (i.e. non-zero `y`) to avoid triggering undefined behaviour.

We could have made other design decisions, for example:

```c++
int divide(int x, int y) {
  if (y == 0) {
    throw std::runtime_error("Bad argument: Division by zero");
  }
  return x / y;
}
```

In this implementation, an additional run-time check is made to validate the input. Passing `y = 0` is now valid and triggers well-defined behaviour: an exception is thrown. The price we pay for this is a compulsory check, which may have performance implications. There is no way to opt out of this check if we know already that the input is valid.

Another alternative uses assertions to trigger the check in debug mode and avoid incurring runtime penalties in non-development code:

```c++
int divide(int x, int y) {
  assert(y != 0);
  return x / y;
}
```

Given that integer division-by-zero is undefined behaviour [[1]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf) it may be preferablke to use a sanitized build [[2]](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) [[3]](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) to detect such violations rather than relying on an explicit check.

The eventual choice is an engineering compromise that we are all qualified and happy to make given the particular constraints of our production and development environments.

Here, we concern ourselves not with the decision that is made, but the changes we are later allowed to make or are forbidden from making.

## Which interface changes do we allow?

In the discussion below, we use the term _routine_ to refer to a function or block of code that, in isolation, expects some conditions to be true on entry (preconditions) and guarantees some conditions to be true on exit (postconditions). The presence of input that violates the preconditions may result in undefined behaviour which causes the postconditions to also be violated.

We are intentionally vague about the definition of a _routine_, _precondition_ or _postcondition_; we do not wish to discuss formal contracts, but only the effect of defining previously undefined behaviour.

Although we may express an opinion on general sentiment, we do not take a firm position on whether any of the cases enumerated below should be allowed or prohibited; we aim only to provide a basis for discussion with a view to reaching (non-binding) consensus to guide us in handling specific proposals.

### Relaxing a precondition

In the context of this paper, relaxing a precondition means specifically that we accept a wider range of input and retain well-defined behaviour. This now-defined behaviour may be to throw an exception, which is probably not what the client wanted, but it is at least well defined.

It would be natural to expect that it is always acceptable to widen an interface in such a way, but we must consider the trade-offs we make when doing so.

**Performance:** In many cases, reducing an input restriction incurs a performance penalty due to additional validation checks and/or additional logic to signal an error (`throw`, `assert`, etc.), construct an "empty" value, or do some other special-case processing. This additional cost is incurred even when the client does meet the requirements on entry, and is usually not easy to bypass.

**Diagnostics:** By accepting and handling special values or input cases, we make the behaviour well-defined, but it may not be the behaviour the user intended. There exist diagnostic tools such as `ubsan` [[2]] [[3]] that will instrument code and detect instances where undefined behaviour would be triggered. In some cases, the "fix" is to implement an additional check and some error handling. In other cases, these flag up a genuine mistake or misconception in the program itself. By aiming to always define the behaviour, we reduce the scope for diagnostics to flag questionable cases for developer attention.

### Relaxing a postcondition

In the context of this paper, relaxing a postcondition means specifically that we allow the result of our routine to take on additional states, which were excluded previously. For example, we may have guaranteed that a pointer was non-null, or that a string was not empty, but relax the condition to allow null pointers or empty strings.

Here, the impact is that we may violate a precondition for some subsequent routine, where we previously did not. This could trigger further undefined behaviour, or result in additional validation checks being inserted to handle the increased range of values (of some state).

### Relaxing a diagnostic

### Restricting a precondition
In the context of this paper, restricting a precondition means specifically that we accept a narrower state on entry to a routine. Values outside of the accepted state may trigger undefined behaviour where previously they did not.

In practice, this would mean rendering undefined an operation that was previously well-defined (or at least implementation-defined). While we expect this to be a rare situation, it is included here for completeness.

The impact of such a change would likely be a maintenance headache for clients, who would have to check that their previously valid code did not trigger the newly-undefined behaviour. In this, clients could be assisted by compilers and libraries which could implement a mode that checks for such values and flags them for developer attention.

### Restricting a postcondition

In the context of this paper, restricting a postcondition means specifically that we provide additional guarantees about the state at the end of our routine.

This is a broadly desirable situation, although it may have performance impact in that subsequent code may perform checks that are no longer required. We consider this to be a fairly benign issue as developers regularly have to tidy up legacy code when new functionality becomes available that renders the existing approach unnecessary.

### Restricting a diagnostic

## Case studies

### Relaxing a precondition for `std::string_view`

P0903 [[4]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0903r1.pdf) proposes to make `string_view(nullptr)` well-defined. In this paper, we take no position on whether this should be adopted, but enumerate some of the arguments raised for and against adoption from the perspective of widening the interface.

#### In favour of widening
- It is a non-breaking change – all currently valid code will remain valid.
- There is potentially reduced cognitive load on consumers of `string_view` and maintainers of code using `string_view`.

#### Against widening
- Additional run-time checking, regardless of build mode (i.e. a performance impact).
- Prevents certain classes of potential bugs being detected by the implementation.

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

Let us assume that `getFooName` and `getBarName` always set their second parameter to point to a valid C string (possibly empty, i.e. `"\0"`).

In this case, the programmer forgot to handle the `BAR` case properly, and the call to `processName` may construct a `string_view` with a `nullptr` argument. In the current specification, passing `nullptr` to the single-parameter constructor for `string_view` violates a precondition and may result in undefined behaviour when attempting to calculate the length of the string view (which will involve dereferencing the pointer). This provides an opportunity for a library implementation to emit diagnostics that could guide the developer towards the source of the problem.

By making this constructor valid, and internally reinterpreting it as a call to `string_view(nullptr, 0)`, we would eliminate the possibility for the library to flag this for attention, and instead produce an empty string view, a situation that is much harder to debug as the cause and effect may be separated by some considerable distance or time.

A more subtle example involves the interaction of `initializer_list` constructors with the implicit conversion to `string_view`:

```c++
std::vector<int> v0{0};
std::vector<std::string_view> v1{0};
```

Under the current rules, the vector `v0` clearly has a single element with the value `0`. The vector `v1` is more nefarious; currently it implicitly constructs a `string_view` from `(const char*)0`, which results in undefined behaviour. Permitting null values for this constructor would produce an `initializer_list` containing a single empty `string_view` instead. This is unlikely to be what the user expected.

### Defining the behaviour for signed integer overflow

## Polls

- Should preconditions whose violation may result in undefined behaviour be preserved between C++ standards?
- Should postconditions be preserved between C++ standards?
- Should additional preconditions that may introduce new sources of undefined behaviour be accepted into the C++ standard?
- Should compiler / library diagnostics, optimisations and other technology that makes use of undefined behaviour be considered in-scope when determining if a new feature is non-breaking?

## References

- [[1]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf) N4741 – Working Draft, Standard for Programming Language C++, Section 8.1 paragraph 4, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf
- [[2]](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html) GCC Instrumentation Options, https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
- [[3]](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) Clang Undefined Behaviour Sanitizer, https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
- [[4]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0903r1.pdf) P0903R1 – Define `basic_string_view(nullptr)`, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0903r1.pdf
