# Advent of Code 2015 - Day 8 (C64 Edition)

This is a Commodore 64 implementation of Advent of Code 2015 Day 8, written in C and compiled with cc65.

## The Problem

### Part 1: Matchsticks
Calculate the difference between the number of characters in the code representation of string literals and the number of characters in the in-memory string itself.

### Part 2: Encoded Strings
Calculate the difference between the number of characters in a newly encoded version of the string literal and the original code representation.

## Running the Program

### Requirements
- cc65 compiler suite
- Commodore 64 emulator (like VICE) or real C64 hardware

### Compilation
```bash
make
```

This creates `day8.prg` which can be loaded and run on a Commodore 64.

## Test Cases

The program includes the following validation tests:

- `""`
- `"abc"`
- `"aaa\"aaa"`
- `"\x27"`

All tests print "OK" if the results match the expected differences.
