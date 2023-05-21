# BLoC

This project proposes a file format for programs encoded in binary
lambda calculus (BLC). Its main goal is minimal redundancy and therefore
allowing maximal optimization. The included `bloc` tool converts BLC to
BLoC and heavily reduces its size.

## Results

Before explaining the format or its optimization techniques, let me
first show you some results:

1.  x86 C compiler [8cc](https://github.com/rui314/8cc) translated [to
    lambda calculus](https://github.com/woodrush/lambda-8cc):
    - the original expression takes ~5M bytes in bit-encoded BLC
    - the same expression in BLoC needs only ~650K bytes (which is
      around 2x the original 8cc!)
2.  The [bruijn](https://github.com/marvinborner/bruijn) expression
    `fac (+30)`, where `fac` is the factorial implementation from
    `std/Math`:
    - the original expression takes 1200 bytes in bit-encoded BLC
    - the same expression in BLoC needs only 348 bytes
3.  [My
    solution](https://github.com/marvinborner/bruijn/blob/main/samples/aoc/2022/01/solve.bruijn)
    for the “Advent of Code” challenge
    [2022/01](https://adventofcode.com/2022/day/1) in
    [bruijn](https://github.com/marvinborner/bruijn):
    - the original expression takes 6258 bytes in bit-encoded BLC
    - the same expression in BLoC needs only 946 bytes

You can find these examples in `test/`.

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
| 011I             | index\* to an entry           |

(\*): The encoding of indices is quite special: $I=XA$, where
$X\in\\{00,01,10,11\\}$ and length of binary index $A$ is
$L(A)\in\\{1,2,4,8\\}$ byte respectively (see `src/{build,parse}.c`).

The final program will be in the last entry. The indices start counting
from the number of entries down to 0.

## Example

Let `E` be some kind of expression like `E=\x.(((M (\y.N)) M) N)`, where
`M` and `N` are both arbitrary expressions of length 16.

The raw BLC expression of `E` would then be `E=00010101M00NMN`. This
obviously has the drawback of redundant repetition of `M` and `N`.

A possible encoding in BLoC:

| from | to   | content                                                                                                         |
|:-----|:-----|:----------------------------------------------------------------------------------------------------------------|
| 0x00 | 0x04 | “BLoC”                                                                                                          |
| 0x04 | 0x06 | number of entries: 3                                                                                            |
| 0x06 | 0x17 | encoded `M`: gets a bit longer due to different encoding                                                        |
| 0x17 | 0x28 | encoded `N`: gets a bit longer due to different encoding                                                        |
| 0x28 | 0x34 | `00010010010011<M>00011<N>011<M>011<N>`, where `<M>=00{1}` and `<N>=00{0}` are indices with length of 1.25 byte |

Even in this small example BLoC uses less space than BLC (0x34 vs. 0x42
bytes). Depending on the content of `M` and `N`, this could have
potentially been compressed even more.

## Optimizer

The optimizer converts a normal BLC expression to the BLoC format.

In order to find the largest repeating sub-expressions, the expression
first gets converted to a hashed tree (similar to a Merkle tree).

The repeating sub-trees then get sorted by length and inserted into the
final table, while replacing them with references in the original
expression (see `src/tree.c` for more).

As of right now, expressions **don’t** get beta-reduced or manipulated
in any other way. As an idea for the future, long expressions could get
reduced using different techniques/depths and then get replaced with the
shortest one (as fully reduced expressions aren’t necessarily shorter).
