---
id: IMP-007
type: imp
title: Rendered GUI workflow verification
summary: Drive the real GUI frame loop with a deterministic C++ harness against the shipped UI path
date: 2026-11-11
status: implemented
---

# Rendered GUI workflow verification

- **Question:** how do tests exercise the shipped ImGui UI path rather than only the app model?
- **Evidence:** a state-level harness can miss wiring regressions, interaction bugs, and discoverability failures in the rendered UI (`INFO-044`).
- **Change:** a C++ harness drives the real GUI frame loop through the rendered application surface, deterministically enough for repeatable tests.
- **Risk:** pixel-diff visual regression, cross-platform desktop automation, and accessibility automation are out of scope.
- **Acceptance:** tests exercise the actual GUI entry path, a scripted run opens sources, creates tabs, binds parameters, and saves a project through the GUI, and at least one integration test fails if the UI wiring breaks while the model still compiles.