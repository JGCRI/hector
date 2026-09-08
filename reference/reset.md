# Reset a Hector instance to an earlier date

Resetting the model returns it to its state at a previous time. If the
requested time is before the model start date, then the spinup will be
rerun, and the model will be left ready to run at the start date. (By
contrast, resetting *to* the start date leaves the model ready to run at
the start date, but without having rerun the spinup.)

## Usage

``` r
reset(core, date = 0)
```

## Arguments

- core:

  Handle for the Hector instance that is to be reset.

- date:

  Date to reset to. The default is to reset to the model start date with
  a rerun of the spinup.

## See also

Other main user interface functions:
[`fetchvars()`](https://jgcri.github.io/hector/reference/fetchvars.md),
[`get_tracking_data()`](https://jgcri.github.io/hector/reference/get_tracking_data.md),
[`newcore()`](https://jgcri.github.io/hector/reference/newcore.md),
[`run()`](https://jgcri.github.io/hector/reference/run.md),
[`setvar()`](https://jgcri.github.io/hector/reference/setvar.md),
[`shutdown()`](https://jgcri.github.io/hector/reference/shutdown.md)
