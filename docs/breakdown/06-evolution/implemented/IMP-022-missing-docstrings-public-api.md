---
id: IMP-022
type: imp
title: Add docstrings to public API headers
summary: Document purpose, parameters, return values, preconditions, and side effects on all public API surfaces
date: 2026-11-11
status: implemented
---

# Add docstrings to public API headers

- **Question:** how does a contributor understand the public API without reading every implementation?
- **Evidence:** all public API headers carry zero docstrings on their types, functions, or parameters, blocking onboarding and maintenance (`INFO-027`).
- **Change:** triple-slash docstrings cover purpose, parameters, return value, and preconditions or side effects across the nine public headers.
- **Risk:** the change is surgical: no function signatures or implementations change, and the code compiles without new warnings.
- **Acceptance:** every public type and function declaration in the nine headers is documented, no signature changes, and the build stays clean.