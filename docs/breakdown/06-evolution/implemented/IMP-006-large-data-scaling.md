---
id: IMP-006
type: imp
title: Large-data scaling strategy
summary: Define the dataset size envelope and the implementation path for the in-memory model
date: 2026-11-11
status: implemented
---

# Large-data scaling strategy

- **Question:** what is the practical dataset envelope of the in-memory model and how does the product scale beyond it?
- **Evidence:** selected series load into memory, which is appropriate for medium datasets but not very large archives (`INFO-017`, `INFO-049`).
- **Change:** the product has an explicit scaling strategy and documented threshold, with cache, chunking, or lazy-loading options identified while preserving the current UX for small and medium files.
- **Risk:** converting the product into a database server or adding distributed storage is out of scope.
- **Acceptance:** the scaling strategy and threshold are documented, the implementation path does not break the existing project format, and large-file behavior is predictable and explainable.