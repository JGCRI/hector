# Fetch results from a running Hector core

This function will fetch results and return them as a data frame. The
core must be active in order for this to work; attempting to fetch
results from a core that has been shut down will cause an error.

## Usage

``` r
fetchvars(core, dates, vars = NULL, scenario = NULL)
```

## Arguments

- core:

  Hector core object

- dates:

  Vector of dates to fetch; this will be automatically trimmed to dates
  that are between the start date and the latest date currently run. Set
  to `NA` to return data with no associated dates such as model
  [parameters](https://jgcri.github.io/hector/reference/parameters.md).

- vars:

  List (or vector) of capability strings defining the variables to be
  fetched in the result.

- scenario:

  Optional scenario name. If not specified, the name element of the
  Hector core object will be used.

## Details

The variables to fetch should be given as a sequence of character
strings corresponding to the capabilities declared by hector components.
These strings should be generated using the capability identifier
functions (see below for a list of documentation pages for these).

The list of variables to fetch if you don't specify `vars` is stored in
the `hector.default.fetchvars` option. If this option is also unset,
then the default variable list is CO2 concentration, total radiative
forcing, CO2 forcing, and global mean temperature. To see a list of the
potential `vars` see `data(inputstable)` and `data(fxntable)`.

## See also

[concentrations](https://jgcri.github.io/hector/reference/concentrations.md),
[emissions](https://jgcri.github.io/hector/reference/emissions.md),
[forcings](https://jgcri.github.io/hector/reference/forcings.md),
[carboncycle](https://jgcri.github.io/hector/reference/carboncycle.md),
[haloemiss](https://jgcri.github.io/hector/reference/haloemiss.md),
[haloforcings](https://jgcri.github.io/hector/reference/haloforcings.md),
[methane](https://jgcri.github.io/hector/reference/methane.md),
[ocean](https://jgcri.github.io/hector/reference/ocean.md),
[so2](https://jgcri.github.io/hector/reference/so2.md),
[temperature](https://jgcri.github.io/hector/reference/temperature.md),
[parameters](https://jgcri.github.io/hector/reference/parameters.md)

Other main user interface functions:
[`get_tracking_data()`](https://jgcri.github.io/hector/reference/get_tracking_data.md),
[`newcore()`](https://jgcri.github.io/hector/reference/newcore.md),
[`reset()`](https://jgcri.github.io/hector/reference/reset.md),
[`run()`](https://jgcri.github.io/hector/reference/run.md),
[`setvar()`](https://jgcri.github.io/hector/reference/setvar.md),
[`shutdown()`](https://jgcri.github.io/hector/reference/shutdown.md)

## Examples

``` r
if (FALSE) { # \dontrun{
ini <- system.file(package = "hector", "input/hector_ssp245.ini")
hc <- newcore(ini)
run(hc)
out <- fetchvars(core = hc, dates = 1900:2100, vars = c(GLOBAL_TAS(), NPP()))
print(out)
} # }
```
