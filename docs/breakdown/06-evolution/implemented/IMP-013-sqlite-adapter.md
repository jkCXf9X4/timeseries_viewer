---
id: IMP-013
type: imp
title: Extract SQLite source adapter
summary: Encapsulate SQLite database handling, including connection and statement lifetime, behind a clean adapter boundary
date: 2026-11-11
status: implemented
---

# Extract SQLite source adapter

- **Question:** how does SQLite-specific database handling live behind a clean adapter boundary?
- **Evidence:** SQLite logic lives in the monolithic core header interleaved with model types, expressions, and JSON helpers, with error handling that must not leak into other modules (`INFO-024`).
- **Change:** SQLite logic moves to `src/io/sqlite/catalog.hpp` and `src/io/sqlite/load.hpp`, with connection and statement lifetime encapsulated within the module.
- **Risk:** the adapter may depend on `src/model/` types and the SQLite3 library but must not depend on CSV, expression, persistence, or ImGui code.
- **Acceptance:** all SQLite loading functions compile and tests pass without CSV, Lua, or JSON headers, no file under `src/io/sqlite/` includes non-model headers, and database and statement lifetime is managed entirely within the module.