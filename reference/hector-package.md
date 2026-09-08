# hector: The Hector Simple Climate Model

Provides an R interface for the Hector Simple Climate Model. Using this
interface you can set up and initialize the model, change model
parameters and emissions inputs, run Hector, and retrieve model outputs.
Note that the package authors are not identical to the C++ model
authors.

## Details

This package allows you to run the Hector Simple Climate Model (SCM)
from R and receive the results back as R data frames.

## Getting started

To run a scenario, you will need a scenario input file. Several examples
are included in the `input` directory, which you can find with the
expression `system.file('input', package='hector')`. The input files
have names like `'hector_sspNNN.ini'`, where `NNN` is one of `119`,
`126`, `245`, `370`, `434`, `460`, or `ssp585`.

There is a quickstart wrapper called
[`runscenario`](https://jgcri.github.io/hector/reference/runscenario.md),
which takes an input file as an argument, runs the scenario, and returns
the results in a data frame. Which variables you get out are controlled
by an option setting. Setting this option is explained in the manual
page for
[`fetchvars`](https://jgcri.github.io/hector/reference/fetchvars.md).

## Advanced usage

You can get a lot more control over your hector run if you create a
hector instance (sometimes called a "core" elsewhere in this
documentation) using the
[`newcore`](https://jgcri.github.io/hector/reference/newcore.md)
function. Once you have that you can run Hector up to specific dates,
fetch results, set parameters or input data, rewind to an earlier date
so you can rerun (e.g., with changed emissions or parameters). When you
are finished with a hector instance, you should call
[`shutdown`](https://jgcri.github.io/hector/reference/shutdown.md) on it
to release the resources it has allocated.

## References

Hartin, C. A., Patel, P., Schwarber, A., Link, R. P., and Bond-Lamberty,
B. P.: A simple object-oriented and open-source model for scientific and
policy analyses of the global climate system - Hector v1.0, Geosci.
Model Dev., 8, 939-955, <https://doi.org/10.5194/gmd-8-939-2015>, 2015.

Hartin, C. A., Bond-Lamberty, B., Patel, P., and Mundra, A.: Ocean
acidification over the next three centuries using a simple global
climate carbon-cycle model: projections and sensitivities,
Biogeosciences, 13, 4329-4342,
<https://doi.org/10.5194/bg-13-4329-2016>, 2016.

## See also

[`runscenario`](https://jgcri.github.io/hector/reference/runscenario.md),
[`newcore`](https://jgcri.github.io/hector/reference/newcore.md),
[`shutdown`](https://jgcri.github.io/hector/reference/shutdown.md),
[`fetchvars`](https://jgcri.github.io/hector/reference/fetchvars.md)

## Author

**Maintainer**: Kalyn Dorheim <kalyn.dorheim@pnnl.gov>
([ORCID](https://orcid.org/0000-0001-8093-8397))

Authors:

- Ben Bond-Lamberty <bondlamberty@pnnl.gov>
  ([ORCID](https://orcid.org/0000-0001-9525-4633))

Other contributors:

- Skylar Gering <sgering@g.hmc.edu>
  ([ORCID](https://orcid.org/0000-0003-1974-3966)) \[contributor\]

- Corinne Hartin <hartin.corinne@epa.gov>
  ([ORCID](https://orcid.org/0000-0003-1834-6539)) \[contributor\]

- Robert Link ([ORCID](https://orcid.org/0000-0002-7071-248X))
  \[contributor\]

- Mat Nicholson (mnichol3) \[contributor\]

- Pralit Patel <pralit.patel@pnnl.gov>
  ([ORCID](https://orcid.org/0000-0003-3992-1061)) \[contributor\]

- Leeya Pressburger <leeya.pressburger@pnnl.gov>
  ([ORCID](https://orcid.org/0000-0002-6850-2504)) \[contributor, data
  contributor\]

- Peter Scully <ptrscll@gmail.com>
  ([ORCID](https://orcid.org/0009-0007-0234-0366)) \[contributor\]

- Alexey Shiklomanov <alexey.shiklomanov@nasa.gov>
  ([ORCID](https://orcid.org/0000-0003-4022-5979)) \[contributor\]

- Benjamin Vega-Westhoff
  ([ORCID](https://orcid.org/0000-0001-7881-8388)) \[contributor\]

- Dawn Woodard <dawn.woodard@pnnl.gov>
  ([ORCID](https://orcid.org/0000-0002-0468-4660)) \[contributor\]
