# Usage

## Open Data

1. Launch `timeseries_viewer`.
2. Use `Open` in the Sources panel.
3. Select a CSV file or SQLite database.
4. Expand the source tree to browse tables and variables.
5. Select a non-time variable to add it to the active plot.

## Plot Data

- Raw series are plotted in the active plot view.
- Multiple series can be shown together in the same view.
- Tabs are used for separate plot views.

## Create Derived Series

1. Type an expression into the expression field.
2. Use series names such as `series("run1.speed") - series("run2.speed")`.
3. Select `Add derived`.
4. The derived series is added to the current plot view.

## Save A Workspace

1. Use `Save project`.
2. Choose a JSON file.
3. Reopen that file later with `Open project`.

## Live Refresh

- Toggle `Live` to poll sources for changes.
- The current implementation checks for file changes about once per second.
- If a source changes, the app reloads the affected data and keeps the plot layout.

## Product Context

The supported workflows and scope are defined in:

- [01 product](../product-breakdown/01-product/layer.md)
- [00 intent](../product-breakdown/00-intent/layer.md)
