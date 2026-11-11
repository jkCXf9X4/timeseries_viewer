---
id: INFO-034
type: info
title: Implementation risks
summary: Model-layer sprawl, expression helper coupling, and backward-readable project schemas
date: 2026-11-11
status: current
---

# Implementation risks

The implementation carries three main risks.

- **Model sprawl:** if the codebase grows quickly, the model layer can become a dumping ground unless boundaries stay explicit.
- **Expression coupling:** expression helpers need good naming and stable lookup semantics to avoid hidden coupling.
- **Schema evolution:** project schema changes must remain backward-readable once saved views exist in the wild.