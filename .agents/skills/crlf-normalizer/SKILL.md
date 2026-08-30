---
name: crlf-normalizer
description: Normalize newline characters in text files to CRLF
license: MIT
compatibility: opencode
metadata:
  audience: developers
  workflow: filesystem
  entrypoint: crlf-normalizer.py
---

## What I do

- Expand glob patterns from `--input`
- Read UTF-8 text files and normalize all newline characters to CRLF
- Safely overwrite files using atomic replace
- Leave files untouched if conversion fails

## When to use me

Use this when you need to ensure consistent CRLF newlines across multiple text files.

This is helpful for:
- Preparing files for Windows-based tooling
- Enforcing consistent newline formatting in repositories
- Cleaning up mixed newline styles before committing

If the target files or glob patterns are ambiguous, ask for clarification before proceeding.
