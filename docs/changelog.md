# Change History

## Unreleased

- Initial implementation of the standalone Linux time series viewer.
- CSV and SQLite source loading.
- Nested variable browsing.
- Live plotting with polling refresh.
- Derived series via Lua expressions.
- Project JSON save/load.
- Catch2 test suite covering the verification plan.

### Module Restructuring (IMP-011 through IMP-018)

- **IMP-011**: Extracted canonical model types into `src/model/` (types, config, registry, equality, utility).
- **IMP-012**: Extracted CSV source adapter into `src/io/csv/` (catalog loading, series streaming).
- **IMP-013**: Extracted SQLite source adapter into `src/io/sqlite/` (table discovery, column inspection, series loading).
- **IMP-014**: Extracted expression engine into `src/expr/` (Lua/sol2 evaluation, series arithmetic, interpolation).
- **IMP-015**: Extracted persistence layer into `src/persist/` (JSON serialization, project save/load, path resolution).
- **IMP-016**: Restructured app model into domain sub-modules (`src/app/windows/`, `selection/`, `cache/`, `source/`, `project/`).
- **IMP-017**: Isolated UI rendering from app model into `src/ui/` (panels, plot workspace, top-level render).
- **IMP-018**: Extracted ImGui backend from `main.cpp` into `src/ui/backend/imgui_backend.hpp`.

## Product Context

Future changes and evolution direction are tracked in:

- [06 evolution](../product-breakdown/06-evolution/layer.md)
