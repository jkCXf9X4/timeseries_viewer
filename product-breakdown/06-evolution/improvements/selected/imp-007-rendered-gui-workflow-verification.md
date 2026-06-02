# IMP-007 Rendered GUI Workflow Verification

## Problem

The current test harness verifies app state and workflow logic, but it does not exercise the shipped ImGui UI path in `src/app/main.cpp`.

## Why It Matters

The product requirements for the parameter menu, window layout, and plot tabs are user-interface requirements, not just data-model requirements. A state-only harness can miss wiring regressions, interaction bugs, and discoverability failures.

## Scope

- Add a C++-level harness that can drive the real GUI frame loop
- Verify core UI flows against the rendered application surface
- Keep the harness deterministic enough for repeatable tests

## Out of Scope

- Full pixel-diff visual regression infrastructure
- Cross-platform desktop automation tooling
- Accessibility-layer automation

## Progress State

`Backlog`

## Acceptance Criteria

- Tests can exercise the actual GUI entry path, not only the app model
- A scripted run can open sources, create tabs, bind parameters, and save a project through the GUI surface
- At least one integration test fails if the UI wiring is broken while the model still compiles

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Verification: [04-verification](../../04-verification/layer.md)
- Implementation boundary: [03-implementation](../../03-implementation/layer.md)
