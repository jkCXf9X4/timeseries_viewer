---
id: IMP-038
type: imp
title: Plugin and extension points
summary: Add extension points for behavior or data workflows if the core workflow stays stable
date: 2026-11-11
status: suggested
---

# Plugin and extension points

- **Question:** how do users extend the viewer without forking it?
- **Evidence:** the core workflow is a closed set of capabilities with no extension surface (`INFO-007`, `CP-001`).
- **Change:** plugin or extension points are added if the core workflow remains stable, allowing external behavior without modifying the core.
- **Risk:** extension points add complexity and can erode the low-dependency goal if not constrained.
- **Acceptance:** an external extension hooks in through a documented interface without changing core behavior, and the low-dependency footprint stays bounded.