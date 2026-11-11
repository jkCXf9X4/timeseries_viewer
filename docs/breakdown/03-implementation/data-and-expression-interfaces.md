---
id: INFO-031
type: info
title: Data, expression, and persistence interfaces
summary: Normalized import results, stable lookup, serializable bindings, a narrow expression API, and model round-tripping
date: 2026-11-11
status: current
---

# Data, expression, and persistence interfaces

The data, expression, and persistence layers expose narrow contracts.

- **Source import:** returns normalized series metadata plus data vectors.
- **Variable registry:** exposes stable lookup by fully qualified name.
- **Selection layer:** keeps parameter-to-source bindings explicit and serializable.
- **Expression evaluation:** works through a narrow API that exposes named series and returns a derived series.
- **Persistence:** round-trips the same model objects used by the UI.