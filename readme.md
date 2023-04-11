# BLoC

This project proposes a file format for programs encoded in binary
lambda calculus. Its main goal is minimal redundancy and therefore
allowing maximal optimzation.

It’s kind of inspired by ELF.

## Format

It’s assumed that all bit-encoded strings are padded with zeroes at the
end.

### Header

| from | to   | content            |
|:-----|:-----|:-------------------|
| 0x00 | 0x04 | identifier: “BLoC” |
| 0x04 | 0x?? | entries            |
| 0x?? | 0x?? | structure          |

### Entry

| from | to   | content                           |
|:-----|:-----|:----------------------------------|
| 0x00 | 0x02 | length of BLC expression in bytes |
| 0x02 | 0x?? | bit-encoded BLC expression        |

The final entry is represented by an entry size of 0.

### Structure

This reflects the basic structure of the program and can’t use
variables. Instead, the structure entry uses the following derivation of
normal bit-encoded BLC:

| prefix   | content                      |
|:---------|:-----------------------------|
| 00M      | abstraction                  |
| 01MN     | application                  |
| 1<entry> | 2 byte reference to an entry |
