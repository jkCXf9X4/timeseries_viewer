# Contributing

## Development Loop

1. Configure with `cmake --preset=vcpkg`.
2. Build with `cmake --build build`.
3. Run tests with `ctest --test-dir build --output-on-failure`.
4. Make the smallest change that addresses the current behavior gap.

## Project Layout

- `src/model/`: core data types, config, registry, equality operators, and utility functions
- `src/io/csv/`: CSV source adapter (catalog loading, series streaming)
- `src/io/sqlite/`: SQLite source adapter (table discovery, column inspection, series loading)
- `src/expr/`: expression engine (Lua/sol2 evaluation, series arithmetic, interpolation)
- `src/persist/`: persistence layer (JSON serialization, project save/load, path resolution)
- `src/ui/`: UI rendering (panels/, plot/, backend/)
- `src/app/`: app model (windows/, selection/, cache/, source/, project/)
- `src/timeseries_viewer/app_model.hpp`: unified public header for the app model
- `src/timeseries_viewer/core.hpp`: legacy forwarding header (being phased out)
- `src/app/main.cpp`: GUI shell and application flow
- `src/third_party/imgui/backends/`: vendored Dear ImGui backend glue
- `tests/core_tests.cpp`: Catch2 coverage for core data types, CSV/SQLite loading, expression engine
- `tests/app_model_tests.cpp`: Catch2 coverage for app model behavior (using `GuiHarness`)
- `tests/gui_render_tests.cpp`: Catch2 coverage for UI rendering (using `GuiHarness`)

## Adding Features

- Put stable product decisions in `product-breakdown/`.
- Put runnable commands and procedures in `docs/`.
- Add or extend tests alongside every behavior change.
- Keep source-specific logic behind the core layer instead of leaking it into the UI.

## Product Context

Implementation boundaries are captured in:

- [03 implementation](../product-breakdown/03-implementation/layer.md)
- [04 verification](../product-breakdown/04-verification/layer.md)
