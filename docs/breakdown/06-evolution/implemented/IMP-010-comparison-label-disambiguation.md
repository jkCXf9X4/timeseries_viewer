---
id: IMP-010
type: imp
title: Comparison label disambiguation
summary: Keep series labels distinct in mixed-source comparisons across save and reload
date: 2026-11-11
status: implemented
---

# Comparison label disambiguation

- **Question:** how do series labels stay clear when similar names appear together in one comparison plot?
- **Evidence:** the suite covers mixed-source comparisons but does not assert that series labels stay distinct when similar names appear together (`UC-002`).
- **Change:** raw and derived series are labeled distinctly in comparison views, series from different sources stay distinguishable when names overlap, and label clarity survives save and reload.
- **Risk:** changing comparison math semantics or reworking the plot engine is out of scope.
- **Acceptance:** a comparison plot with similar series names remains readable, derived-series labels are distinguishable from raw series, and label clarity survives a project round-trip.