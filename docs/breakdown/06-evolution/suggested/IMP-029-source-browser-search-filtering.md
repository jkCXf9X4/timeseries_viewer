---
id: IMP-029
type: imp
title: Source browser search and filtering
summary: Add stronger search and filtering so large source trees stay navigable
date: 2026-11-11
status: suggested
---

# Source browser search and filtering

- **Question:** how does a user find a variable when the dotted-name tree grows large?
- **Evidence:** the variable browser is the main discovery surface but lacks search and filtering beyond expand and collapse (`UC-001`, `INFO-011`).
- **Change:** the source browser gains search and filtering over variable names, keeping the tree navigable at scale.
- **Risk:** filter semantics must stay discoverable and must not replace the explicit tickbox selection workflow.
- **Acceptance:** a user can filter the source tree to matching variables and still add or remove selections with the existing tickboxes.