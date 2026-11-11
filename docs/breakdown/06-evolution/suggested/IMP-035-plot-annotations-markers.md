---
id: IMP-035
type: imp
title: Plot annotations, bookmarks, and markers
summary: Add visual markers to plots for events and points of interest
date: 2026-11-11
status: suggested
---

# Plot annotations, bookmarks, and markers

- **Question:** how does a user mark events or points of interest on a plot?
- **Evidence:** the plot surface shows series only, with no way to annotate events or mark points for later reference (`INFO-012`).
- **Change:** plots support annotations, bookmarks, or markers that persist with the view.
- **Risk:** markers must not interfere with the plot-only surface contract or clutter comparisons.
- **Acceptance:** a user places a marker or annotation on a plot and it survives save and reload.