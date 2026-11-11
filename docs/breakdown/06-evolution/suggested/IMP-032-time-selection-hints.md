---
id: IMP-032
type: imp
title: Time-selection hints for mixed formats
summary: Guide users when time metadata varies across sources
date: 2026-11-11
status: suggested
---

# Time-selection hints for mixed formats

- **Question:** how does a user choose the right time axis when sources carry different time metadata?
- **Evidence:** time metadata may vary across sources and require user correction (`INFO-009`, `REQ-002`).
- **Change:** the time-selection UI offers hints when a source's time metadata is uncertain or differs from other open sources.
- **Risk:** hints must not override an explicit user choice.
- **Acceptance:** mixed-format sources surface usable time-axis suggestions while the user's override wins.