# 02 Architecture

## How The Product Is Structured

The architecture should separate concerns into source ingestion, normalized time-series state, expression evaluation, view state, and UI rendering.

## Main Boundaries

- UI shell and docking layout
- Source adapters for CSV and SQLite
- Canonical variable registry
- Time-series cache and derived-series model
- Expression engine
- Project persistence
- Background refresh and invalidation

## Components

### UI Shell

Provides the dockable workspace, source browser, plot tabs, expression editor, and project actions.

### Source Adapters

Normalize CSV and SQLite inputs into a common internal representation so plotting and expressions do not care about the original source format.

### Variable Registry

Maps fully qualified names to source-local series objects and preserves dotted hierarchy for the browser tree.

### Plot Workspace

Owns overlays, axes, visibility, styling, and multiple tabs or views.

### Expression Engine

Evaluates derived series from named inputs and produces a normal plottable result.

### Persistence Layer

Serializes and restores the workspace, source selections, plot layout, and expressions.

### Refresh Scheduler

Polls sources for changes and invalidates or reloads affected series when live mode is enabled.

## Data Flow

1. User opens one or more sources.
2. Source adapters extract columns or tables and normalize them.
3. The variable registry exposes selectable series in a tree.
4. The user adds series or expressions to one or more plot views.
5. The plot workspace renders raw and derived series together.
6. The refresh scheduler reloads changed sources and updates dependent plots.
7. Project persistence stores and restores the current state.

## Quality Attributes

- Low dependency footprint.
- Predictable behavior on Linux desktop environments.
- Responsive interaction for medium-sized datasets.
- Clear recovery when data changes or a variable becomes unavailable.
- Traceable state restoration for analysis sessions.

## Stable Decisions

- PB-003: Dear ImGui + ImPlot + GLFW are the UI/plotting foundation.
- PB-005: Load selected variables into memory for medium-file operation.
- PB-006: Use polling for live updates.
- PB-007: Use embedded Lua with `sol2` bindings for expressions.
- PB-008: Use interpolation onto the left-hand time grid for comparison and derived series.

## Architecture Risks

- In-memory loading may not scale to very large sources without later redesign.
- Polling is simple but less immediate than file notifications.
- Expression evaluation needs a controlled interface so results stay deterministic and explainable.
