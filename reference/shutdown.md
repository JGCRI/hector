# Shut down a hector instance

Shutting down an instance will free the instance itself and all of the
objects it created. Any attempted operation on the instance after that
will raise an error.

## Usage

``` r
shutdown(core)
```

## Arguments

- core:

  Handle to a Hector instance

## Value

The Hector instance handle

## Caution

This function should be called as `mycore <- shutdown(mycore)` so that
the change from active to inactive will be recorded in the caller.

## See also

Other main user interface functions:
[`fetchvars()`](https://jgcri.github.io/hector/reference/fetchvars.md),
[`get_tracking_data()`](https://jgcri.github.io/hector/reference/get_tracking_data.md),
[`newcore()`](https://jgcri.github.io/hector/reference/newcore.md),
[`reset()`](https://jgcri.github.io/hector/reference/reset.md),
[`run()`](https://jgcri.github.io/hector/reference/run.md),
[`setvar()`](https://jgcri.github.io/hector/reference/setvar.md)
