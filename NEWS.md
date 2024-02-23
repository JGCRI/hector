# hector 3.2.0 

* Correct aerosol forcing coefficients based on Zelinka et al. (2023)
* Enable permafrost module and recalibrate model's default parameterization


# hector 3.1.1 

Return global mean surface temperature as a Hector output.

# hector 3.0.1

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.7617326.svg)](https://doi.org/10.5281/zenodo.7617326)

Minor documentation change to the citation file.


# hector 3.0.0

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.7615632.svg)](https://doi.org/10.5281/zenodo.7615632)

* Updated parameterization, incorporating latest consensus science and for best model performance relative to historical observations
* New radiative forcing calculations for consistency with AR6
* Better and more complete software tests; many bug fixes
* Updated from RCPs to SSPs as the default included scenarios
* New net biome production (land-atmosphere carbon exchange) constraint
* New [online documentation](https://jgcri.github.io/hector/)
* Experimental implementation of permafrost implementation: models permafrost thaw as well as CO2 and CH4 release; optional and currently off by default. See [Woodard et al. 2021](https://gmd.copernicus.org/articles/14/4751/2021/gmd-14-4751-2021.pdf)
* New carbon tracking: lets users trace the origin and fate of CO2-C within Hector’s carbon cycle; optional and currently off by default see [carbon tracking example](../articles/ex_carbon_tracking.html) additional details can be found in 
* All changes will be documented in Dorheim et al. in prep 
* Variable name changes see [v3 variable changes](../articles/v3_output_changes.html) for more details


# hector 2.5.0

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.4721584.svg)](https://doi.org/10.5281/zenodo.4721584)

* Register Ftalbedo as input
* Add other non CO~2~ GHG constraints
* Version of Hector that participated in [RCMIP I](https://gmd.copernicus.org/articles/13/5175/2020/) and [RCMIP II](https://agupubs.onlinelibrary.wiley.com/doi/full/10.1029/2020EF001900)


# hector 2.3.0

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.3144007.svg)](https://doi.org/10.5281/zenodo.3144007)

* Constrain atmospheric CO~2~ concentration to be equal to the
  preindustrial concentration during the model spinup. The result of
  this is that the concentration at the beginning of the simulation
  will be equal to the value specified in the `PREINDUSTRIAL_CO2`
  parameter, which was not the case previously.

# hector 2.2.2

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.2667325.svg)](https://doi.org/10.5281/zenodo.2667325)

* Fix bug that was causing requests for H~2~O forcing in the R interface
  to return N~2~O forcing instead (the model internals were
  unaffected).
* Fix bug that was causing API requests for halocarbon forcing to
  return absolute forcing values, rather than values relative to the
  base year (which is what is done for all other forcings).
* Add missing `RF_VOL` (volcanic forcing) dependency to forcing component.  In practice the
  missing dependency had no effect because the forcing component
  already had a dependency on the SO~2~ component, which also provides
  the volcanic forcing, but in the event that we ever split them up,
  this will ensure that the dependency graph is still valid. (All 2.2.2 changes merged in PR #303)

# hector 2.2.1

* Report global averages for land temperature, ocean air
  temperature, and ocean surface temperature.
* Fix bug that prevented CH~4~ emissions from being read.

# hector 2.2.0

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.2605439.svg)](https://doi.org/10.5281/zenodo.2605439)

* Add a new parameter: `VOLCANIC_SCALE`.  This parameter adjusts the
  strength of the response to volcanic forcing.  (PR #291)
* Add `getname()` function to return the name of a Hector core.

# hector 2.1.0

* Add `reset()` function to the API. Calling this function resets a
  Hector core to an earlier time, rerunning the spinup if
  appropriate. (PR #243)

# hector 2.0.0

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1194360.svg)](https://doi.org/10.5281/zenodo.1194360)

* Incorporated 1-D diffusive ocean heat model as new temperature component (DOECLIM) (PR #206)
* Bugfix: double counting halocarbon radiative forcing (PR #201)
* Bugfix: re-enabled CO~2~ concentration constraint (PR #163)
* Various changes to internals to support calling Hector from external code like `pyhector`
* Component loggers are now optional (PR #218)
* Renamed anthro emissions to ffi emissions (fossil fuel industrial) (PR #116)

# hector 1.1.2

* Add `libhector` target OS X and Windows project files (it was already
  available in the Linux build). This library is used when
  you want to link Hector to an outside program.

# hector 1.1.1

* Fix OS X build
* Update sample output files
* Add GPL license

# hector 1.1

* API for linking with external models.
* Backend R scripts faster and cleaner.
* Minor bug fixes and documentation updates.

# hector 1.0

* First release. Corresponds to [Hartin et al. (2015)](http://www.geosci-model-dev.net/8/939/2015/gmd-8-939-2015.pdf).

