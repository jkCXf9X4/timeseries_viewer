---
id: IMP-041
type: imp
title: Close windows from the window banner
summary: Let a user close an analysis window with a right-click on its banner
date: 2026-11-11
status: suggested
---

# Close windows from the window banner

- **Question:** how does a user close an analysis window without a dedicated control?
- **Evidence:** multiple analysis windows are supported but there is no explicit close affordance on the window banner (`INFO-012`, `UC-005`).
- **Change:** right-clicking a window banner offers a close action for that window.
- **Risk:** the action must not discard unrelated workspace state in other windows.
- **Acceptance:** a user right-clicks a window banner, selects close, and the window closes while other windows and their tabs remain intact.