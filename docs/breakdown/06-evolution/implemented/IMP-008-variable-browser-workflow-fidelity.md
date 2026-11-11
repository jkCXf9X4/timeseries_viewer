---
id: IMP-008
type: imp
title: Variable browser workflow fidelity
summary: Verify the full browse-and-select workflow, not just dotted-name tree construction
date: 2026-11-11
status: implemented
---

# Variable browser workflow fidelity

- **Question:** how do tests verify the full browsing workflow a user follows to select variables from a large source?
- **Evidence:** the suite verifies dotted-name tree construction but not the browse-and-select workflow that is the main discovery surface for variable selection (`UC-001`).
- **Change:** tests verify that leaf nodes, not parent nodes, drive series selection, that a selected leaf adds the expected variable to the active plot, and that the browser remains usable with large variable sets.
- **Risk:** redesigning the browser widget or adding search and filtering is out of scope unless required by the browser implementation.
- **Acceptance:** a test proves a dotted-name leaf selection yields the correct fully qualified series, parent nodes remain navigation-only, and the workflow stays stable with many variables present.