# 04 Verification

## How We Know It Works

Verification must prove the product can open real files, select variables, render plots, evaluate expressions, and restore saved views.

## Verification Layers

### Unit Coverage

- CSV parsing and column inference
- SQLite table and column discovery
- time-column detection and user override behavior
- dotted-name tree construction
- series interpolation
- expression evaluation
- project JSON serialization and deserialization

### Integration Coverage

- Open multiple sources in one session.
- Select variables from nested names.
- Overlay multiple raw series in one view.
- Create a derived `a - b` series and verify alignment.
- Save a project and restore it into the same workspace shape.

### Workflow Coverage

- Live mode detects a changed source and reloads the affected series.
- A missing or renamed variable is reported clearly.
- Reloaded data keeps the view layout and selected plots intact where possible.

### Manual Acceptance

- The app launches cleanly on Linux.
- The user can browse multiple sources without a backend service.
- The user can create, compare, and save views without leaving the application.

## Acceptance Criteria

- The source browser exposes CSV and SQLite content consistently.
- The same source state can be saved and restored from a project file.
- A comparison plot can show multiple series together.
- Derived expressions evaluate deterministically and render like normal series.
- Live refresh updates visible data without corrupting the workspace.

## Traceability Targets

- Product capabilities are traced from [01-product](../01-product/layer.md).
- Architecture dependencies are traced from [02-architecture](../02-architecture/layer.md).
- Implementation hooks are traced from [03-implementation](../03-implementation/layer.md).

## Verification Risks

- Visual correctness is best validated with focused manual checks rather than only unit tests.
- Expression and interpolation edge cases need explicit fixtures to avoid false confidence.
