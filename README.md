# Vampire-Numbers
Multithreaded C++ program that searches for Base‑12 vampire numbers using digit‑profile pruning and parallel computation.

A high-performance C++ program that searches for 2K-digit vampire numbers in base 12.
The algorithm uses packed digit-frequency profiles, exact digit-sorting verification,
and automatic multithreading to efficiently scan large numeric ranges.

## Features
- Multithreaded search using all available CPU cores
- Fast base-12 conversion
- Packed 4-bit digit-frequency profiles for pruning
- Exact verification to avoid false positives
- Supports K = 2–6 (up to 12-digit vampire numbers)

## Configuration
Set `K` in the source file:

const int K = 6;

Meaning:
- K = 4 → 8-digit vampires
- K = 5 → 10-digit vampires
- K = 6 → 12-digit vampires

## Build
g++ -O3 -std=c++20 -pthread search.cpp -o vampire12

## Run
./vampire12 > output.out

## Example Output
[FOUND] 1010AB35 = 1031 * BA05  (dec: 36099545 = 1765 * 20453)
