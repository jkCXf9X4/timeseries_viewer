# IMP-006 Large-Data Scaling Strategy

## Problem

The application currently loads selected series into memory, which is appropriate for medium datasets but not for very large archives.

## Why It Matters

Production users may need to inspect longer histories or larger telemetry exports than the current model handles efficiently.

## Scope

- Define the practical dataset size envelope for the current in-memory model
- Identify cache, chunking, or lazy-loading options
- Preserve the current UX for small and medium files while improving the upper bound

## Out of Scope

- Converting the product into a database server
- Adding distributed storage

## Progress State

`Backlog`

## Acceptance Criteria

- The product has an explicit scaling strategy and documented threshold.
- The implementation path is chosen without breaking the existing project format.
- Large-file behavior is predictable and explainable.

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Architecture: [02-architecture](../../02-architecture/layer.md)
- Verification: [04-verification](../../04-verification/layer.md)
