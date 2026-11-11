<!-- GENERATED FILE — do not edit. Regenerate with `pb registers`. -->

# Decision Stream

Flat, dated history of every committed design choice. Current state lives
in the layer leaves named by each record's `state:` field.

Records live in the cold-storage archive; extract one with `pb archive read <ID>`.

## By Layer

### Intent

- **PB-001** — Linux-first standalone desktop application · accepted · 2026-11-11 · `PB-001`
  The viewer is a Linux-first, standalone desktop application with local-file workflows

### Product

- **PB-004** — CSV and SQLite initial sources with overridable time selection · accepted · 2026-11-11 · `PB-004`
  CSV and SQLite are the initial source types, with user-overridable time selection
- **PB-009** — Saved views as project JSON files · accepted · 2026-11-11 · `PB-009`
  Saved views are project JSON files that restore sources, plots, and expressions

### Architecture

- **PB-003** — Dear ImGui, ImPlot, and GLFW UI foundation · accepted · 2026-11-11 · `PB-003`
  Dear ImGui, ImPlot, and GLFW are the UI and plotting foundation
- **PB-005** — In-memory loading for medium files · accepted · 2026-11-11 · `PB-005`
  Selected variables load into memory for medium-file operation
- **PB-006** — Polling for live updates · accepted · 2026-11-11 · `PB-006`
  Live updates use polling rather than platform file watchers
- **PB-007** — Embedded Lua with sol2 bindings for expressions · accepted · 2026-11-11 · `PB-007`
  Derived series use embedded Lua with sol2 bindings
- **PB-008** — Left-hand time grid interpolation · accepted · 2026-11-11 · `PB-008`
  Cross-series comparison aligns to the left-hand time grid using interpolation
- **PB-010** — Explicit parameter-to-source binding · accepted · 2026-11-11 · `PB-010`
  Parameter selection is explicit and bound to the open source or database that provides the series
- **PB-011** — Multi-window tabbed layout with one plot per tab · accepted · 2026-11-11 · `PB-011`
  The top-level layout uses simultaneous windows, multiple tabs per window, and exactly one plot per tab
- **PB-012** — Fixed side panels following the active plot · accepted · 2026-11-11 · `PB-012`
  The parameter browser sits fixed left and the plot inspector fixed right, and both follow the active plot

### Implementation

- **PB-002** — C++23, CMake, and vcpkg implementation stack · accepted · 2026-11-11 · `PB-002`
  The implementation stack is C++23, CMake, and vcpkg as the dependency source of record
