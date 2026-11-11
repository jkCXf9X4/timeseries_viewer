---
title: Architecture
summary: How the viewer's parts fit and interact
---

# Architecture

## Contents

<!-- pb:index:start -->
<!-- GENERATED FILE — do not edit. Regenerate with `pb registers`. -->
- [Components](components/__INDEX__.md) — The individual architectural components and their responsibilities
- **INFO-017** [Architecture structure](structure.md) — Separates source ingestion, normalized time-series state, expression evaluation, view state, and UI rendering
- **INFO-018** [Architecture boundaries](boundaries.md) — The major module boundaries that partition UI, windows, binding, sources, registry, cache, expressions, persistence, and refresh
- **INFO-019** [Data flow](data-flow.md) — How data moves from opening sources through adapters, the registry, binding, plotting, refresh, and persistence
- **INFO-020** [Quality attributes](quality-attributes.md) — Low dependency footprint, predictable Linux behavior, responsive medium-data interaction, clear recovery, and traceable restoration
- **INFO-021** [Architecture risks](risks.md) — In-memory scale limits, polling latency, expression determinism, and multi-window state complexity
<!-- pb:index:end -->
