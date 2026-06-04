# Verification

## Automated Tests

Run the Catch2 suite with:

```sh
ctest --test-dir build --output-on-failure
```

Current coverage includes:

- CSV import and time-column inference
- SQLite table discovery and import
- nested variable tree construction
- interpolation between differing time grids
- Lua expression evaluation for derived series
- project JSON save/load round-trip
- missing-variable behavior after reload

## Manual Acceptance

Verify the application can:

- open more than one source
- select variables visually
- plot multiple series in one view
- create a derived `a - b` trace
- save and reopen a project
- refresh when live mode is enabled

## Test Files

- `tests/core_tests.cpp` — Core data types, CSV/SQLite loading, expression engine
- `tests/app_model_tests.cpp` — App model behavior (using `GuiHarness`)
- `tests/gui_render_tests.cpp` — UI rendering (using `GuiHarness`)

The `GuiHarness` test support class provides a simulated UI backend for testing app model and rendering logic without a real display.

## Test Fixtures

- `tests/fixtures/sample_numeric.csv`
- `tests/fixtures/sample_datetime.csv`

## Product Context

Acceptance behavior is defined in:

- [04 verification](../product-breakdown/04-verification/layer.md)
