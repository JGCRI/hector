# Create and initialize a new hector instance

The object returned is a handle to the newly created instance. It will
be required as an argument for all functions that operate on the
instance. Creating multiple instances simultaneously is supported.

## Usage

``` r
newcore(
  inifile,
  loglevel = 0,
  suppresslogging = TRUE,
  name = "Unnamed Hector core"
)
```

## Arguments

- inifile:

  (String) name of the hector input file.

- loglevel:

  (int) minimum message level to output in logs (see
  [`loglevels`](https://jgcri.github.io/hector/reference/loglevels.md)).

- suppresslogging:

  (bool) If true, suppress all logging (loglevel is ignored in this
  case).

- name:

  (string) An optional name to identify the core.

## Value

handle for the Hector instance.

## See also

Other main user interface functions:
[`fetchvars()`](https://jgcri.github.io/hector/reference/fetchvars.md),
[`get_tracking_data()`](https://jgcri.github.io/hector/reference/get_tracking_data.md),
[`reset()`](https://jgcri.github.io/hector/reference/reset.md),
[`run()`](https://jgcri.github.io/hector/reference/run.md),
[`setvar()`](https://jgcri.github.io/hector/reference/setvar.md),
[`shutdown()`](https://jgcri.github.io/hector/reference/shutdown.md)
