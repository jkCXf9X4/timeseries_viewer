---
id: INFO-029
type: info
title: Data and I/O code structure
summary: Source layout for CSV and SQLite import, the canonical model, expressions, persistence, and tests
date: 2026-11-11
status: current
---

# Data and I/O code structure

The data, I/O, expression, and persistence code is split into focused source directories.

- **`src/io/csv/`:** CSV parsing and column inference.
- **`src/io/sqlite/`:** SQLite inspection and table and column import.
- **`src/model/`:** canonical series, source metadata, plot state, and view state.
- **`src/expr/`:** Lua integration, series lookup helpers, and interpolation support.
- **`src/persist/`:** project JSON schema and serialization.
- **`tests/`:** import, expression, persistence, and workflow tests.