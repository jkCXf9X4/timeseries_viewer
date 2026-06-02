# 06 Evolution

## How The Product Should Change Over Time

The first release should prioritize the core workflow. Later changes should increase scale, expression power, and source flexibility without breaking saved views.

## Near-Term Improvements

- Add stronger search and filtering in the source browser.
- Improve live refresh feedback and error messaging.
- Add more expression helpers once basic arithmetic is stable.
- Improve time-selection hints for mixed-format sources.

## Mid-Term Improvements

- Add chunked or lazy loading for large datasets.
- Add file watchers where platform support and complexity justify them.
- Add annotations, bookmarks, or markers on plots.
- Add more export formats for plots and project state.

## Long-Term Improvements

- Add new source types beyond CSV and SQLite.
- Add plugin or extension points if the core workflow remains stable.
- Add richer derived-data workflows, presets, or reusable formulas.
- Add collaboration or shared review flows only if the product direction changes materially.

## Backlog Themes

- Scale: large-file behavior, caching, and incremental reloads.
- Usability: search, presets, and plot ergonomics.
- Expression power: helper functions, reusable calculations, and units.
- Source breadth: additional file types and metadata sources.

## Product Risks

- Feature expansion can erode the low-dependency goal if it is not constrained.
- Saved-view compatibility must be protected as the schema evolves.
- Larger data support may require a revised internal data model.
