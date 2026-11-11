---
id: IMP-018
type: imp
title: Extract ImGui backend from main.cpp
summary: Move the concrete backend bridging to ImGui, GLFW, and ImPlot out of the entry point
date: 2026-11-11
status: implemented
---

# Extract ImGui backend from main.cpp

- **Question:** how does the platform-specific backend become reusable and keep the entry point focused?
- **Evidence:** the entry point contains the entire concrete backend struct alongside the main entry point and event loop, preventing reuse and making the file a dumping ground (`INFO-028`).
- **Change:** the backend struct moves to `src/ui/backend/imgui_backend.hpp` and `.cpp`, and the entry point shrinks to initialization and the frame loop.
- **Risk:** the module may depend on Dear ImGui, ImPlot, GLFW, glad, and nfd only, and the template interface contract used by the rendering templates stays stable.
- **Acceptance:** the application compiles and runs identically after extraction, the entry point is under 150 lines, and the backend compiles independently of the app, model, IO, expression, and persistence code.