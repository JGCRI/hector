# A data.frame containing information on all of the Hector inputs listed in an ini file created by save-input-params.R.

A data.frame containing information on all of the Hector inputs listed
in an ini file created by save-input-params.R.

## Usage

``` r
inputstable
```

## Format

A data.frame of 8 columns and 222 rows.

- section:

  String of the section of the ini file of a parameter

- parameter:

  String of the parameter name

- biome.specific:

  String of whether or not the parameter is biome-specific

- time.variant:

  String of whether or not the parameter is time-variant

- required:

  String of wheterh or not the parameter is required

- default:

  String of the default value of the parameter

- units:

  String of the parameter units

- description:

  String of a brief description of the parameter from the ini file
