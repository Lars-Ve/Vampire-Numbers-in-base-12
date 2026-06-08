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

## Searching Vampire Numbers (search.cpp)
### Configuration
Set `K` in the source file:

const int K = 6;

Meaning:
- K = 4 → 8-digit vampires
- K = 5 → 10-digit vampires
- K = 6 → 12-digit vampires

### Build
g++ -O3 -std=c++20 -pthread search.cpp -o vampire12

### Run
./vampire12 > output_x_digits.out

### Example Output
[FOUND] 1010AB35 = 1031 * BA05  (dec: 36099545 = 1765 * 20453)

## Sorting the Vampire Numbers (sort.cpp)
This sorts the found vampire numbers in ascending order.

### Build
g++ -std=c++17 -O3 sort.cpp -o sort

### Run
./sort output_x_digits.out sorted_x.txt

### Example Output
1270 = 21 * 70  (dec: 2100 = 25 * 84)
