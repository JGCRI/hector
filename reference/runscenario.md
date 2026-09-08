# Run a single scenario

Run the scenario defined by the input file and return a data frame
containing results for the default variable list. The default variable
list can be changed by setting the `hector.default.fetchvars` option, as
described in
[`fetchvars`](https://jgcri.github.io/hector/reference/fetchvars.md).

## Usage

``` r
runscenario(infile)
```

## Arguments

- infile:

  INI-format file containing the scenario definition

## Value

Data frame containing Hector output for default variables
