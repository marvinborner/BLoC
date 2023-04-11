# BLoC

This project proposes a file format for programs encoded in binary
lambda calculus. Its main goal is minimal redundancy and therefore
allowing maximal optimization.

It’s kind of inspired by ELF.

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

## Example

Let `E` be some kind of expression like `E=\x.(((M (\y.N)) M) N)`, where
`M` and `N` are both arbitrary expressions of length 16.

The raw BLC expression of `E` would then be `E=00010101M00NMN`. This
obviously has the drawback of redundant repetition of `M` and `N`.

A possible encoding in BLoC:

| from | to   | content                                                                               |
|:-----|:-----|:--------------------------------------------------------------------------------------|
| 0x00 | 0x04 | “BLoC”                                                                                |
| 0x04 | 0x06 | number of entries: 2                                                                  |
| 0x06 | 0x16 | encoded `M`                                                                           |
| 0x16 | 0x26 | encoded `N`                                                                           |
| 0x26 | 0x33 | `00010010010011<M>00011<N>011<M>011<N>`, where `<M>=0` and `<N>=1` are 2 byte indices |

Even in this small example BLoC uses less space than BLC (0x33 vs. 0x42
bytes). Depending on the values of `M` and `N`, this could have
potentially been compressed even more.

The compressor in this project uses Merkle trees to accomplish this.
