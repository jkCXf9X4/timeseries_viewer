---
id: IMP-015
type: imp
title: Extract persistence layer
summary: Keep JSON serialization and project schema logic in one place with backward-readable schema evolution
date: 2026-11-11
status: implemented
---

# Extract persistence layer

- **Question:** how does project JSON serialization and schema logic stay separate from model types and source adapters?
- **Evidence:** JSON serialization for all config types and project save and load live in the monolithic core header, mixing serialization concerns with unrelated code (`INFO-031`, `INFO-034`).
- **Change:** persistence moves to `src/persist/save.hpp`, `src/persist/load.hpp`, and `src/persist/json_helpers.hpp`, containing all `to_json` and `from_json` specializations for config types.
- **Risk:** the module depends on `src/model/` types and `nlohmann_json` only, and schema changes stay backward-readable.
- **Acceptance:** project round-trip works without CSV, SQLite, Lua, or ImGui headers, the legacy pre-schema project format still loads, and no file under `src/persist/` includes non-model headers.