# Broadening contracts and Preserving Undefined Behaviour
## ISO/IEC JTC1 SC22 WG21 - DXXXX

Working Group: Evolution, Library Evolution, Undefined Behaviour

Date: 2018-04-27

_Jonathan Coe \<jonathanbcoe@gmail.com\>_

_Roger Orr \<rogero@howzatt.demon.co.uk\>_

## TL;DR
Can people still rely on `ubsan` to find bugs after a compiler upgrade?


## Introduction
Should `C++` guarantee that undefined behaviour remains undefined behaviour as
the language and library evolve? 

We have recently seen papers that propose rendering currently undefined
behaviour as well-defined.  Discussion has ensued in which the possibility of
lost optimisation opportunity and lost ability to detect bugs at runtime.
Rather than have precendent determined by a small number of cases we would like
to discuss more generally the issues of changes to contract and detection of
contract violation.

This paper aims to invite the combined evolution groups to discuss, if not
determine, (non-binding) policy on preserving preconditions, preserving
postconditions and preserving behaviour when they are violated. All terminology
and tools referred to are explained in intentionally over-sufficient detail -
any ambiguity would be costly.

## Preconditions and postconditions
A precondition is a condition or predicate that must always be true just prior
to the execution of some section of code or before an operation in a formal
specification.

A postcondition is a condition or predicate that must always be true just after
the execution of some section of code or after an operation in a formal
specification. 

In the presence of inheritance, the routines inherited by descendant classes
(subclasses) do so with their contracts, that is their preconditions and
postconditions, in force. This means that any implementations or redefinitions
of inherited routines also have to be written to comply with their inherited
contracts. Postconditions can be modified in redefined routines, but they may
only be strengthened. That is, the redefined routine may increase the benefits
it provides to the client, but may not decrease those benefits.  Preconditions
can be modified in redefined routines, but they may only be weakened. That is,
the redefined routine may lessen the obligation of the client, but not increase
it.

### Diagnosing violations of preconditions and postconditions
Violation of a precondition or postcondition may be detected at compile time or
at run time.

When violation of a pre/post-condition can be detected at compile time the
compiler may be required to emit a diagnostic.  Compile time detection of
pre/post-condition violation may be deemed too costly to be performed at
compile time.

When violation of a pre.post-condition can only be detected at run time a
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

Given that integer division-by-zero is undefined behaviour [REF] it may be
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

* WIKIPEDIA
* STRING VIEW PAPER

