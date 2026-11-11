# Sparse-set storage

A sparse set maps a potentially large entity index range into a compact dense sequence. Membership checks must verify the dense entry and full entity identity, rather than trusting a sparse index alone. Swap-and-pop removal repairs the moved entry’s reverse mapping and can invalidate component addresses even when no allocation occurs.

Read the connected explanation in [[Continuous Notes/Game Engine - From First Frame to Living Worlds#Let identity survive changes in storage]].
