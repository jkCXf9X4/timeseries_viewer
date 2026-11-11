---
title: Components
summary: The individual architectural components and their responsibilities
---

# Components

## Contents

<!-- pb:index:start -->
<!-- GENERATED FILE — do not edit. Regenerate with `pb registers`. -->
- **INFO-022** [UI shell component](ui-shell.md) — Provides the multi-window workspace, source browser, parameter menu, plot tabs, expression editor, and project actions on the UI foundation
- **INFO-023** [Window manager and plot workspace components](window-and-plot.md) — Window manager owns top-level windows and tab sets; plot workspace owns overlays, axes, visibility, and per-plot selection
- **INFO-024** [Parameter binding and source adapter components](binding-and-sources.md) — Explicit source-to-parameter mapping plus CSV and SQLite adapters that normalize inputs into a common representation
- **INFO-025** [Registry, expression, and persistence components](registry-data-expression.md) — Variable registry maps fully qualified names; expression engine evaluates derived series with interpolation; persistence stores and restores state
- **INFO-026** [Refresh scheduler component](refresh-scheduler.md) — Polls sources for changes and invalidates or reloads affected series when live mode is enabled
<!-- pb:index:end -->
