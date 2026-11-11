---
id: IMP-021
type: imp
title: Add PB-012 to the decision log
summary: Record the sidebar-layout decision in the centralized decision log
date: 2026-11-11
status: implemented
---

# Add PB-012 to the decision log

- **Question:** how is the sidebar-layout decision findable from the centralized decision log?
- **Evidence:** the sidebar layout is defined in the architecture layer but missing from the decision log, breaking the traceability contract that the log is the root index of stable decisions.
- **Change:** one row records the sidebar layout decision in the decision log with the architecture layer and recorded status.
- **Risk:** existing decision entries stay unchanged.
- **Acceptance:** the decision log contains the sidebar layout entry whose title and description match the architecture definition, with consistent table formatting.