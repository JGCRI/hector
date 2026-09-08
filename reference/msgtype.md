# Message type identifiers

These identifiers indicate the type of a message so that the core can
route it to the correct method in the recipient.

## Usage

``` r
GETDATA()

SETDATA()

BIOME_SPLIT_CHAR()
```

## Functions

- `GETDATA()`: Message type for retrieving data from a component

- `SETDATA()`: Message type for setting data in a component

- `BIOME_SPLIT_CHAR()`: Character used to separate biome from variable
  name

## Note

Because these identifiers are provided as `#define` macros in the hector
code, these identifiers are provided in the R interface as functions.
Therefore, these objects must be called to use them; *e.g.*, `GETDATA()`
instead of the more natural looking `GETDATA`.
