---
id: IMP-011
type: imp
title: Extract canonical model types
summary: Split the monolithic core header into a pure model layer with no I/O, expression, or persistence dependencies
date: 2026-11-11
status: implemented
---

# Extract canonical model types

- **Question:** how do model types compile without pulling in CSV, SQLite, Lua, or JSON dependencies?
- **Evidence:** the core header is a monolithic 1270-line file mixing model types with IO, expression, and persistence code, forcing recompilation and obscuring the type hierarchy (`INFO-029`).
- **Change:** model types split into `src/model/types.hpp`, `src/model/registry.hpp`, `src/model/config.hpp`, `src/model/equality.hpp`, and `src/model/utility.hpp`; the old core header becomes a forwarding header.
- **Risk:** the change is surgical, staying C++23 standard-library-only with no third-party headers reachable from `src/model/`.
- **Acceptance:** all model types compile without third-party headers, the core tests pass after include-path migration, and no CSV, SQLite, Lua, JSON, or ImGui header is reachable from any file under `src/model/`.