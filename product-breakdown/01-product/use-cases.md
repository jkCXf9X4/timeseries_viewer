# Use Cases

## UC-01 Variable Selection Tree

### Goal

Let the user browse variables through an expandable tree where `.` in a variable name represents hierarchy.

### User Need

The data source may expose many variables with long names. The user needs to find a variable quickly without scanning a flat list.

### Flow

1. The user opens a CSV file or SQLite database.
2. The app builds a tree from variable names by splitting on `.`.
3. The browser shows parent nodes that can be expanded and collapsed.
4. The user expands a branch such as `ECS_HW`.
5. The user checks a tickbox next to a leaf variable such as `consumerFeed.hh` or `consumerFeed.m`.
6. The checked variable is added to the active plot view.
7. The user clears the tickbox to remove the variable from the active plot view.

### Expected Behavior

- Parent nodes are expandable and do not need to be selected as data series.
- Leaf nodes have tickboxes that add or remove variables from the active plot.
- The same logical name always maps to the same fully qualified variable identifier.
- The tree must remain usable even when many variables are present.

### Notes

- This behavior applies to both CSV and SQLite sources.
- Dotted names are a naming convention only; no special file format is required.

## UC-02 Compare Multiple Files In One Plot

### Goal

Let the user compare signals from multiple files or databases in the same plot view.

### User Need

The user may have the same measurement stored in separate runs, files, or databases and needs to inspect them together.

### Flow

1. The user opens more than one source.
2. The user selects a variable from the first source and plots it.
3. The user selects a comparable variable from the second source.
4. Both series appear in the same plot view.
5. The user can inspect differences, alignment, and trends visually.

### Expected Behavior

- Multiple sources can be open at the same time.
- A plot view can contain series from different sources.
- The user does not need to create a separate plot per source.
- Plot labels must remain clear enough to distinguish series that share similar names.

### Notes

- This use case includes multiple CSV files, multiple SQLite databases, and mixed source sets.
- The comparison view is a first-class workflow, not an export-only feature.

## UC-03 Derived Comparison

### Goal

Let the user plot a simple derived relationship such as `a - b`.

### User Need

The user wants to compare two signals directly without exporting data to another tool.

### Flow

1. The user selects two compatible series.
2. The user defines a derived expression.
3. The app evaluates the expression and creates a new plot series.
4. The derived series appears alongside the raw series in the same plot.

### Expected Behavior

- Derived series are treated like ordinary plot series once created.
- The user can compare the raw traces and the derived result in one view.
- The expression system should be simple enough to use during analysis, not only for power users.

## UC-04 Parameter Binding Menu

### Goal

Let the user choose which parameters are displayed and bind them to one or more open files or databases through an explicit menu or selection panel.

### User Need

The user needs a direct way to say "show this parameter from that source" without guessing how the app interpreted the source or variable name.

### Flow

1. The user opens several CSV files or SQLite databases.
2. The user opens the parameter selection menu or panel.
3. The app shows the available parameters and their source binding, including tickboxes for selected variables.
4. The user assigns one or more parameters to the current plot using the tickboxes.
5. The plot updates to show exactly the chosen source/parameter combination.

### Expected Behavior

- The user can see which source each parameter comes from.
- The user can add or remove parameters using tickboxes next to the variable list.
- The user can change the source binding without reopening the file.
- The menu should support filtering or grouping by open file or database.
- The selection state should be explicit and stable across the session.

### Notes

- This is the control surface for source-to-parameter mapping, not a hidden automatic rule.
- The menu may be implemented as a side panel, tree, table, or other explicit selection UI, but it must remain menu-like and discoverable.

## UC-05 Multi-Window Tabbed Plot Workspace

### Goal

Let the user view multiple windows simultaneously, with each window containing multiple tabs and each tab containing exactly one plot with its own parameter selection.

### User Need

The user may want separate analysis contexts visible at the same time, such as one window per scenario or one window per comparison task.

### Flow

1. The user opens or creates more than one analysis window.
2. Each window shows a tab bar.
3. The user adds one or more tabs to a window.
4. Each tab contains one plot only.
5. The user chooses the parameters for that tab independently from other tabs.
6. The user compares different plots by moving between tabs or viewing multiple windows at once.

### Expected Behavior

- More than one top-level window can be visible simultaneously.
- Each window may have multiple tabs.
- Each tab owns a single plot.
- Each plot has its own parameter selection and does not inherit another tab’s selection automatically.
- The user can use the same data source in more than one tab with different parameter combinations.
- The parameter browser is fixed to the left side of the app and stays anchored there while its width can be resized.
- The plot inspector is fixed to the right side of the app and stays anchored there while its width can be resized.
- The right plot inspector shows the active plot status fields.
- Clicking a window or tab makes that plot the active plot for both side panels.

### Notes

- This use case defines the top-level analysis layout.
- Tabs are a plot container, not a grouping for multiple plots.

## Product Linkage

- These use cases are constrained by [01 product](layer.md).
- Their implementation boundary is described in [02 architecture](../02-architecture/layer.md).
- Their verification expectations are described in [04 verification](../04-verification/layer.md).
