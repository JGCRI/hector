# Retrieve the tracking data for a Hector instance

Retrieve the tracking data for a Hector instance

## Usage

``` r
get_tracking_data(core)
```

## Arguments

- core:

  Handle to the Hector instance.

## Value

A [`data.frame`](https://rdrr.io/r/base/data.frame.html) with the
tracking data. Columns include `year` (integer), `component`
(character), `pool_name` (character), `pool_value` (double),
`pool_units` (character), `source_name` (character), and
`source_fraction` (double). The fractions will always sum to 1 for a
given pool and year.

## Note

The `pool_name`, `pool_value`, and `pool_units` names differ from those
used in the model's standard output stream (`variable`, `value`, and
`units` respectively).

## See also

Other main user interface functions:
[`fetchvars()`](https://jgcri.github.io/hector/reference/fetchvars.md),
[`newcore()`](https://jgcri.github.io/hector/reference/newcore.md),
[`reset()`](https://jgcri.github.io/hector/reference/reset.md),
[`run()`](https://jgcri.github.io/hector/reference/run.md),
[`setvar()`](https://jgcri.github.io/hector/reference/setvar.md),
[`shutdown()`](https://jgcri.github.io/hector/reference/shutdown.md)
