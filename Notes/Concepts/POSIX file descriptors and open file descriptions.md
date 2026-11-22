# POSIX file descriptors and open file descriptions

A POSIX file descriptor is a small process-local integer referring to an open file description maintained by the kernel. The descriptor number is not the file itself.

Duplicating a descriptor with dup creates another descriptor referring to the same open file description. Among other shared state, the current file offset is shared. Closing one descriptor does not close the underlying open file description while another descriptor still references it.

This distinction explains several common bugs: storing a descriptor after close and accidentally reusing the same integer for a different resource, assuming duplicated descriptors have independent offsets, or conflating path names with open-resource lifetime.

## Unlink and lifetime

On POSIX filesystems, removing a directory entry with unlink does not necessarily destroy file contents immediately. An already-open file can continue to exist until the final reference is closed. This is why temporary files can be opened and unlinked while still being usable by the process.

The exact crash/durability semantics are filesystem/platform concerns and need separate evidence.

## Executable witness

tools/labs/posix_vm_file_lab.cpp creates a temporary file, duplicates its descriptor and proves the shared offset by reading through the two descriptors in sequence. It also unlinks the path while descriptors remain open.

Related: [[Addresses and virtual memory]], [[Atomic publication and durability]].
