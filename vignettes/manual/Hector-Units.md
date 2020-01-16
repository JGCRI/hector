## Units for emissions passed into Hector

The "Unit Symbol" is the symbol used in the code for the unit.  They are all part of the `Hector::` namespace, so in actual use you would write `Hector::U_TG`, etc.  In most cases the "per year" is left implicit, so emissions in Tg/yr are written as `U_TG`.  The notable exception is CO<sub>2</sub>, where the code tracks both carbon fluxes and carbon pools, and must therefore make a distinction.

Where a mass unit is given without qualification, such as Tg/yr, the mass is the total mass of the emitted compound.  This is to be distinguished from, say Tg N/yr or Pg C/yr, where the mass refers only to the nitrogen or carbon in the emissions.  Many compounds have units that explicitly refer to the total mass, such as Tg CO/yr for carbon monoxide.  Such units are technically the same as a plain Tg/yr (and have been written that way in the table), but the code treats them separately and will not convert them automatically, so be sure to use the unit symbol given for passing in your emissions.  Passing an incorrect unit, even if technically equivalent, will cause Hector to throw an exception.

Emission   |  Units    |    Unit Symbol |   Notes
-----------|-----------|----------------|---------
CO<sub>2</sub> emissions | Pg C/yr | `U_PGC_YR` |  same units for land use change and anthropogenic emissions
BC  | Tg/yr | `U_TG` | 
OC  | Tg/yr | `U_TG` | 
NOx | Tg N/yr | `U_TG_N`| 
CO | Tg/yr | `U_TG_CO`| 
NMVOC | Tg/yr | `U_TG_NMVOC` | 
CH<sub>4</sub> | Tg/yr | `U_TG_CH4` |
N<sub>2</sub>O | Tg/yr | `U_TG_N2O` | Note difference with NOx, which uses Tg N
SO<sub>2</sub> | Gg S/yr | `U_GG_S` |
halocarbons | Gg/yr | `U_GG` | All halocarbons use the same unit.

## Units for Hector outputs

Hector outputs are returned as `unitval` structures, so if there is any doubt, you can always dump the return value to an output stream to see what the unit is (or test the units field against likely candidates).  What follows is a non-exhaustive list of some of the commonly-used outputs


Value     |  Units    |    Unit Symbol |   Notes
-----------|-----------|----------------|---------
Forcings  | W/m<sup>2</sup> | `U_W_M2` | All forcings given in W/m<sup>2</sup> relative to baseline
Temperature | degrees C | `U_DEGC` | Temperature anomaly
Atmospheric CO<sub>2</sub> concentration | PPMv CO<sub>2</sub> | `U_PPMV_CO2` |
