---
id: IMP-034
type: imp
title: File watchers for live updates
summary: Replace or supplement polling with platform file notifications where justified
date: 2026-11-11
status: suggested
---

# File watchers for live updates

- **Question:** when is file-watcher-based refresh justified over polling?
- **Evidence:** live updates use polling, which is simple but less immediate than file notifications (`INFO-026`).
- **Change:** file watchers are added where platform support and complexity justify them, keeping predictable refresh behavior.
- **Risk:** platform variation adds complexity; polling must remain the fallback so behavior stays predictable.
- **Acceptance:** supported platforms get immediate refresh on source change, and behavior degrades gracefully where watchers are unavailable.