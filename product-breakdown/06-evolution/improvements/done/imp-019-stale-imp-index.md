# IMP-019 Fix stale IMP index in improvements/README.md

## Problem

`product-breakdown/06-evolution/improvements/README.md` lists IMP-011 through IMP-018 under the "Backlog" section with paths pointing to `candidates/imp-01[1-8]-*.md`. However, all eight files actually reside in `product-breakdown/06-evolution/improvements/done/`. Readers following those links will encounter non-existent paths.

## Why It Matters

The improvements README is the entry point for all improvement artifacts. Stale references break discoverability and mislead contributors about the state of completed work.

## Scope

- Edit `product-breakdown/06-evolution/improvements/README.md` only.
- Move the eight IMP-011..IMP-018 entries from the "Backlog" section to the "Done" section.
- Update all eight link paths from `candidates/` to `done/`.

## Out of Scope

- Do not modify the actual IMP files in `done/`.
- Do not change formatting or structure of existing "Done" entries.
- Do not rearrange IMP-001..IMP-010 entries.

## Constraints and Dependencies

- Follow KM-007: clean up stale references — no stale paths may remain.
- Must NOT break existing links in the file.
- Must maintain the existing markdown list format.

## Prevention Rules

KM-007, PAT-001

## Progress State

`Backlog`

## Acceptance Criteria

- `improvements/README.md` lists IMP-011 through IMP-018 under "Done" (not "Backlog").
- All eight entry paths point to the `done/` directory.
- Existing IMP-001..IMP-010 "Done" entries remain unchanged.
- Existing IMP-004 "Backlog" entry remains unchanged.
- No broken links in the file.

## Traceability

- Source: `product-breakdown/06-evolution/improvements/README.md`
- Evidence: `product-breakdown/06-evolution/reviews/doc-review-2026-06-04.md` (Finding 1, HIGH)