---
id: IMP-012
type: imp
title: Extract CSV source adapter
summary: Isolate CSV parsing behind its own adapter so format changes cannot affect other code
date: 2026-11-11
status: implemented
---

# Extract CSV source adapter

- **Question:** how does CSV-specific parsing live behind a clean adapter boundary?
- **Evidence:** CSV parsing logic lives in the monolithic core header interleaved with SQLite, JSON, and expression code (`INFO-024`).
- **Change:** CSV logic moves to `src/io/csv/catalog.hpp` and `src/io/csv/load.hpp`, with a public API accepting a file path and column metadata and returning normalized series data.
- **Risk:** the adapter may depend on `src/model/` types only and must not depend on SQLite, expression, persistence, or ImGui code.
- **Acceptance:** CSV load functions compile and pass without SQLite, Lua, or JSON headers in scope, CSV tests run independently of the SQLite and Lua suites, and no file under `src/io/csv/` includes non-model headers.