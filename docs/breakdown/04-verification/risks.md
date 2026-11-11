---
id: INFO-044
type: info
title: Verification risks
summary: Visual correctness needs manual checks, expression and interpolation edges need fixtures, and GUI wiring needs a rendered harness
date: 2026-11-11
status: current
---

# Verification risks

Verification has three areas where automated checks alone can mislead.

- **Visual correctness:** best validated with focused manual checks rather than only unit tests.
- **Edge cases:** expression and interpolation edge cases need explicit fixtures to avoid false confidence.
- **GUI wiring:** the shipped GUI path is exercised through a scripted render-path harness in addition to the state layer.