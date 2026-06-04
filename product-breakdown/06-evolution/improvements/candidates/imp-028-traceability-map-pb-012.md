# IMP-028 Add PB-012 to traceability-map.md

## Problem

`product-breakdown/traceability-map.md` is the centralized traceability map
linking product intent to behavior, architecture, implementation, verification,
operation, and evolution. Its table covers PB-001 through PB-011 behavior but is
missing a row for PB-012 (sidebar layout decision), which is recorded in
`02-architecture/layer.md` line 87. The missing row breaks the traceability
contract.

## Why It Matters

The traceability map is defined as the cross-layer reference for all product
decisions. Missing entries make it impossible to trace a decision through all
layers from a single table.

## Scope

- Edit `product-breakdown/traceability-map.md` only.
- Add one row to the traceability map table for PB-012.

## Out of Scope

- Do not modify `product-breakdown/02-architecture/layer.md`.
- Do not modify existing rows in the traceability map.
- Do not add entries for any other missing decisions not identified in the
  review.

## Constraints and Dependencies

- Follow KM-007: no stale references — the new entry must accurately reflect
  PB-012 content from `02-architecture/layer.md`.
- Must match the existing table format (7 columns: Intent/outcome | Product
  behavior | Architecture | Implementation | Verification | Operation |
  Evolution).
- Layer references should point to the appropriate product-breakdown layer
  files.

## Prevention Rules

KM-007, PAT-001

## Progress State

`Backlog`

## Acceptance Criteria

- `traceability-map.md` contains a PB-012 row.
- PB-012 row content accurately describes the sidebar layout decision from
  `02-architecture/layer.md`.
- Existing rows are unchanged.
- Table formatting is consistent.

## Traceability

- Source: `product-breakdown/traceability-map.md`
- Reference: `product-breakdown/02-architecture/layer.md` (line 87, PB-012)
- Evidence: `product-breakdown/06-evolution/reviews/doc-review-2026-06-04.md`
  (Finding 2, LOW)