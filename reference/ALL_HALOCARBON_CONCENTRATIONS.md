# Get all of the possible Hector halocarbon concentrations

This function returns a vector of the possible hector halocarbon
concentrations that can be accessed with
[`fetchvars`](https://jgcri.github.io/hector/reference/fetchvars.md).

## Usage

``` r
ALL_HALOCARBON_CONCENTRATIONS()
```

## Value

Character vector of variable names.

## See also

Other outputs:
[`ALL_HALOCARBON_RF()`](https://jgcri.github.io/hector/reference/ALL_HALOCARBON_RF.md),
[`ALL_VARS()`](https://jgcri.github.io/hector/reference/ALL_VARS.md)

## Examples

``` r
if (FALSE) { # \dontrun{
ini <- system.file(package = "hector", "input/hector_ssp245.ini")
hc <- newcore(ini)
run(hc)
out <- fetchvars(core = hc, dates = 1900:2100, vars = ALL_HALOCARBON_CONCENTRATIONS())
print(out)
} # }
```
