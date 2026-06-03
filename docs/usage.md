# Usage

## Open Data

1. Launch `timeseries_viewer`.
2. Use `Open` in the Parameters panel.
3. Select a CSV file or SQLite database.
4. Expand the source tree to browse tables and variables.
5. Use the dotted-name tree to drill into nested variable groups.
6. Tick the checkbox next to a non-time variable to add it to the active plot tab.
7. Clear the checkbox to remove that variable from the active plot tab.
8. Rebind an existing series from the selected-series controls in the plot editor when you need a different source or column.
9. Use the left Parameters panel for source selection and the right Plot Inspector panel for plot settings, point budget, and series details; the inspector shows the active plot status fields.
10. Click a window or tab to make it the active plot for both side panels.

## Compare Sources

1. Open a second CSV file or SQLite database.
2. Add a comparable variable from that source to the same plot tab.
3. Inspect both series together in one plot.
4. Add additional series as needed to compare multiple runs or databases.
5. Use `New window` and `New tab` to create separate analysis contexts when needed.
6. Use the active-series list in the plot tab to pick which series should be edited or rebound.

## Plot Data

- Raw series are plotted in the active plot tab.
- Multiple series can be shown together in the same plot.
- Each analysis window can hold multiple tabs.
- Each tab contains exactly one plot and its own parameter selection.
- The left sidebar is the fixed Parameters panel.
- The right sidebar is the fixed Plot Inspector panel.

## Create Derived Series

1. Type an expression into the tab-local expression field.
2. Use series names such as `series("run1.speed") - series("run2.speed")`.
3. Select `Add derived`.
4. The derived series is added to the current plot tab.

## Save A Workspace

1. Use `Save project`.
2. Choose a JSON file.
3. Reopen that file later with `Open project`.

## Live Refresh

- Toggle `Live` to poll sources for changes.
- The current implementation checks for file changes about once per second.
- If a source changes, the app reloads the affected data and keeps the plot layout.

## Large Data

- Use `Point budget` in the Plot Inspector to cap the number of plotted samples loaded per selected series.
- `0` disables the explicit cap and leaves the loader unbounded.
- The app keeps a cache of raw series bindings so rebinding and redraws avoid unnecessary reloads when the source file has not changed.

## Product Context

The supported workflows and scope are defined in:

- [01 product](../product-breakdown/01-product/layer.md)
- [00 intent](../product-breakdown/00-intent/layer.md)
