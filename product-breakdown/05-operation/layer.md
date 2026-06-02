# 05 Operation

## Operational Requirements

The product must behave as a local desktop utility with clear user-owned data boundaries.

## Support Constraints

- No backend service is required.
- Source files remain under user control.
- Project files should be portable across machines when paths are resolvable.
- The application should fail clearly when a file cannot be opened, parsed, or refreshed.
- Live mode should remain understandable even when a source changes unexpectedly.

## Deployment Expectations

- The product should be buildable as a standalone Linux desktop application.
- Runtime dependencies should remain limited and explicit.
- The application should not require an external database server.

## Operational Behavior

- Preserve unsaved view state only within the current session.
- Surface import and expression errors in the UI with enough context to fix them.
- Keep polling behavior predictable so users know when live data will refresh.
- Keep saved project files tied to the user’s local file layout, not to remote state.

## Support Scenarios

- A source file is moved or renamed.
- A SQLite table schema changes between sessions.
- A variable disappears from a live source.
- A project file is opened on another machine with partial path differences.

## Operational Risks

- Large files may need stronger memory guidance later.
- Path portability can fail when project files are moved across environments with different directory layouts.
- The low-dependency approach may still require packaging attention for Linux desktop distributions.
