# 03 Implementation

## How The Product Is Built

The codebase should be organized around the same separation used in the architecture: app shell, source import, normalized series data, expressions, persistence, and tests.

## Proposed Code Structure

- `src/app/`: application startup, main loop, routing between UI panels
- `src/app/windows/`: top-level window and tab ownership
- `src/app/selection/`: parameter binding menu and source-to-parameter mapping
- `src/ui/`: fixed parameter sidebar, plot inspector, plot workspace, expression editor, project dialogs
- `src/io/csv/`: CSV parsing and column inference
- `src/io/sqlite/`: SQLite inspection and table/column import
- `src/model/`: canonical series, source metadata, plot state, view state
- `src/expr/`: Lua integration, series lookup helpers, interpolation support
- `src/persist/`: project JSON schema and serialization
- `tests/`: import, expression, persistence, and workflow tests

## Implementation Interfaces

- Source import should return normalized series metadata plus data vectors.
- The variable registry should expose stable lookup by fully qualified name.
- The selection layer should keep parameter-to-source bindings explicit and serializable.
- The window/tab layer should own exactly one plot per tab and keep per-tab parameter state isolated.
- The UI shell should pin the parameter browser to the left and the plot inspector to the right while keeping both panels tied to the active plot.
- The plot layer should consume ready-to-render series objects rather than raw files.
- Expression evaluation should work through a narrow API that exposes named series and returns a derived series.
- Persistence should round-trip the same model objects used by the UI.

## Configuration And Environment

- C++23 is the target language level.
- CMake is the build system.
- vcpkg is the dependency source of record.
- The Linux build environment should assume a local vcpkg toolchain and a desktop graphics stack.

## Implementation Decisions

- PB-002: Use C++23, CMake, and vcpkg as the implementation stack.

## Implementation Constraints

- Keep the first implementation focused on the product scope from `01-product`.
- Avoid introducing abstractions that are not needed by CSV/SQLite import, plotting, or expressions.
- Keep source-specific logic behind adapters so the UI and plot layers remain format-agnostic.

## Implementation Risks

- If the codebase grows quickly, the model layer can become a dumping ground unless boundaries stay explicit.
- Expression helpers need good naming and stable lookup semantics to avoid hidden coupling.
- Project schema changes must remain backward-readable once saved views exist in the wild.
