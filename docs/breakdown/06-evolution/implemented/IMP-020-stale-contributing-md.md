---
id: IMP-020
type: imp
title: Update contributing guide for module structure
summary: Replace the monolithic project layout description with the current module layout and test files
date: 2026-11-11
status: implemented
---

# Update contributing guide for module structure

- **Question:** how does the contributing guide describe the current source layout after the module extraction?
- **Evidence:** the contributing guide describes a monolithic core header and a single test file, but the codebase is restructured into module directories and three test files (`INFO-028`, `INFO-029`).
- **Change:** the project layout section lists the module directories and all three test files, and describes the old core header as a forwarding header.
- **Risk:** edits stay confined to the contributing guide so code and other docs are untouched.
- **Acceptance:** the layout section matches the actual source directory structure, all three test files are listed, and no stale file paths remain.