# Broadening contracts and Preserving Undefined Behaviour
## ISO/IEC JTC1 SC22 WG21 - DXXXX

Working Group: Evolution, Library Evolution, Undefined Behaviour

Date: 2018-04-27

_Jonathan Coe \<jonathanbcoe@gmail.com\>_

_Roger Orr \<rogero@howzatt.demon.co.uk\>_

_Daven Gahir \<daven.gahir@gmail.com\>_

_Andrew Bennieston \<a.j.bennieston@gmail.com\>_

_Thomas Russell \<thomas.russell97@gmail.com\>_

## TL;DR
Can people still rely on `ubsan` and/or library instrumentation to find bugs after a compiler upgrade?

## Introduction
Should `C++` guarantee that undefined behaviour remains undefined behaviour as
the language and library evolve? 

We have recently seen papers that propose rendering currently undefined
behaviour as well-defined.  Discussion has ensued in which concerns around the possibility of
run-time performance degregatation and lost ability to detect bugs at run-time have been raised.
Rather than have precendent determined by a small number of cases we would like
to discuss, more generally, the issues of changes to contract and detection of
contract violation.

This paper aims to invite the combined evolution groups to discuss, if not
determine, (non-binding) policy on preserving preconditions, preserving
postconditions and preserving behaviour when they are violated. All terminology
and tools referred to are explained in intentionally over-sufficient detail -
any ambiguity would be costly.

A core question that we wish to explore is: 
> Should the committee support users and implementers relying on the invalidity of currently invalid programs for performance/testing purposes as well as retained validity of currently well-formed programs?

## Preconditions and postconditions
A precondition [[1]](https://en.wikipedia.org/wiki/Precondition) is a condition
or predicate that must always be true just prior to the execution of some
section of code or before an operation in a formal specification.

A postcondition [[2]](https://en.wikipedia.org/wiki/Postcondition) is a
condition or predicate that must always be true just after the execution of
some section of code or after an operation in a formal specification.

A contract is the set of preconditions and postconditions guaranteed by the
interface to a section of code.

In the presence of inheritance, the routines inherited by descendant classes
(subclasses) do so with their contracts in force. This means that any
implementations or redefinitions of inherited routines also have to be written
to comply with their inherited contracts. Postconditions can be modified in
redefined routines, but they may only be strengthened. That is, the redefined
routine may increase the guarantee it provides to the client, but may not
decrease it.  Preconditions can be modified in redefined routines, but they may
only be weakened. That is, the redefined routine may lessen the obligation of
the client, but not increase it.

### Diagnosing violations of preconditions and postconditions
Violation of a precondition or postcondition may be detected at compile time or
at run time.

When violation of a pre/post-condition can be detected at compile time the
compiler may be required to emit a diagnostic.  Compile time detection of
pre/post-condition violation may be deemed too costly to be performed.

When violation of a pre/post-condition can only be detected at run time a
run-time diagnostic may be emitted. This could be a thrown exception [REFs] or
an assertion that is only observable in specific build modes. Run time
detection of pre/post-condition violations may be deemed too costly and it may
be preferable if the compiler were to assume that the pre/post-condition were
true and optimise accordingly. 

[Signed overflow and vectorisation]

Consider the following code:

```~cpp
int divide(int x, int y) {
  return x / y;
}
```

If `y=0` then the result of the division cannot be represented as an `int` and
the result is undefined. We can require, as a precondition, that `y!=0` and
guarantee as a postcondition that calling `divide` will not result in program
termination.

This could be enforced by an assertion:

```~cpp
int divide(int x, int y) {
  assert(y != 0);
  return x / y;
}
```

or with an exception:

```~cpp
int divide(int x, int y) {
  if(y == 0) 
    throw std::runtime_error("Bad argument: y==0");
  return x / y;
}
```

The assertion will be active in debug mode only and will incur no cost in
non-development code.

The check and exception are always active and will have performance impact in
non-development code.

Given that integer division-by-zero is undefined behaviour [[3]](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf) it may be
preferable to use a sanitized build [REF] to detect such contract violations
rather than rely on an explicit debug check. 

The eventual choice is an engineering compromise that we are all qualified and
happy to make given the particular constraints of our production and
development environments.

We concern ourselves not with the decision that is made, but with the changes we
are later allowed to make or forbidden from making.

### Relaxing preconditions

### Relaxing postconditions

### Relaxing diagnostics

### Restricting preconditions

### Restricting postconditions

### Restricting diagnostics

## Reference Cases

### Relaxing preconditions for `string_view`
P0903 [REF] proposes making passing `nullptr` to `string_view::string_view(const char*)` well defined. In this paper we will take no position on whether or not this should be adopted but enumerate some of the arguments raised for and against adoption from the perspective of widening the contract.

#### In favour of widening
- It is a non-breaking change, all currently valid code will remain valid.
- Potentially reduced cognitive load on consumers of `string_view` and maintainers of code utilizing `string_view`.

#### Against widening
- Guaranteed additional run-time checking regardless of build mode.
- Prevents certain classes of potential bugs being detected by the library implementation. 

To explore these points further, consider the following code:

```c++
std::string processName(std::string_view userName);
std::string getUserName(const UserConfig& userConfig)
{
    const char* userName;
    switch (userConfig.userType)
    {
        case UserType::FOO:
        {
            getFooName(userConfig.uuid, &userName); // External library C function.
            break;
        }
        // case UserType::BAR: // Programmer error
        // {
        //     getBarName(userConfig.uuid, &userName); // External library C function
        //     break;
        // }
    }
    
    return processName(userName);
}
```

Let us also assume that `getFooName` and `getBarName` can set `userName` to point to an empty string literal when there does not exist a user of an appropriate type with the specified `uuid`. 

In this case the programmer has forgotten to handle the `BAR` user type. Currently, when he/she calls `getUserName` with a `BAR` user, during the internal call to `processName` and implicit construction of the `string_view`, there is an opportunity for his/her standard library implementation to report the mistake. Making `string_view(nullptr)` defined and the empty string would prohibit such an instrumented implementation; the programmer may subsequently expend significant effort tracing this bug as the empty string signifies something specific in his/her application.

Perhaps an even more subtle example would be the interaction of `initializer_list` constructors with the implicit conversion to `string_view`:

```c++
std::vector<int> v0{0};
std::vector<std::string_view> v1{0};
```

With current rules, the first vector `v0` clearly has a single element with the value `0`. The second vector is more nefarious, currently it implicitly constructs a `string_view` from `(const char*)0`, currently this is undefined behaviour and most implementations in debug mode will either throw an exception or `assert`.

### Removing undefined behaviour for signed integer overflow

## Polls

* Compile-time preconditions should be preserved between C++ standards.

* Run-time preconditions should be preserved between C++ standards.

* Compile-time postconditions should be preserved between C++ standards.

* Run-time postconditions should be preserved between C++ standards.

* Pre and postcondition violations that result in compiler diagnostics should be preserved between C++ standards.

* Pre and postcondition violations that result in exceptions should be preserved between C++ standards.

* Pre and postcondition violations that result in undefined behaviour should be preserved between C++ standards.


## Acknowledgements
The authors would like to thank their friends and families.


## References

* [1] Precondition, Wikipedia, https://en.wikipedia.org/wiki/Precondition
* [2] Postcondition, Wikipedia, https://en.wikipedia.org/wiki/Postcondition
* [3] Section 8.1 paragraph 4, N4741 (Working Draft, Standard for Programming Language C++) http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf
* [4] Define `basic_string_view(nullptr)`, P0903R1, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0903r1.pdf

