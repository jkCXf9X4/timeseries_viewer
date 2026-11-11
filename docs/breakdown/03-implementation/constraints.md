---
id: INFO-033
type: info
title: Implementation constraints
summary: Stay focused on the product scope, avoid unneeded abstractions, and keep source logic behind adapters
date: 2026-11-11
status: current
---

# Implementation constraints

The first implementation stays focused and format-agnostic.

- **Scope:** keep the first implementation focused on the product scope from `01-product`.
- **Minimalism:** avoid introducing abstractions not needed by CSV or SQLite import, plotting, or expressions.
- **Adapters:** keep source-specific logic behind adapters so the UI and plot layers remain format-agnostic.