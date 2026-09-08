# Ocean Carbon Cycle

## Science

Based on the work of Lenton (2000)[^1], Knox and McElroy (1984)[^2], and
Sarmiento and Toggweiler (1984)[^3], Hector’s implementation of the
ocean carbon cycle was documented in Hartin et al. (2016).[^4] Hector’s
ocean is a four box model with two surface level boxes (high latitude
and low latitude), an intermediate, and deep ocean boxes. Within the
surface boxes Hector solves for pCO₂, pH, \[HCO$`{_3}^{-1}`$ \],
\[CO$`_{3}^{-2}`$\], aragonite $`\Omega_{Ar}`$ and calcite
$`\Omega_{Ca}`$. The chemistry of carbon in the surface boxes depends on
salinity, dissolved inorganic carbon (DIC), and temperature.

## Parameters

Six parameters may be set from the INI file or R interface:

- `tt` ([`TT()`](https://jgcri.github.io/hector/reference/ocean.md) in
  R), thermohaline overturning (m3/s)
- `tu` ([`TU()`](https://jgcri.github.io/hector/reference/ocean.md)),
  high latitude overturning (m3/s)
- `twi` ([`TWI()`](https://jgcri.github.io/hector/reference/ocean.md)),
  warm-intermediate exchange (m3/s)
- `tid` ([`TID()`](https://jgcri.github.io/hector/reference/ocean.md)),
  intermediate-deep exchange (m3/s)
- `preind_surface_c`
  ([`OCEAN_PREIND_C_SURF()`](https://jgcri.github.io/hector/reference/ocean.md)),
  preindustrial carbon in the surface ocean (Pg C)
- `preind_interdeep_c`
  ([`OCEAN_PREIND_C_ID()`](https://jgcri.github.io/hector/reference/ocean.md)),
  preindustrial carbon in the intermediate and deep ocean (Pg C)

## Implementation

1.  The `oceanbox` cpp and hpp files set up the four ocean boxes and
    determine connections between one another. How carbon, energy, and
    volume will move between the four ocean boxes.
2.  The `ocean_csys` cpp and hpp files define the solver for the
    temperature dependent system of equations that determine solubility
    and equilibrium constants.
3.  Within the `ocean_component`

[^1]: Lenton, T. M.: Land and ocean carbon cycle feedback effects on
    global warming in a simple Earth system model, Tellus B, 52,
    1159–1188, <doi:10.1034/j.1600-0889.2000.01104.x>, 2000.

[^2]: Knox, F. and McElroy, M. B.: Changes in Atmospheric CO2: Influence
    of the Marine Biota at High Latitude, J. Geophys. Res., 89,
    4629–4637, <doi:10.1029/JD089iD03p04629>, 1984.

[^3]: Sarmiento, J. L. and Toggweiler, J. R.: A new model for the role
    of the oceans in determining atmospheric PCO2, Nature, 308, 621–624,
    1984

[^4]: Hartin, C. A., Bond-Lamberty, B., Patel, P., and Mundra, A.: Ocean
    acidification over the next three centuries using a simple global
    climate carbon-cycle model: projections and sensitivities,
    Biogeosciences, 13, 4329–4342,
    <https://doi.org/10.5194/bg-13-4329-2016>, 2016.
