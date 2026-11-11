---
id: IMP-027
type: imp
title: Add module structure to the application plan
summary: Connect the plan's architecture to the actual module layout in the codebase
date: 2026-11-11
status: implemented
---

# Add module structure to the application plan

- **Question:** how does the application plan connect architectural decisions to the actual code layout?
- **Evidence:** the plan describes the intended architecture but not the module structure implemented by the extraction work, breaking the onboarding traceability chain (`INFO-028`, `INFO-029`).
- **Change:** the plan gains an architecture and module structure section describing each source directory and its responsibility.
- **Risk:** existing plan sections stay unchanged, and each module description matches the actual directory layout.
- **Acceptance:** the plan contains the module structure section listing the source directories with accurate responsibility descriptions and a coherent insertion point.