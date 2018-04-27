# Broadening contracts and Preserving Undefined Behaviour
## ISO/IEC JTC1 SC22 WG21 - DXXXX
##(Uniform-function-calling syntax lite)

Working Group: Evolution, Library Evolution, Undefined Behaviour

Date: 2018-04-27

_Jonathan Coe \<jonathanbcoe@gmail.com\>_

_Roger Orr \<rogero@howzatt.demon.co.uk\>_

## TL;DR
Can people still rely on `ubsan` to find bugs after a compiler upgrade?


## Introduction
Should `C++` guarantee that undefined behaviour remains undefined behaviour as
the language and library evolve?

This paper aims to invite the combined evolution groups to discuss, if not
determine, (non-binding) policy on preserving preconditions, postconditions and
behaviour when they are violated. All terminology and tools referred to are
explained in intentionally over-sufficient detail - any ambiguity would be
costly.

### Preconditions and postconditions

### Violation of preconditions and postconditions

### Using `ubsan` to find bugs at run-time

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
STRING_VIEW_PAPER

