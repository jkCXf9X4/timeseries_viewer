---
id: IMP-033
type: imp
title: Chunked and lazy loading for large datasets
summary: Load series lazily or in chunks so the GUI stays responsive and large files open
date: 2026-11-11
status: suggested
---

# Chunked and lazy loading for large datasets

- **Question:** how does the GUI stay responsive and how do large files open when series load fully into memory?
- **Evidence:** selected series load into memory for medium-file operation, which is the ceiling for large archives and makes the GUI heavy (`INFO-017`, `INFO-049`).
- **Change:** series load lazily or in chunks so the GUI stays snappier and large files can open, while the UX for small and medium files is preserved.
- **Risk:** a revised internal data model may be required; the existing project format must not break.
- **Acceptance:** large files open with a responsive GUI, the loading strategy is documented, and small and medium-file behavior is unchanged.