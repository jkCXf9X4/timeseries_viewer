---
id: IMP-030
type: imp
title: Live refresh feedback and error messaging
summary: Make live-mode changes and failures clear to the user
date: 2026-11-11
status: suggested
---

# Live refresh feedback and error messaging

- **Question:** how does a user understand what live refresh changed or why it failed?
- **Evidence:** live mode reloads changed sources but the feedback and error messaging are thin (`INFO-039`, `INFO-045`).
- **Change:** live refresh shows what changed, and refresh failures surface with enough context to act on.
- **Risk:** messaging must stay predictable so users know when live data will refresh.
- **Acceptance:** a changed source visibly reloads with an explanation, and a failed refresh reports the cause in the UI.