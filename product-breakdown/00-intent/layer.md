# 00 Intent

## Why This Product Exists

The product exists to give engineers a simple, standalone way to inspect and compare time-series data on Linux without requiring a server, cloud workspace, or heavy operating environment.

It should reduce friction in the common workflow of:

- opening local data files
- finding the right variable quickly
- comparing multiple signals in one view
- testing derived relationships such as `a - b`
- saving the current analysis state for later reuse

## Primary Users

- Engineers and analysts who inspect logged or measured data.
- Users who work with repeated local file sets and want a lightweight desktop workflow.
- Users who value direct visual inspection over notebook-style or server-backed analysis.

## Desired Outcomes

- A standalone desktop tool that feels immediate and local.
- A workflow that keeps source discovery, plotting, comparison, and saved views in one place.
- A product that works well for medium-sized datasets and common engineering logs.

## Constraints

- Linux is the primary supported platform.
- The application should stay low-dependency and self-contained.
- The workflow should center on local files rather than remote services.
- The first version should stay practical instead of overgeneralizing to every possible data source or expression feature.

## Stable Decisions

- PB-001: Linux-first standalone desktop application with local-file workflows.
- PB-005: Medium-file operation is acceptable as the first performance target.

## Assumptions

- The user wants a tool that is usable without a backend service.
- CSV and SQLite are sufficient to cover the first release use case.
- The product should optimize for discoverability and comparison, not for advanced data science automation.
