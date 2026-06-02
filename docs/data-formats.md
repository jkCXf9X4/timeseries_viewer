# Data Formats

## CSV Input

- A header row is required.
- Columns are interpreted as time, numeric values, or unsupported text.
- The first numeric or ISO-like column is treated as the default time axis.
- Time selection can be overridden by the user when importing or loading data.

Example:

```csv
time,speed,pressure
0,10,101
1,11,102
2,12,103
```

## SQLite Input

- Tables are discovered from the database.
- Table columns are exposed through the source browser.
- Time selection is inferred from the table data and can be overridden.
- The current implementation expects a single table selection per plotted source entry.

## Variable Naming

- Fully qualified names use `.` as a hierarchy separator.
- Example: `aircraft.engine.rpm`
- The browser shows this as nested nodes while preserving the full name for lookup.

## Project Files

Project files are JSON documents that store:

- source paths and types
- selected tables and time columns
- selected variables
- plot views and series selections
- expression definitions

## Product Context

The durable data-model rules are in:

- [01 product](../product-breakdown/01-product/layer.md)
- [03 implementation](../product-breakdown/03-implementation/layer.md)
