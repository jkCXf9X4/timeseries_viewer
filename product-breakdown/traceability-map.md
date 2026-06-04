# Traceability Map

This map links product intent to behavior, structure, implementation, verification, and operational expectations.

| Intent / outcome | Product behavior | Architecture | Implementation | Verification | Operation | Evolution |
| --- | --- | --- | --- | --- | --- | --- |
| Standalone Linux desktop tool | Local app, no server dependency | Native UI shell | C++ app entrypoint and app state | Build and launch smoke check | Package as a self-contained desktop artifact | Broaden packaging options later |
| Open multiple CSV and SQLite sources | Source manager and file/table browser | Source adapter boundary | CSV and SQLite loaders | Source import tests | Handle local file paths and permissions | Add more source types later |
| Browse nested variables visually | Tree browser with dotted names | Canonical variable registry | Variable model and tree builder | Tree construction tests | Keep variable naming stable across sessions | Add filtering and search refinements |
| Select parameters through a menu | Explicit source/parameter binding UI | Parameter binding menu and source grouping | Selection state and binding model | Binding persistence and selection tests | Keep bindings clear when sources change | Add richer filtering and grouping |
| View multiple windows with tabbed plots | Window-level analysis contexts, one plot per tab | Window manager and tabbed plot containers | Window/tab state and plot ownership | Multi-window layout tests | Preserve per-tab selections across sessions | Add window presets and layout templates |
| Live plot selected variables | Plot workspace updates in place | Polling refresh scheduler | Refresh loop and cache invalidation | Reload behavior tests | Predictable refresh and failure messaging | Replace polling or add watchers if needed |
| Compare series in one view | Overlaid series plots | Shared time-series model | Plot view model and series registry | Comparison rendering tests | Keep view state recoverable after reload | Add richer comparison modes |
| Plot `a - b` and similar expressions | Derived series from named inputs | Expression engine with interpolation | Lua/sol2 evaluator and alignment logic | Expression math and interpolation tests | Surface expression errors clearly | Add functions and reusable formulas |
| Save views | Project files restore state | Persistence boundary | JSON serializer/deserializer | Round-trip and restore tests | Portable local project files | Add templates, presets, and export formats |
| Navigate parameters and inspect plots through fixed side panels | Fixed left/right side panels that follow the active plot | PB-012 — parameter browser left, plot inspector right, both track active plot | UI shell panel layout with focus-follow wiring | Side panel positioning and focus-follow behavior tests | Side panel layout persists across sessions | Add user-customizable panel layout |

## Trace Links

- Intent comes from [00-intent](00-intent/layer.md).
- Product behavior is captured in [01-product](01-product/layer.md).
- Structural decomposition is captured in [02-architecture](02-architecture/layer.md).
- Code and configuration mapping is captured in [03-implementation](03-implementation/layer.md).
- Test expectations are captured in [04-verification](04-verification/layer.md).
- Support and deployment constraints are captured in [05-operation](05-operation/layer.md).
- Future change direction is captured in [06-evolution](06-evolution/layer.md).
