---
id: INFO-014
type: info
title: Focus and activation rules
summary: The active plot follows the clicked tab or focused window; both side panels track it, and dialogs stay in front
date: 2026-11-11
status: current
---

# Focus and activation rules

Focus drives which plot the side panels edit, and dialogs stay in front.

- **Window focus:** the active analysis window follows focus.
- **Active plot:** the active plot follows the clicked tab or focused window.
- **Inspector:** the plot inspector always reflects the current active plot.
- **Browser:** the parameter-browser selection always targets the current active plot.
- **No extra selector:** the user does not need a separate active-window selector.
- **Dialogs:** file dialogs appear in front of the application, not behind it, and are parented to the main application window.
- **Launch:** dialogs are launched from the app shell, not from the plot body.