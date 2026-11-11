---
id: INFO-027
type: info
title: Build approach
summary: The codebase is organized around the same separation as the architecture: shell, import, data, expressions, persistence, tests
date: 2026-11-11
status: current
---

# Build approach

The codebase is organized around the same separation used in the architecture.

- **Organization:** app shell, source import, normalized series data, expressions, persistence, and tests.
- **Mirror:** the code structure mirrors the architecture separation.
- **Modules:** the app is split into model, io, expr, persist, ui, and app modules.