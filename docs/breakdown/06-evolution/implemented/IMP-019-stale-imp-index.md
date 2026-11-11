---
id: IMP-019
type: imp
title: Fix stale IMP index
summary: Correct the improvements index so completed IMPs are listed as done with working paths
date: 2026-11-11
status: implemented
---

# Fix stale IMP index

- **Question:** how do readers reach the completed improvement records without stale paths?
- **Evidence:** the improvements index lists completed IMPs under backlog with paths pointing to a directory that does not exist.
- **Change:** the index lists the completed IMPs under the done section with paths to their actual files, leaving the open IMP entry unchanged.
- **Risk:** edits stay confined to the index file so no actual IMP record is modified.
- **Acceptance:** completed IMPs are listed as done, all entry paths point to existing files, and no broken links remain in the index.