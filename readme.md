# BLoC

This project proposes a file format for programs encoded in binary
lambda calculus (BLC). Its main goal is minimal redundancy and therefore
allowing maximal optimization. The included `bloc` tool converts BLC to
BLoC and heavily reduces its size.

## Results

Before explaining the format or its optimization techniques, let me
first show you some results:

1.  The bruijn expression `fac (+30)`, where `fac` is the factorial
    implementation from `std/Math`:
    - the original expression takes 2000 bytes in bit-encoded BLC
    - the same expression in BLoC needs only 423 bytes
2.  [My
    solution](https://github.com/marvinborner/bruijn/blob/main/samples/aoc/2022/01/solve.bruijn)
    for the “Advent of Code” challenge 2022/01 in bruijn:
    - the original expression takes 6258 bytes in bit-encoded BLC
    - the same expression in BLoC needs only 1169 bytes

## Format

It’s assumed that all bit-encoded strings are padded with zeroes at the
end.

### Header

| from | to   | content            |
|:-----|:-----|:-------------------|
| 0x00 | 0x04 | identifier: “BLoC” |
| 0x04 | 0x06 | number of entries  |
| 0x06 | 0x?? | entries            |

### Entry

This reflects the basic structure of an expression. It uses the
following derivation of normal bit-encoded BLC:

| prefix           | content                       |
|:-----------------|:------------------------------|
| 00M              | abstraction of expression `M` |
| 010MN            | application of `M` and `N`    |
| 1<sup>i+1</sup>0 | bruijn index `i`              |
| 011I             | 2 byte index to an entry      |

The final program will be in the last entry. The indices start counting
from the number of entries down to 0.

## Optimizer

The optimizer converts a normal BLC expression to the BLoC format. Its
strategies are similar to compression using finite state entropy.

In order to find the largest repeating sub-expressions, the expression
first gets converted to a hashed tree (similar to a Merkle tree).

The repeating sub-trees then get sorted by length and inserted into the
final table, while replacing them with references in the original
expression (see `src/tree.c` for more).

As of right now, expressions **don’t** get beta-reduced or manipulated
in any other way. As an idea for the future, long expressions could get
reduced using different techniques/depths and then get replaced with the
shortest one (as fully reduced expressions aren’t necessarily shorter).

## Example

Let `E` be some kind of expression like `E=\x.(((M (\y.N)) M) N)`, where
`M` and `N` are both arbitrary expressions of length 16.

The raw BLC expression of `E` would then be `E=00010101M00NMN`. This
obviously has the drawback of redundant repetition of `M` and `N`.

A possible encoding in BLoC:

| from | to   | content                                                                               |
|:-----|:-----|:--------------------------------------------------------------------------------------|
| 0x00 | 0x04 | “BLoC”                                                                                |
| 0x04 | 0x06 | number of entries: 3                                                                  |
| 0x06 | 0x17 | encoded `M`: gets a bit longer due to different encoding                              |
| 0x17 | 0x28 | encoded `N`: gets a bit longer due to different encoding                              |
| 0x28 | 0x35 | `00010010010011<M>00011<N>011<M>011<N>`, where `<M>=1` and `<N>=0` are 2 byte indices |

Even in this small example BLoC uses less space than BLC (0x35 vs. 0x42
bytes). Depending on the content of `M` and `N`, this could have
potentially been compressed even more.
