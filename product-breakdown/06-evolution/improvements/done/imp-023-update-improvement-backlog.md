# IMP-023 Update improvement-backlog.md for IMP-011..IMP-018

## Problem

`product-breakdown/06-evolution/improvement-backlog.md` contains a table listing IMP-001 through IMP-010. IMP-011 through IMP-018 are absent even though they have been implemented and their records exist in `improvements/done/`. The backlog does not provide a complete picture of product improvement history.

## Why It Matters

The improvement backlog is the canonical tracker for all product improvements. Omitting eight completed improvements means the backlog is incomplete and cannot serve as a reliable reference for what has been done.

## Scope

- Edit `product-breakdown/06-evolution/improvement-backlog.md` only.
- Add eight new rows to the backlog table for IMP-011 through IMP-018 with status "Done" and links to their files in `improvements/done/`.

## Out of Scope

- Do not modify the actual IMP files in `done/`.
- Do not change any existing IMP-001..IMP-010 entries.
- Do not change table structure or formatting beyond adding rows.

## Constraints and Dependencies

- Follow KM-007: no stale references — each new entry must link to the actual existing file in `improvements/done/`.
- Must match the existing table format (columns and styling).
- Each entry's status must be "Done".

## Prevention Rules

KM-007, PAT-001

## Progress State

`Backlog`

## Acceptance Criteria

- `improvement-backlog.md` contains IMP-011 through IMP-018 entries.
- All eight new entries have status "Done".
- All eight new entries link to existing files in `improvements/done/`.
- Existing IMP-001..IMP-010 entries are unchanged.

## Traceability

- Source: `product-breakdown/06-evolution/improvement-backlog.md`
- Reference: `product-breakdown/06-evolution/improvements/done/`
- Evidence: `product-breakdown/06-evolution/reviews/doc-review-2026-06-04.md` (Finding 5, MEDIUM)