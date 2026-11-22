# EPUB cross-publication link probes

These are disposable conformance probes retained from builder development; they are not production EPUBs.

- `relative-cross-link-file-uri.epub`: copied from the Stage 1 build and changed sibling EPUB `href` values to `file:` URIs. EPUBCheck 5.4.0 rejected the file URLs (RSC-030) and reported unresolved package resources (RSC-007).
- `custom-scheme-probe.epub`: copied from the Stage 1 build and changed sibling EPUB `href` values to the `allmight:` scheme. EPUBCheck did not resolve this as a portable destination and reported MathML errors before the tag whitelist correction. It is retained only to document the rejected probe.

Both copies were based on a pre-final attempt using `.generation-n3azv4es`; neither is part of `Books/Ebooks` delivery.
