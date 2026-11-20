# Individual Contribution Statement

**Project:** Project 07 — Banking Transaction Fraud Investigation Workbench (`BankTransFraud_C17`)  
**Language & Standard:** Pure ISO C17 (`gcc -std=c17 -Wall -Wextra`)  
**Mode:** Individual student implementation

## Summary of Individual Ownership

All core data structures, algorithm implementations, CLI integration, automated tests, benchmarks, and technical documentation in this repository were designed, implemented, and verified by the author:

1. **Domain & Persistence Layer:** `Transaction`, `TransactionList`, CSV generator (`data_generator.c`), CSV parser (`transaction.c`), and rule-based risk engine (`fraud_rules.c`).
2. **Core Algorithmic Libraries (All 9 Syllabus Families):**
   - *Sorting & Searching:* Merge Sort, Quick Sort (`sorting.c`), Iterative & Recursive Binary Search (`searching.c`).
   - *String Matching:* Naive, Rabin-Karp, Finite Automaton, KMP (`string_matching.c`).
   - *Divide & Conquer:* Max Subarray D&C vs. Kadane, Modular Exponentiation, Celebrity Problem, Classical vs. Strassen Matrix Multiplication (`divide_conquer.c`, `divide_conquer_matrix.c`).
   - *Greedy Methods:* Activity Selection, Interval Partitioning, Fractional Knapsack, Huffman Coding, Dijkstra, Kruskal & Prim MST (`greedy.c`).
   - *Dynamic Programming:* 0/1 Knapsack, Matrix-Chain Multiplication, LCS, Optimal BST, Bellman-Ford, Floyd-Warshall, Bounded Held-Karp TSP (`dp.c`).
   - *Backtracking & Branch and Bound:* N-Queens, Sum of Subsets, 0/1 Knapsack Backtracking, FIFO / LIFO / Least-Cost Branch & Bound (`backtracking.c`).
   - *Flow & Matching:* Ford-Fulkerson (Edmonds-Karp) Max Flow and Maximum Bipartite Matching (`flow.c`).
   - *Number Theory:* Extended GCD, Modular Inverse, Chinese Remainder Theorem (`number_theory.c`).
3. **Verification, Benchmarking & Release Engineering:** `Makefile` (`all`, `test`, `sanitize`, `clean`), `tests/test_all.c` (oracles, randomized property tests, malformed CSV, negative-cycle, and 10k stress tests), `benchmark.c` (84-row CSV export), and `report.md`.
