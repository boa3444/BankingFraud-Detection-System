# Demonstration Script & Viva Defense Guide

## 1. Clean Build & Automated Verification
```bash
make clean all        # Compiles ./workbench with gcc -std=c17 -Wall -Wextra
make test             # Runs all unit, oracle, randomized, malformed-CSV, and 10k stress tests
make sanitize         # Re-runs the full test suite under AddressSanitizer + UBSan
```
Expected final line: `ALL_C17_PROJECT07_TESTS_PASSED`.

## 2. Live CLI Demonstration (`./workbench`)
1. **Option `[1]` & `[2]` (Graph & DFS Ring Detection):** Shows the directed `ACCOUNT -> DEVICE -> MERCHANT -> ACCOUNT` graph and detects the `A1001 -> A1002 -> A1003 -> A1001` laundering cycle (`Rs. 72,000`).
2. **Option `[7]` (Multi-Tier & Adversarial Benchmark):** Runs `Small`, `Medium`, `Large`, and `Adversarial` suites across Sorting, Searching, String Matching, Divide & Conquer, and Greedy, exporting 84 rows to `data/benchmark_results.csv`.
3. **Option `[10]` (Three Mandatory Project 07 Scenarios):**
   - *Scenario 1 (Normal Operation):* Ring detection + high-risk account triage + Kruskal investigation MST.
   - *Scenario 2 (Capacity Shock):* Tight 8-hour analyst budget comparing Greedy Fractional vs. DP 0/1 Knapsack vs. FIFO/Least-Cost Branch & Bound node counts.
   - *Scenario 3 (Large-Data Stress):* $n=2{,}000$ transactions comparing Merge Sort vs. Quick Sort comparisons and Max Subarray D&C vs. Kadane's linear scan.

## 3. Viva Defense Quick Answers
- **Why does `BinarySearch` pass `const Transaction*` instead of copying?** Passing by value copies ~224 bytes per element on every recursive call ($O(n \log n)$); passing `const Transaction*` preserves true $O(\log n)$ time.
- **Why is Branch & Bound's fractional upper bound valid?** Relaxing 0/1 integrality to fractional selection can only increase or preserve total risk, so `bound <= bestRisk` safely prunes suboptimal subtrees without missing the integer optimum.
- **Why is 0/1 Knapsack NP-hard while Fractional Knapsack is in P?** Greedy by `risk/hour` is optimal when items can be split, whereas integer constraints allow a polynomial-time reduction from Subset Sum ($\text{Subset Sum} \le_p \text{0/1 Knapsack}$).
