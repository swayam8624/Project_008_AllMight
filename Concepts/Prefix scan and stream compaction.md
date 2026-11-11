# Prefix scan and stream compaction

Exclusive prefix sums turn local output counts into unique output offsets. Predicate, scan and scatter can compact data without an atomic increment per output. Hierarchical scans require correct dependencies between phases.

Teach and reconstruct: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Scan turns local counts into nonoverlapping output regions]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].
