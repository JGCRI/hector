# Get the hector function associated with a specific string

This function provides the Hector function that returns a the Hector
string

## Usage

``` r
getfxn(str)
```

## Arguments

- str:

  String name to find the Hector function for

## Value

Hector function name

## See also

getunits

## Examples

``` r
getfxn("beta")
#> [1] "BETA()"
getfxn("q10_rh")
#> [1] "Q10_RH()"
```
