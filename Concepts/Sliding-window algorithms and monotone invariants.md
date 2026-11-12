# Sliding-window algorithms and monotone invariants

A sliding window is not “two indices that move right.” It works when the problem has a monotone property that lets one boundary move without needing to reconsider discarded states.

For a nonnegative sequence and a budget $B$, consider the longest contiguous interval whose sum is at most $B$.

Maintain window $[L,R]$ and sum $S$. Each time $R$ advances, add the new value. While $S>B$, remove values from the left and advance $L$.

Why is this correct? Because all values are nonnegative. If the current sum is too large, extending the right edge cannot make it smaller. Any valid interval ending at the current R must therefore start after some prefix of the current window. Once a left endpoint has been removed for being too expensive, it will never become useful again for a later, larger right endpoint under this invariant.

That monotonicity fails with arbitrary negative values. Example: adding a negative value can make an over-budget window valid again. Reusing the same shrink rule on signed input is therefore an algorithmic bug, not a small edge case.

## Complexity

Each element enters the window once through R and leaves at most once through L, so the total pointer movement is O(n) even though there is a nested while loop.

## Executable differential test

tools/labs/algorithm_patterns_lab.cpp implements the nonnegative-budget window and compares it exhaustively over several arrays/budgets against a quadratic brute-force oracle. A negative-input case is deliberately rejected.

The same lab includes a sorted two-pointer pair-sum search and states its sorted-input precondition.

Related: [[Complexity, asymptotics, and amortized analysis]].
