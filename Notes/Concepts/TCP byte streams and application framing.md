# TCP byte streams and application framing

TCP presents an ordered byte stream. It does **not** preserve the boundaries of individual send calls as application messages.

If a sender calls send for header bytes and then payload bytes, the receiver may observe them as one receive, many smaller receives, or any grouping consistent with the ordered byte stream. Therefore the application protocol must define its own framing.

## Length-prefix framing

A common record format is

length | payload

where length has an explicitly defined width and byte order. A receiver keeps a buffer:

1. wait until enough bytes exist to decode the fixed-size length;
2. reject lengths above the protocol maximum before allocating large memory;
3. wait until the complete declared payload is buffered;
4. emit exactly that record;
5. repeat with any remaining bytes.

The decoder must work when the four-byte length field itself arrives in several receives and when several complete frames arrive together.

## Partial I/O

Both send and receive are allowed to make partial progress. A robust send loop advances by the number of bytes actually written. A receive loop interprets zero as orderly stream shutdown and handles errors according to the platform’s retry/error rules.

A single successful TCP connection also does not imply application reliability under disconnect/reconnect. The protocol may need request IDs, replay/idempotence rules, timeouts and state recovery.

## Executable native witness

tools/labs/network_loopback_lab.cpp creates real POSIX loopback TCP sockets. It writes two length-prefixed messages with boundaries deliberately unrelated to the logical messages and forces the receiver to read only three bytes at a time. The stream decoder reconstructs exactly the original two records.

The same laboratory sends two UDP datagrams and verifies that each receive returns one datagram boundary, making the TCP/UDP semantic difference observable.

Related: [[Byte swapping and network order]], [[Integrity and authenticity]].
