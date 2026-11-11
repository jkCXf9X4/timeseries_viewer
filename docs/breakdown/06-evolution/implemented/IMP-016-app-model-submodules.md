---
id: IMP-016
type: imp
title: Restructure app model into domain submodules
summary: Split the monolithic app model into window, selection, cache, source, and project submodules
date: 2026-11-11
status: implemented
---

# Restructure app model into domain submodules

- **Question:** how do the app model's distinct responsibilities become separately testable units?
- **Evidence:** the app model implementation is a monolithic file tangling workspace management, cache logic, source lifecycle, parameter binding, derived series, project load and save, and live reload (`INFO-028`).
- **Change:** the app model splits into `src/app/windows/workspace`, `src/app/selection/binding`, `src/app/cache`, `src/app/source`, and `src/app/project` submodules, with `src/app/app_model.hpp` as a unified public header.
- **Risk:** submodules must not depend on UI or ImGui code, and shared types stay in a single header so behavior is unchanged.
- **Acceptance:** the monolithic implementation file is removed or reduced to forwarding includes, all tests pass with no behavior change, and no ImGui or UI header is reachable from any file under `src/app/`.