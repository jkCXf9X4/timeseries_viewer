# IMP-010 Comparison Label Disambiguation

## Problem

The suite covers mixed-source comparisons and derived expressions, but it does not assert that series labels stay clear when similar names appear together in one plot.

## Why It Matters

Comparison workflows depend on the user being able to tell series apart quickly. If labels become ambiguous, the plot can still render correctly while the analysis result becomes easy to misread.

## Scope

- Verify that raw and derived series are labeled distinctly enough in comparison views
- Ensure series from different sources remain distinguishable when names overlap
- Preserve label clarity across save and reload

## Out of Scope

- Changing the math semantics of comparisons
- Reworking the plot engine
- Adding automatic naming heuristics beyond clarity fixes

## Progress State

`Done`

## Acceptance Criteria

- A comparison plot with similar series names remains readable
- Derived series have labels that can be distinguished from raw series
- Label clarity survives project round-trip

## Traceability

- Product need: [01-product/use-cases](../../01-product/use-cases.md)
- Verification: [04-verification](../../04-verification/layer.md)
- Product scope: [01-product/layer.md](../../01-product/layer.md)

## Implementation Notes

- Plot labels now include the source alias and backing file name for raw series.
- Derived-series labels include the expression, and the render-path tests assert that similar names remain distinct after save/load.
