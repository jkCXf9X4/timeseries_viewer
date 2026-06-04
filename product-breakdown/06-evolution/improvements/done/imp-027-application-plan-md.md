# IMP-027 Add module structure section to docs/application-plan.md

## Problem

`docs/application-plan.md` describes the intended architecture but does not
mention the `src/model/`, `src/io/csv/`, `src/io/sqlite/`, `src/expr/`,
`src/persist/`, `src/ui/`, `src/app/` module structure that was implemented by
IMP-011 through IMP-018. Anyone reading the application plan has no way to
connect the architectural decisions to the actual code layout.

## Why It Matters

The application plan is the primary onboarding document. Missing the module
structure makes it harder for contributors to understand where to find or add
code. It also breaks the traceability chain between the architecture described
in the plan and the implementation in the codebase.

## Scope

- Edit `docs/application-plan.md` only.
- Add one "Architecture and Module Structure" section describing each top-level
  source directory with a brief responsibility statement and references to the
  implementation layer.

## Out of Scope

- Do not modify the existing architectural decisions, dependency list, or test
  plan sections.
- Do not add code examples or inline documentation — the section should be a
  structural overview.

## Constraints and Dependencies

- Follow the existing document tone and formatting.
- Each module description must match the actual directory layout produced by
  IMP-011 through IMP-018.

## Prevention Rules

KM-007 (no stale references), PAT-001

## Progress State

`Backlog`

## Acceptance Criteria

- `docs/application-plan.md` contains an "Architecture and Module Structure"
  section.
- The section lists all seven top-level source directories with accurate
  responsibility descriptions.
- Existing sections are unchanged except for the addition of the new section.
- The document reads coherently with the new section inserted between the
  "Saved Views" subsection and the "Initial Dependency Set" heading.

## Traceability

- Source: `docs/application-plan.md`
- Reference: `product-breakdown/03-implementation/layer.md` (IMP-011–IMP-018
  module layout)
- Evidence: `product-breakdown/06-evolution/reviews/doc-review-2026-06-04.md`
  (Finding 1, LOW)