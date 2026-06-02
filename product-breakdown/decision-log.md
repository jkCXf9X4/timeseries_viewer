# Decision Log

Root index of stable decisions recorded in the product breakdown.

| ID | Layer | Decision | Status |
| --- | --- | --- | --- |
| PB-001 | 00-intent | Linux-first standalone desktop application with local-file workflows | Recorded |
| PB-002 | 03-implementation | C++23, CMake, and vcpkg are the implementation stack | Recorded |
| PB-003 | 02-architecture | Dear ImGui + ImPlot + GLFW are the UI/plotting foundation | Recorded |
| PB-004 | 01-product | Initial data sources are CSV and SQLite with user-overridable time selection | Recorded |
| PB-005 | 02-architecture | Selected variables are loaded into memory for medium-file operation | Recorded |
| PB-006 | 02-architecture | Live updates use polling rather than platform file watchers | Recorded |
| PB-007 | 02-architecture | Derived series use embedded Lua with `sol2` bindings | Recorded |
| PB-008 | 02-architecture | Cross-series comparison aligns to the left-hand time grid using interpolation | Recorded |
| PB-009 | 01-product | Saved views are project JSON files that restore sources, plots, and expressions | Recorded |

Each decision is detailed in the layer where its consequences are most direct.
