# Sparse-set storage

A sparse set maps a potentially large entity index range into a compact dense sequence. Membership checks must verify the dense entry and full entity identity, rather than trusting a sparse index alone. Swap-and-pop removal repairs the moved entry’s reverse mapping and can invalidate component addresses even when no allocation occurs.

Read the connected explanation in [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Let identity survive changes in storage]].

## Mastery expansion — dense iteration with sparse identity lookup

A sparse set typically stores a dense entity array, a dense component array aligned by row, and a sparse mapping from entity index to dense row. Membership is valid only if the sparse entry points inside the dense array and the dense entity at that row matches the requested entity identity.

Removal commonly uses swap-and-pop. If row (r) is removed and the last row (L) is moved into (r), the sparse entry for the moved entity must be rewritten from (L) to (r) before the old tail is discarded. Forgetting that repair produces a data structure that appears correct until the moved entity is queried.

With generational entity IDs, the sparse layer must not confuse an old generation with a newly reused entity index. Store or validate the full generational identity at the dense row.

Iterator/pointer invalidation needs an explicit contract. Reallocation can invalidate addresses; swap-and-pop can invalidate row identity even without reallocation. Therefore long-lived external pointers into component pools are usually unsafe unless the storage strategy explicitly guarantees stability.

**Required next witness:** exhaustive small-state membership/remove/reinsert tests plus randomized differential testing against a simple map model.

