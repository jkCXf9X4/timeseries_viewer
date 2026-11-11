---
id: INFO-015
type: info
title: Empty and error states
summary: How the UI explains missing sources, missing selections, and variables that no longer exist
date: 2026-11-11
status: current
---

# Empty and error states

The UI explains missing sources, empty selections, and missing series rather than failing silently.

- **No sources open:** the parameter browser explains that a source must be opened before parameters can be selected; the plot inspector explains that an open source is required for meaningful plot editing.
- **No series selected:** the plot inspector still shows the active plot summary and plot settings, and the selected-series editor shows that no series is selected.
- **Missing series:** if a project reload references a variable that is no longer available, the UI surfaces that clearly.
- **No silent loss:** missing data does not silently disappear from the workspace state.