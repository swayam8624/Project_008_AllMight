---
tags: [visual-recall, systems, emulator, operating-systems, networking, security]
coverage: "Phase 28; M163–M168; Parts 2701–2800 (authored reconstruction)"
companion: "Systems — From Cartridge Bytes to Reproducible State"
---

# Systems recall — one snapshot from executed byte to trusted replay

This is a reconstruction companion, not a replacement for [[Continuous Notes/Systems - From Cartridge Bytes to Reproducible State|the continuous systems teaching]]. Start with the user loading a cartridge, making a snapshot and attempting a remote restore. Follow causal operations in order and reconstruct the reason for every byte and lifetime boundary.

## Rebuild the emulated CPU

Given an NROM image containing one 16-KiB PRG bank and a reset vector pointing to $8000, trace the file/header offsets, PRG bank mapping to CPU $8000/$C000, the bytes read at $FFFC/$FFFD, the PC after reset, and the first opcode fetch. Explain why a raw 65536-byte array cannot replicate PPU/controller/mapper I/O side effects. Define the minimal state machine S(t+1)=T(S(t),input(t)) and identify the missing state if only A/X/Y/PC are saved.

Trace ADC $7F+$01 with C=0. The 8-bit result is $80, carry is clear, signed overflow and negative are set. Trace $FF+$01: $00 with carry and zero set but signed overflow clear. Derive the overflow formula from operand/result sign bits rather than memorizing it. Then decode BNE relative: the signed 8-bit displacement is added to PC after the operand is fetched; a taken branch incurs a cycle and a page crossing may incur another.

Explain why a controller shift register is stateful I/O, why a simplified OAM page copy does not prove 513/514-cycle DMA timing, and why accumulating 3 PPU dots per CPU tick cannot implement VBlank, odd-frame skip or sprite evaluation.

Reproduce the [[tools/labs/phase28_emulator_core_lab.cpp|C++20 CPU oracle]] and identify its explicitly unsupported behavior before claiming any emulator accuracy beyond the tested subset.

## Rebuild a safe state transfer

Design a versioned save-state record without writing native struct bytes. Describe each field's representation and endian convention, reject malformed version/length before mutation, and name all missing device states needed for complete replay. Show how an atomic temporary-file publication differs from a stream flush and from crash durability. Include the file/directory sync policy for the chosen platform and a plan for interrupted/short writes.

Define a length-prefixed TCP frame parser whose length bytes arrive separately. Prove it does not read uninitialized data or allocate more than the maximum permitted message size. Explain why UDP can drop, reorder or duplicate datagrams while TCP has no application message boundaries. Draw the timeline of a predicted input tick, a delayed authoritative acknowledgment and deterministic replay of unacknowledged commands. State the exact source of divergence if the client and server physics use inconsistent iteration order.

## Rebuild the trust boundary

Distinguish SHA-256, HMAC and authenticated encryption in terms of the problem each solves. Write the symbolic HKDF extract/expand recurrence and name the context separation role of info. Explain why a digest displayed alongside a file cannot prove its origin when an attacker can replace both.

Derive why an AES-GCM nonce must remain unique for a key even across process restarts. Give a failure-injection plan for corrupt ciphertext, wrong AAD, truncated tags, replay and crash during nonce-counter persistence. Do not invent a homebrew encryption implementation; identify the cryptographic library and API contracts that would be verified first.

For a Shamir threshold (t,n) scheme, express the random polynomial over a finite field and derive reconstruction of the constant coefficient at x=0. Explain why fewer than t shares are information-theoretically insufficient only under the stated randomness assumptions. For a Merkle inclusion proof, list the sibling hashes/positions required to reconstruct a trusted root and explain why a valid proof says nothing about whether the root itself is authentic.

Cross Rust/Python/native boundaries with a pointer: name its owner, lifetime, allocation/deallocation domain, thread restrictions and error/panic conversion. Then create an allow/redact/deny policy for a remote snapshot and specify what information is preserved in an audit log without leaking secrets.

## Evidence and mastery gate

Do not mark all 100 parts as executed just because the CPU lab compiles. M163–M164 have a constrained executable CPU/bus/NROM witness. M165–M168 have connected explanation and primary-source references, but still require filesystem, network, established-cryptography and FFI integration tests. A complete mastery demonstration reconstructs and falsifies the entire byte-to-trusted-replay pipeline under partial failures, not merely a successful load of one synthetic ROM.
