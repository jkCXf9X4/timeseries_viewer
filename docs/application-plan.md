# Time Series Viewer Application Plan

## Purpose

Build a standalone Linux desktop application for opening, inspecting, plotting,
comparing, and saving views over multiple time-series data sources.

The first version targets a practical engineering workflow:

- open multiple CSV and SQLite files
- browse files, tables, and variables visually
- plot selected variables live
- compare multiple series in a single plot
- plot simple derived series such as `a - b`
- save and reload view state

## Architectural Decisions

### Runtime and Build

- Language: C++23
- Build system: CMake
- Dependency manager: vcpkg
- Primary platform: Linux
- Target style: standalone desktop application with low operational overhead

The local development environment already has CMake, Ninja, GCC/Clang, and a
vcpkg checkout available, so the project should use a `CMakePresets.json` preset
that points to `$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`.

### User Interface

- GUI: Dear ImGui with docking enabled
- Plotting: ImPlot
- Windowing/backend: GLFW
- File dialogs: nativefiledialog-extended

Dear ImGui and ImPlot are selected because they fit the intended workflow:
dockable internal windows, multiple tabs, fast plotting, compact controls, and
low ceremony for engineering tools.

The application should provide these main UI regions:

- source/file manager
- nested variable browser
- plot workspace with multiple dockable plot views or tabs
- expression editor
- project save/load controls

### Data Sources

Supported source types for v1:

- CSV files
- SQLite databases

CSV assumptions:

- header row is required
- columns are parsed as time, numeric values, or unsupported text
- first numeric or ISO-8601-like column is auto-selected as the time axis
- user can override the detected time column

SQLite assumptions:

- user can browse tables
- selected tables expose columns in the variable browser
- first numeric or ISO-8601-like column is auto-selected as the time axis
- user can override the detected time column

If no valid time column is detected, row index may be used as the x-axis only
after explicit user confirmation.

### Variable Naming

Variables use fully qualified names so that sources can be compared without name
collisions.

Expected forms:

- `source.variable`
- `source.table.variable`

Nested names using `.` are preserved in the browser tree. For example,
`aircraft.engine.rpm` appears as nested nodes but remains addressable by its full
name.

### In-Memory Model

The first version is optimized for medium-sized files. Selected variables are
loaded into memory as typed time series:

- `time[]`
- `value[]`
- metadata such as source id, display name, unit if available, and reload state

The application should not attempt chunked/lazy processing in v1 unless medium
file behavior proves insufficient.

### Live Plotting

Live plotting uses polling.

Behavior:

- live mode checks file modification time and size periodically
- default polling interval: 1 second
- when a source changes, reload selected variables from that source
- active plots and expressions remain attached to their selected variables
- missing or renamed variables are shown as unavailable rather than silently
  removed

This avoids platform-specific file watcher complexity while supporting the main
live-plotting workflow.

### Comparison and Expressions

The plot workspace supports overlaying multiple variables in one view.

Derived plots use embedded Lua:

- Lua runtime: lua
- C++ binding: sol2

The expression interface should expose series lookup and vector arithmetic. A
typical expression can be written as:

```lua
series("run1.speed") - series("run2.speed")
```

Supported v1 expression behavior:

- numeric constants
- `+`, `-`, `*`, `/`
- parentheses and normal Lua composition
- derived result plotted as a normal series
- derived result updates when source data reloads

When expression inputs have different time grids, use the left-hand series as
the reference grid and linearly interpolate the right-hand series. For `a - b`,
evaluate `b` at `a` timestamps.

### Saved Views

Views are saved as project JSON files.

The project file stores:

- source paths and source types
- selected SQLite tables
- selected or detected time columns
- selected variables
- plot views or tabs
- expression definitions
- visual settings such as colors, visibility, and axis range mode

Paths should be stored relative to the project file when possible. Absolute
paths are preserved when a relative path cannot be represented safely.

## Architecture and Module Structure

The codebase is organized into the following modules, matching the architecture
boundaries described above:

### `src/model/` — Canonical Data Types

Defines the shared data model: time series, source metadata, plot state, view
state, and variable registry types. All other modules depend on these types.

### `src/io/csv/` — CSV Source Adapter

Parses CSV files, infers column types (time, numeric, text), and normalizes
columns into the canonical model. Handles header detection and time-column
selection.

### `src/io/sqlite/` — SQLite Source Adapter

Inspects SQLite databases, discovers tables and columns, and normalizes
selected table data into the canonical model. Supports user-overridable time
column detection.

### `src/expr/` — Expression Engine

Wraps the Lua runtime (via sol2) for evaluating derived series expressions.
Provides series lookup helpers and interpolation to align operands on a common
time grid.

### `src/persist/` — Persistence Layer

Serializes and deserializes the full workspace state (sources, plots,
expressions, visual settings) to and from project JSON files using the
nlohmann-json library.

### `src/ui/` — UI Rendering

Contains all Dear ImGui / ImPlot rendering code, split into sub-areas:

- `src/ui/panels/` — side panels (parameter browser, plot inspector)
- `src/ui/plot/` — plot workspace and per-tab rendering
- `src/ui/backend/` — ImGui backend integration (GLFW window, context setup)

### `src/app/` — Application Model

Holds the application-level state and orchestration logic, split into
sub-modules:

- `src/app/windows/` — window and tab ownership
- `src/app/selection/` — parameter binding and source-to-variable mapping
- `src/app/cache/` — in-memory time-series cache
- `src/app/source/` — source lifecycle and refresh scheduling
- `src/app/project/` — project open/save orchestration

This structure ensures that source ingestion, expression evaluation,
persistence, and UI rendering remain decoupled and independently testable.

## Initial Dependency Set

Expected vcpkg dependencies:

- `imgui`
- `implot`
- `glfw3`
- `sqlite3`
- `fast-cpp-csv-parser`
- `lua`
- `sol2`
- `nlohmann-json`
- `nativefiledialog-extended`

## Test and Verification Plan

Automated tests should cover:

- CSV import with numeric and date-like time columns
- SQLite table and column import
- time-column auto-detection
- user-selected time-column override
- nested variable tree construction from dotted names
- project JSON save/load round-trip
- interpolation between different time grids
- Lua expression evaluation for arithmetic expressions
- handling of missing variables after reload

Integration fixtures should include at least:

- one CSV file with several numeric variables
- one SQLite database with at least two tables
- two sources with overlapping but non-identical time grids

Manual smoke test:

```sh
cmake --preset=vcpkg
cmake --build build
./build/timeseries_viewer
```

Manual acceptance scenarios:

- open multiple CSV and SQLite sources
- select variables through the nested browser
- overlay multiple series in one plot
- create a derived `a - b` plot
- enable live mode and observe source changes
- save a project file
- reopen the project and verify the same views are restored

## Open Risks

- Dear ImGui gives an engineering-tool UI rather than a traditional native
  widget look.
- Medium-file in-memory loading may need revisiting for very large data sets.
- Lua expression ergonomics may need a small helper API once real users try it.
- Time parsing and unit metadata conventions may vary across input files.

## Decisions Recorded

- Use C++ instead of Python or Rust.
- Use vcpkg-managed dependencies.
- Use Dear ImGui and ImPlot for the UI and plotting layer.
- Use polling for live updates.
- Use project JSON files for saved views.
- Use Lua for expression-based derived series.
- Use interpolation to align comparison series on the left-hand time grid.

## Traceability

- [Product breakdown](../product-breakdown/README.md)
