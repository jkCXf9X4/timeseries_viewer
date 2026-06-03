# UI Description

## Purpose

This document is the design reference for the application user interface. It describes the visible layout, interaction rules, panel responsibilities, and focus behavior that the implementation should preserve.

It is not a usage guide. It exists so design decisions remain consistent while the UI evolves.

## Visual Layout

The interface is a three-region desktop workspace:

```text
+----------------------+-------------------------------+----------------------+
| Left sidebar         | Center analysis workspace      | Right sidebar        |
| Parameters           | Windows -> tabs -> one plot    | Plot Inspector       |
| fixed height         | resizable central area         | fixed height         |
| width resizable      | multiple windows and tabs      | width resizable      |
+----------------------+-------------------------------+----------------------+
```

### Left Sidebar

- Anchored to the left edge of the application.
- Stretches from the top to the bottom of the viewport.
- Width can be resized by the user.
- Position is fixed. It does not float and cannot be moved to another side.
- Hosts source management and parameter selection.

### Center Workspace

- Occupies the remaining space between the two sidebars.
- Contains the active analysis windows.
- Can show multiple windows at the same time.
- Each window can contain multiple tabs.
- Each tab contains exactly one plot.

### Right Sidebar

- Anchored to the right edge of the application.
- Stretches from the top to the bottom of the viewport.
- Width can be resized by the user.
- Position is fixed. It does not float and cannot be moved to another side.
- Hosts plot settings, plot status, and series editing for the active plot.

## Left Sidebar Behavior

The left sidebar is the parameter browser and source control area.

### Main Responsibilities

- Open CSV files and SQLite databases.
- Reload sources.
- Toggle live refresh.
- Open and save project files.
- Add and remove variables from the active plot using tickboxes.
- Show source trees built from dotted variable names.

### Source Browser

- Sources are grouped by open file or database.
- Each source expands into a tree based on `.` separators in variable names.
- Parent nodes are navigation only.
- Leaf nodes represent bindable variables.
- Leaf nodes use tickboxes for add/remove behavior.
- The browser must make the source of a variable clear enough to avoid ambiguous selection.

### Active Plot Coupling

- The parameter browser always applies to the currently active plot context.
- It does not duplicate the full plot summary that appears in the right sidebar.
- The active plot context may change when the user clicks another window or tab.

## Center Workspace Behavior

The center workspace is the plot surface and analysis area.

### Windows

- More than one analysis window may be visible simultaneously.
- A window is a top-level analysis context inside the application window.
- Each analysis window has its own tab set.
- Clicking a window makes it the active analysis window.

### Tabs

- Each tab contains exactly one plot.
- Tabs belong to one analysis window only.
- Clicking a tab makes it the active plot.
- Tab titles are editable from the plot inspector.
- The active tab determines which plot is affected by side-panel edits.

### Plot Surface

- The plot surface shows the plot only.
- Plot settings are not edited inside the plot body.
- The plot surface may contain only plot-specific visual output, not the full inspector UI.
- Raw series and derived series may be overlaid in the same plot.

## Right Sidebar Behavior

The right sidebar is the plot inspector for the active plot.

### Main Responsibilities

- Show the active plot summary.
- Edit tab-level plot settings.
- Edit series settings for the selected series.
- Add derived series to the active tab.
- Show source and binding information for the selected series.

### Active Plot Summary

- Window title.
- Tab title.
- Selected series name when a series is active.
- The right sidebar is the only place where the active plot summary is shown.

### Plot Settings

- Tab title.
- Autoscale toggles for X and Y axes.
- Manual X and Y range entry when autoscale is off.
- Point budget for large series.

### Series List

- Lists the series attached to the active tab.
- Each series can be selected for editing.
- Each series can be shown or hidden.
- Each series can be recolored.
- Each series can be removed.
- Raw series should expose their source alias and binding details.
- Derived series should expose their expression.

### Selected Series Editor

- Shows the currently selected series in detail.
- Allows editing the series name.
- Allows visibility and color changes.
- Shows binding details for raw series.
- Shows expression editing for derived series.
- Allows removal of the selected series.

### Derived Series Controls

- Provides an expression draft for the active tab.
- Allows creating a new derived series from that draft.
- Derived series are scoped to the active tab, not globally shared across the whole app.

## Focus And Activation Rules

- The active analysis window follows focus.
- The active plot follows the clicked tab or focused window.
- The plot inspector always reflects the current active plot.
- The parameter browser selection always targets the current active plot.
- The user does not need a separate "active window" selector.

## Dialog Behavior

- File dialogs must appear in front of the application, not behind it.
- Open and save dialogs should be parented to the main application window.
- Dialogs are launched from the app shell, not from the plot body.

## Empty And Error States

### No Sources Open

- The parameter browser should explain that a source must be opened before parameters can be selected.
- The plot inspector should explain that an open source is required for meaningful plot editing.

### No Series Selected

- The plot inspector should still show the active plot summary and plot settings.
- The selected-series editor should show that no series is selected.

### Missing Series

- If a project reload references a variable that is no longer available, the UI should surface that clearly.
- Missing data should not silently disappear from the workspace state.

## Interaction Summary

1. Open one or more sources.
2. Browse source trees in the left sidebar.
3. Check variables to add them to the active tab.
4. Click a window or tab to change the active plot.
5. Edit the active plot in the right sidebar.
6. Add derived series from the active tab when needed.
7. Save the workspace as a project file.

## Design Constraints

- The left and right sidebars are fixed to their sides of the application.
- The sidebars span the full viewport height.
- Their widths are adjustable.
- The workspace in the middle must remain usable as the viewport changes size.
- The inspector must stay tied to the active plot, not to a static window position.
- The browser must stay legible when many variables are present.

## Related Product Artifacts

- [01-product/layer.md](layer.md)
- [01-product/use-cases.md](use-cases.md)
- [02-architecture/layer.md](../02-architecture/layer.md)
