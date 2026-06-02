# 01 Product

## What The Product Should Do

The product should let a user open multiple time-series sources, browse their content visually, select variables, plot them live, compare them in one view, define simple derived series, and save the resulting workspace.

## Scope

### In Scope

- Multiple CSV files
- Multiple SQLite databases
- Visual browsing of files, tables, and nested variable names
- Plotting one or more selected series in the same view
- Derived comparisons such as `a - b`
- Live refresh while source files change
- Saving and restoring views as project files

### Out Of Scope For V1

- Networked collaboration
- Remote data sources
- Streaming services
- Notebook-style scripting environment
- Advanced analytics pipelines beyond simple expressions and comparisons

## Capabilities

- Open and inspect multiple source files in one session.
- Find variables through a nested tree built from dotted names.
- Use a menu or panel to choose which parameters are displayed and which open source provides them.
- Select time axis and value columns where necessary.
- View multiple analysis windows simultaneously, with multiple tabs per window and one plot per tab.
- Overlay multiple series in a single plot.
- Create derived series from expressions.
- Persist workspace state and restore it later.

## Use Cases

See [use cases](use-cases.md) for the concrete user flows that the first implementation must support.

## Product Requirements

- The source browser must make file and variable selection visual and explicit.
- The app must support nested variable names that use `.` as a hierarchy separator.
- The app must expose an explicit parameter-selection menu or panel that maps parameters to open files or databases.
- The workspace must support multiple simultaneous windows, multiple tabs per window, and exactly one plot per tab.
- Time selection must be understandable and overridable by the user.
- Derived comparisons must be usable in the same plot workspace as raw series.
- Saved views must restore the essential state of the analysis session.

## Domain Terms

- Source: a CSV file or SQLite database opened in the session.
- Variable: a selectable series within a source.
- Time column: the x-axis field used to align a series.
- View: a saved or active plot configuration containing series, layout, and display choices.
- Expression: a derived series definition computed from one or more inputs.

## Stable Decisions

- PB-004: CSV and SQLite are the initial supported source types, with user-overridable time selection.
- PB-009: Saved views are project JSON files that restore sources, plots, and expressions.

## Product Risks

- Very large files may exceed the comfort zone of in-memory loading.
- Time metadata may vary across sources and require user correction.
- Expression usability may need refinement after real-world use.
