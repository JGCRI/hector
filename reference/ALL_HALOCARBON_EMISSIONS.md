# Get all of the possible Hector halocarbon emissions

This function returns a vector of the possible hector halocarbon
emissions that can be accessed with
[`fetchvars`](https://jgcri.github.io/hector/reference/fetchvars.md) or
set with [`setvar`](https://jgcri.github.io/hector/reference/setvar.md).

## Usage

``` r
ALL_HALOCARBON_EMISSIONS()
```

## Value

Character vector of variable names.

## Examples

``` r
if (FALSE) { # \dontrun{
ini <- system.file(package = "hector", "input/hector_ssp245.ini")
hc <- newcore(ini)
run(hc)
out <- fetchvars(core = hc, dates = 1900:2100, vars = ALL_HALOCARBON_EMISSIONS())
print(out)
} # }
```
