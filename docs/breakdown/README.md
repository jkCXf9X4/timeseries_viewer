# Timeseries Viewer Breakdown

The timeseries viewer is a standalone Linux desktop tool for inspecting and
comparing time-series data, with a variable browser, plot tabs, derived
expressions, and saved project views. This tree is the source of truth for
what the viewer is, promises, and is verified against; `docs/` holds runnable
guidance, and this tree decides.

## Method

One layer question per folder; enter a layer at its `__INDEX__.md`, then open
the leaf that owns the concern. One leaf answers one question.

Shape is flat: `<layer>/<topic>/<leaf>.md`; a topic folder exists only for three
or more leaves, and one or two stay flat beside them. One `__INDEX__.md` per
content folder; its generated `## Contents` lists the direct children.

Boundary: the breakdown is the source of truth; design detail defaults to the
lower layer, and ownership decides where a fact lives. Review routes findings
back to the owning layer. Current state lives in the leaves; history in the
decision archive, reached through `pb archive`; open work in `06-evolution/`.

Excludes: runnable guidance, install and build walkthroughs, and operator
walkthroughs live in `docs/`; the breakdown states policy and decisions, never
runnable detail.