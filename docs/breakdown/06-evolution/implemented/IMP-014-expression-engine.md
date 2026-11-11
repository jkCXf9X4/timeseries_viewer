---
id: IMP-014
type: imp
title: Extract expression engine
summary: Contain the Lua and sol2 dependency in one module with a narrow named-series-to-derived-series API
date: 2026-11-11
status: implemented
---

# Extract expression engine

- **Question:** how do model, IO, and persistence code avoid linking against Lua while expressions stay testable in isolation?
- **Evidence:** the Lua-based expression engine lives in the monolithic core header alongside model types and IO code, mixing its own dependencies and sandboxing risk (`INFO-025`).
- **Change:** expression code moves to `src/expr/evaluate.hpp`, `src/expr/arithmetic.hpp`, and `src/expr/registry.hpp`, exposing a narrow API that accepts a named-series registry plus an expression string and returns a derived series.
- **Risk:** the module depends on `src/model/` types and sol2 or Lua only, with no access to app state or UI types; sandboxing and expression-language semantics are unchanged.
- **Acceptance:** expression evaluation compiles and works without CSV, SQLite, JSON, or ImGui headers, Lua binding tests pass in isolation, and no file under `src/expr/` includes non-model headers.