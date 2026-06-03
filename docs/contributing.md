# Contributing

## Development Loop

1. Configure with `cmake --preset=vcpkg`.
2. Build with `cmake --build build`.
3. Run tests with `ctest --test-dir build --output-on-failure`.
4. Make the smallest change that addresses the current behavior gap.

## Project Layout

- `src/timeseries_viewer/core.hpp`: core data model, import, expression, and persistence helpers
- `src/app/main.cpp`: GUI shell and application flow
- `src/third_party/imgui/backends/`: vendored Dear ImGui backend glue
- `tests/core_tests.cpp`: Catch2 coverage for the verification plan

## Adding Features

- Put stable product decisions in `product-breakdown/`.
- Put runnable commands and procedures in `docs/`.
- Add or extend tests alongside every behavior change.
- Keep source-specific logic behind the core layer instead of leaking it into the UI.

## Product Context

Implementation boundaries are captured in:

- [03 implementation](../product-breakdown/03-implementation/layer.md)
- [04 verification](../product-breakdown/04-verification/layer.md)
