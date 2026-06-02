# Operations

## Startup And Use

- Launch the desktop app locally.
- Open sources from the Sources panel.
- Save project files when you need to preserve the current workspace state.

## Support Behavior

- The application is intended to run without a backend service.
- Source files remain user-owned local files.
- Project files should be portable when source paths are still valid.
- The app should show import and expression errors directly in the UI.

## Live Mode

- Live mode is polling-based.
- It is suited to changed files, not streaming inputs.
- If a file changes while open, the app reloads affected series on the next poll cycle.

## Recovery Scenarios

- If a source file moves, reopen the source and rebuild the workspace.
- If a SQLite schema changes, reload the source and reselect variables.
- If a variable disappears, remove or replace the plot entry.

## Product Context

Operational requirements are defined in:

- [05 operation](../product-breakdown/05-operation/layer.md)
