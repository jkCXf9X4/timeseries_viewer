# IMP-021 Add PB-012 to decision-log.md

## Problem

`product-breakdown/decision-log.md` is the centralized decision log for all architectural decisions. Its table lists PB-001 through PB-011. However, `product-breakdown/02-architecture/layer.md` line 87 defines PB-012 (sidebar layout decision). PB-012 is recorded in the architecture layer but missing from the centralized decision log, breaking the traceability contract.

## Why It Matters

The decision log is defined as the "root index of stable decisions." Missing entries break traceability and make it impossible to find all architectural decisions from a single location.

## Scope

- Edit `product-breakdown/decision-log.md` only.
- Add one row to the decision table for PB-012.

## Out of Scope

- Do not modify `product-breakdown/02-architecture/layer.md`.
- Do not change existing PB-001..PB-011 entries.
- Do not add entries for any other missing decisions not identified in the review.

## Constraints and Dependencies

- Follow KM-007: no stale references — the new entry must accurately reflect PB-012 content from `02-architecture/layer.md`.
- Must match the existing table format (columns: ID, Title, Layer, Status, Date).
- Layer value: `02-architecture`
- Status value: `Recorded`

## Prevention Rules

KM-007, PAT-001

## Progress State

`Backlog`

## Acceptance Criteria

- `decision-log.md` contains a PB-012 entry.
- PB-012 entry title and description match the content in `02-architecture/layer.md`.
- Existing PB-001..PB-011 entries are unchanged.
- Table formatting is consistent.

## Traceability

- Source: `product-breakdown/decision-log.md`
- Reference: `product-breakdown/02-architecture/layer.md` (line 87, PB-012)
- Evidence: `product-breakdown/06-evolution/reviews/doc-review-2026-06-04.md` (Finding 3, HIGH)