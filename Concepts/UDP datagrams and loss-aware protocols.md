# UDP datagrams and loss-aware protocols

UDP preserves datagram boundaries: one send creates one datagram, and one receive reads from one datagram. That is different from TCP’s byte stream.

Preserving boundaries does not provide reliability. Datagrams can be lost, duplicated or reordered by the network. Large datagrams can also encounter fragmentation/MTU problems. A real-time protocol therefore chooses explicitly which semantics it needs rather than assuming UDP is “fast TCP.”

## Application responsibilities

If the application needs ordering, include sequence numbers.

If it needs freshness more than completeness, old packets may be discarded instead of retransmitted.

If it needs reliable transfer of some message class, add acknowledgment/retry or use a reliable transport for that class.

If duplicate processing would be harmful, carry request/event identity and make handling idempotent where possible.

If authenticity matters, a sequence number without a MAC/signature can still be forged by an attacker.

## Executable native witness

tools/labs/network_loopback_lab.cpp creates real UDP loopback sockets and sends two different datagrams. Two receive calls recover the two message boundaries independently. The test intentionally does not claim loss/reordering behavior because loopback success is not internet evidence.

Related: [[TCP byte streams and application framing]], [[HMAC and HKDF]].
