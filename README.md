# Banking Transaction Fraud Investigation Workbench (`BankTransFraud_C17`)

A modular **C17** implementation of **Project 07** (*Design and Analysis of Algorithms*), covering Sorting, Searching, String Matching, Divide & Conquer, Greedy Methods, Dynamic Programming, Backtracking, Branch & Bound, Network Flow / Bipartite Matching, and Chinese Remainder Theorem.

## Build, Test, and Run

```bash
make            # Compiles ./workbench using gcc -std=c17 -Wall -Wextra
make test       # Compiles and runs tests/test_all.c (oracles, unit, integration, edge, regression)
./workbench     # Launches the interactive CLI menu
make clean      # Removes compiled binaries
```

## CLI Menu Options

- `[1]` Display Fraud Investigation Graph (`FraudGraph`)
- `[2]` Detect Money-Laundering Rings (Recursive DFS)
- `[3]` Run Rule-Based Fraud Scanning (Transaction & Account Risk Reports)
- `[4]` Sort Transactions (Merge Sort / Quick Sort)
- `[5]` Search Transactions (Iterative / Recursive Binary Search)
- `[6]` Scan Transaction Logs (Naive, Rabin-Karp, Finite Automaton, KMP)
- `[7]` Run Complete Performance Benchmark & Export `data/benchmark_results.csv`
- `[8]` Run Dynamic Programming, Backtracking & Branch-and-Bound Demo
- `[9]` Run Ford-Fulkerson Max Flow, Bipartite Matching & CRT Demo
- `[10]` Run Project 07 Three Demo Scenarios (Normal, Capacity Shock, Large-Data Stress)
- `[11]` Exit Workbench

## Project Structure

- `include/` — C17 header files (`transaction.h`, `sorting.h`, `searching.h`, `string_matching.h`, `graph.h`, `graph_types.h`, `fraud_rules.h`, `data_generator.h`, `analysis.h`, `divide_conquer.h`, `greedy.h`, `dp.h`, `backtracking.h`, `flow.h`, `number_theory.h`, `benchmark.h`)
- `src/` — C17 source implementations and `main.c`
- `tests/test_all.c` — Automated unit, integration, oracle, edge-case, invalid-input, and regression test suite
- `data/` — Sample/generated transaction CSVs and `benchmark_results.csv`
- `report.md` — Complexity, recurrence, empirical benchmark, P/NP, and ethics report
