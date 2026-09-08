# Run the Hector climate model

Run Hector up through the specified time. This function does not return
the results of the run. To get results, run `fetch`.

## Usage

``` r
run(core, runtodate = -1)
```

## Arguments

- core:

  Handle to the Hector instance that is to be run.

- runtodate:

  Date to run to. The default is to run to the end date configured in
  the input file used to initialize the core.

## Value

The Hector instance handle

## See also

Other main user interface functions:
[`fetchvars()`](https://jgcri.github.io/hector/reference/fetchvars.md),
[`get_tracking_data()`](https://jgcri.github.io/hector/reference/get_tracking_data.md),
[`newcore()`](https://jgcri.github.io/hector/reference/newcore.md),
[`reset()`](https://jgcri.github.io/hector/reference/reset.md),
[`setvar()`](https://jgcri.github.io/hector/reference/setvar.md),
[`shutdown()`](https://jgcri.github.io/hector/reference/shutdown.md)
