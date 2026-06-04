# IMP-025 Expand docs/changelog.md

## Problem

`docs/changelog.md` contains a single "Unreleased" section with a bullet list of initial features. None of the 18 IMP improvements are mentioned. There is no record of what changed between versions or which improvements were implemented.

## Why It Matters

The changelog is the primary document users and contributors consult to understand what has changed in the product. Without entries for implemented improvements, it is impossible to track the evolution of the product from the changelog alone.

## Scope

- Edit `docs/changelog.md` only.
- Add entries for IMP-011 through IMP-018 under appropriate version headings.
- Each entry should briefly describe what was implemented (2-3 sentences per IMP).

## Out of Scope

- Do not remove existing "Unreleased" content.
- Do not modify IMP files in `improvements/done/`.
- Do not add entries for IMP-001..IMP-010 or other improvements not specified.
- Do not modify other documentation files.

## Constraints and Dependencies

- Follow KM-007: no stale references — each entry should reference the IMP file in `improvements/done/`.
- Follow existing markdown format in the changelog.
- The "Unreleased" section's existing content must remain intact.

## Prevention Rules

KM-007, PAT-001

## Progress State

`Backlog`

## Acceptance Criteria

- `docs/changelog.md` contains entries for IMP-011 through IMP-018.
- Each IMP has a brief description of what was implemented (extracted from the done/ file).
- All existing "Unreleased" content remains unchanged.
- Markdown formatting is consistent with existing entries.

## Traceability

- Source: `docs/changelog.md`
- Reference: `product-breakdown/06-evolution/improvements/done/` (IMP-011 through IMP-018)
- Evidence: `product-breakdown/06-evolution/reviews/doc-review-2026-06-04.md` (Finding 7, MEDIUM)