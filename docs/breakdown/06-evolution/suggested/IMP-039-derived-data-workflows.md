---
id: IMP-039
type: imp
title: Richer derived-data workflows and reusable formulas
summary: Add presets and reusable formula definitions for derived data
date: 2026-11-11
status: suggested
---

# Richer derived-data workflows and reusable formulas

- **Question:** how do users reuse derived-data definitions across analyses?
- **Evidence:** derived series are created per tab from a draft, with no way to save or reuse a definition (`INFO-013`, `UC-003`).
- **Change:** derived-data workflows gain presets or reusable formulas that can be applied across plots and saved views.
- **Risk:** reusable formulas must stay deterministic and must not break the per-tab scoping contract.
- **Acceptance:** a user saves a derived-series definition, applies it to another tab, and the result renders like a normal series.