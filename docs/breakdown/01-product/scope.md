---
id: INFO-007
type: info
title: Product scope
summary: Opens multiple time-series sources, browses, plots, compares, derives, and saves workspaces, with a defined V1 boundary
date: 2026-11-11
status: current
---

# Product scope

The viewer opens multiple time-series sources, browses their content, plots and compares them, defines derived series, and saves the resulting workspace.

- **Sources:** multiple CSV files and multiple SQLite databases.
- **Browsing:** visual browsing of files, tables, and nested variable names.
- **Plotting:** one or more selected series in the same view, with derived comparisons such as `a - b`.
- **Refresh:** live refresh while source files change.
- **Persistence:** saving and restoring views as project files.
- **Out of scope:** networked collaboration, remote data sources, streaming services, notebook-style scripting, and analytics pipelines beyond simple expressions and comparisons.