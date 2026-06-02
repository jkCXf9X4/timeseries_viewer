# Security

## Data Handling

- The application operates on local files chosen by the user.
- There is no backend service, account system, or remote sync path in the current design.
- Project files may contain local filesystem paths and analysis state.

## Access Model

- The app relies on the user's filesystem permissions.
- Opening a file requires that the user can read the file locally.
- Saving a project requires write access to the target directory.

## Exposure Notes

- The product does not currently transmit source data anywhere.
- If you share a project file, it may reveal source paths and analysis choices.

## Product Context

Security expectations are part of the operational constraints in:

- [05 operation](../product-breakdown/05-operation/layer.md)
