<!-- GENERATED FILE — do not edit. Regenerate with `pb registers`. -->

# Traceability Map

Generated from record front-matter: each decision names the leaf that
holds its outcome (`state:`). The breakdown never references a record —
the leaf is the source of truth; the record is how the state got there.

### Intent

- **PB-001** — State: `00-intent/constraints.md` (Intent constraints) · Artifacts: `src/app/main.cpp`, `CMakeLists.txt`

### Product

- **PB-004** — State: `01-product/source-capabilities.md` (Source and variable capabilities) · Artifacts: `src/io/csv/csv.hpp`, `src/io/sqlite/sqlite.hpp`
- **PB-009** — State: `01-product/workspace-capabilities.md` (Workspace and plot capabilities) · Artifacts: `src/persist/persist.hpp`

### Architecture

- **PB-003** — State: `02-architecture/components/ui-shell.md` (UI shell component) · Artifacts: `src/ui/backend/imgui_backend.hpp`, `src/ui/backend/imgui_backend.cpp`
- **PB-005** — State: `02-architecture/structure.md` (Architecture structure) · Artifacts: `src/app/cache.cpp`
- **PB-006** — State: `02-architecture/components/refresh-scheduler.md` (Refresh scheduler component) · Artifacts: `src/app/source.cpp`
- **PB-007** — State: `02-architecture/components/registry-data-expression.md` (Registry, expression, and persistence components) · Artifacts: `src/expr/expr.hpp`
- **PB-008** — State: `02-architecture/components/registry-data-expression.md` (Registry, expression, and persistence components) · Artifacts: `src/expr/expr.hpp`
- **PB-010** — State: `02-architecture/components/binding-and-sources.md` (Parameter binding and source adapter components) · Artifacts: `src/app/selection/binding.cpp`, `src/ui/panels/parameter.hpp`
- **PB-011** — State: `02-architecture/components/window-and-plot.md` (Window manager and plot workspace components) · Artifacts: `src/app/windows/workspace.cpp`
- **PB-012** — State: `01-product/ui-description/visual-layout.md` (Visual layout) · Artifacts: `src/ui/panels/parameter.hpp`, `src/ui/panels/inspector.hpp`, `src/ui/plot/workspace.hpp`

### Implementation

- **PB-002** — State: `03-implementation/configuration-and-environment.md` (Configuration and environment) · Artifacts: `CMakeLists.txt`, `vcpkg.json`
