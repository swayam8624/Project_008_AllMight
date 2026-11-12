# Memory-mapped files and page-backed views

Memory mapping asks the operating system to make a file region accessible through virtual addresses. The program then loads/stores through memory while the OS and filesystem coordinate page residency and backing storage.

A mapping is not “the file loaded into RAM.” Pages can fault in on demand, be evicted, and be written back later. Performance therefore depends on access order, page size, locality and OS policy.

## Shared versus private mappings

A shared writable mapping can propagate changes toward the underlying file. A private mapping gives copy-on-write semantics: modifications become private to the process rather than being published back as ordinary file writes.

The mapping lifetime and file descriptor lifetime are related but not identical on POSIX: an established mapping can remain valid after the descriptor used to create it is closed. The mapping itself must remain alive while pointers into it are used.

## Bounds

Mapping a file does not make out-of-range tensor offsets safe. A binary loader still needs to prove that every interpreted record and tensor interval lies inside the mapped file and that arithmetic cannot overflow before the pointer is formed.

This is directly relevant to GGUF and asset containers.

## Executable witness

tools/labs/posix_vm_file_lab.cpp creates a 4096-byte file-backed MAP_SHARED mapping, writes three locations, synchronizes/unmaps, and reads the bytes back through file I/O.

This is execution evidence for one POSIX/Linux-like environment. It does not prove crash durability, page-fault performance, Windows mapping APIs or production model-loader behavior.

Related: [[GGUF binary model loading]], [[POSIX file descriptors and open file descriptions]], [[Huge pages and TLB reach]].
